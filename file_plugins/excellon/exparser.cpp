// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "exparser.h"
#include "exfile.h"

#include "app.h"
#include "interfaces/file.h"

#include <QFile>
#include <QRegularExpression>
#include <cmath>

#include "leakdetector.h"

using namespace Excellon;

Parser::Parser(FilePluginInterface* const interface)
    : interface(interface)
{
}

FileInterface* Parser::parseFile(const QString& fileName)
{
    QFile file_(fileName);
    if (!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    file = new File;
    file->setFileName(fileName);
    m_state.reset(&this->file->m_format);

    QTextStream in(&file_);
    in.setAutoDetectUnicode(true);

    QString line;
    while (in.readLineInto(&line)) {
        file->lines().push_back(line);
        try {
            if (line == "%")
                continue;

            if (parseComment(line))
                continue;

            if (parseFormat(line))
                continue;

            if (parseTCode(line))
                continue;

            if (parseGCode(line))
                continue;

            if (parseMCode(line))
                continue;

            if (parseRepeat(line))
                continue;

            if (parseSlot(line))
                continue;

            if (parsePos(line))
                continue;
            qWarning() << "Excellon unparsed:" << line;
        } catch (const QString& errStr) {
            qWarning() << "exeption Q:" << errStr;
            emit interface->fileError("", QFileInfo(fileName).fileName() + "\n" + errStr);
            delete file;
            return nullptr;
        } catch (...) {
            qWarning() << "exeption S:" << errno;
            emit interface->fileError("", QFileInfo(fileName).fileName() + "\n" + "Unknown Error!");
            delete file;
            return nullptr;
        }
    }
    if (this->file->isEmpty()) {
        delete file;
        file = nullptr;
    } else {
        emit interface->fileReady(this->file);
    }
    return file;
}

bool Parser::parseComment(const QString& line)
{
    const QRegularExpression regexComment("^;(.*)$");
    if (auto match = regexComment.match(line); match.hasMatch()) {
        const QRegularExpression regexFormat(".*FORMAT.*(\\d{1}).(\\d{1}).*" /*, Qt::CaseInsensitive*/);
        if (auto matchFormat = regexFormat.match(match.captured(1)); matchFormat.hasMatch()) {
            qDebug() << "regexFormat" << matchFormat.capturedTexts();
            file->m_format.integer = matchFormat.captured(1).toInt();
            file->m_format.decimal = matchFormat.captured(2).toInt();
        }
        const QRegularExpression regexTool("\\s*Holesize\\s*(\\d+\\.?\\d*)\\s*=\\s*(\\d+\\.?\\d*).*" /*, Qt::CaseInsensitive*/);
        if (auto matchTool = regexTool.match(match.captured(1)); matchTool.hasMatch()) {
            qDebug() << "regexTool" << matchTool.capturedTexts();
            const int tCode = static_cast<int>(matchTool.captured(1).toDouble());
            file->m_tools[tCode] = matchTool.captured(2).toDouble() * 0.0254 * (1.0 / 25.4);
            m_state.tCode = tCode; //m_state.tCode = file->m_tools.firstKey();
        }
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& line)
{
    const QRegularExpression regex("^G([0]?[0-9]{2}).*$");
    if (auto match = regex.match(line); match.hasMatch()) {
        switch (match.captured(1).toInt()) {
        case G00:
            m_state.gCode = G00;
            m_state.wm = RouteMode;
            parsePos(line);
            break;
        case G01:
            m_state.gCode = G01;
            parsePos(line);
            break;
        case G02:
            m_state.gCode = G02;
            parsePos(line);
            break;
        case G03:
            m_state.gCode = G03;
            parsePos(line);
            break;
        case G05:
            m_state.gCode = G05;
            m_state.wm = DrillMode;
            break;
        case G90:
            m_state.gCode = G90;
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseMCode(const QString& line)
{
    const QRegularExpression regex("^M([0]?[0-9]{2})$");
    if (auto match = regex.match(line); match.hasMatch()) {
        switch (match.captured(1).toInt()) {
        case M00: {
            auto tools = file->m_tools;
            QList<int> keys;
            std::transform(begin(tools), end(tools), std::back_inserter(keys), [](auto& pair) { return pair.first; });
            if (keys.indexOf(m_state.tCode) < (keys.size() - 1))
                m_state.tCode = keys[keys.indexOf(m_state.tCode) + 1];
            //            QList<int> keys(file->m_tools.keys());
            //            if (keys.indexOf(m_state.tCode) < (keys.size() - 1))
            //                m_state.tCode = keys[keys.indexOf(m_state.tCode) + 1];
        } break;
        case M15:
            m_state.mCode = M15;
            m_state.wm = RouteMode;
            m_state.rawPosList = { m_state.rawPos };
            m_state.path = QPolygonF({ m_state.pos });
            break;
        case M16:
            m_state.mCode = M16;
            m_state.wm = RouteMode;
            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
            file->append(Hole(m_state, file));
            m_state.path.clear();
            m_state.rawPosList.clear();
            break;
        case M30:
            m_state.mCode = M30;
            break;
        case M48:
            m_state.mCode = M48;
            break;
        case M71:
            m_state.mCode = M71;
            file->m_format.unitMode = Millimeters;
            break;
        case M72:
            m_state.mCode = M72;
            file->m_format.unitMode = Inches;
            break;
        case M95:
            m_state.mCode = M95;
            break;
        default:
            break;
        }
        return true;
    }
    if (line == "%" && m_state.mCode == M48) {
        m_state.mCode = M95;
        return true;
    }
    return false;
}

bool Parser::parseTCode(const QString& line)
{
    const QRegularExpression regex("^T([0-9]{1,2})"
                                   "(?:([CFS])(\\d*\\.?\\d+))?"
                                   "(?:([CFS])(\\d*\\.?\\d+))?"
                                   "(?:([CFS])(\\d*\\.?\\d+))?"
                                   ".*$");
    if (auto match = regex.match(line); match.hasMatch()) {
        const QStringList capturedTexts(match.capturedTexts());
        const int index = capturedTexts.indexOf("C");
        bool ok;
        if (const auto tCode = match.captured(1).toInt(&ok); ok) {
            m_state.tCode = tCode;
            if (index > 0)
                file->m_tools[m_state.tCode] = match.captured(index + 1).toDouble();
            return true;
        }
    }
    return false;
}

bool Parser::parsePos(const QString& line)
{
    enum {
        G = 1,
        X,
        Y,
        A
    };

    QRegularExpression regex("^(?:G(\\d+))?"
                             "(?:X([+-]?\\d*\\.?\\d*))?"
                             "(?:Y([+-]?\\d*\\.?\\d*))?"
                             "(?:A([+-]?\\d*\\.?\\d*))?"
                             ".*$");

    if (auto match = regex.match(line); match.hasMatch()) {
        if (match.captured(X).isEmpty() && match.captured(Y).isEmpty())
            return false;

        if (!match.captured(X).isEmpty())
            m_state.rawPos.X = match.captured(X);
        if (!match.captured(Y).isEmpty())
            m_state.rawPos.Y = match.captured(Y);
        if (!match.captured(A).isEmpty())
            m_state.rawPos.A = match.captured(A);

        parseNumber(match.captured(X), m_state.pos.rx());
        parseNumber(match.captured(Y), m_state.pos.ry());

        switch (m_state.wm) {
        case DrillMode:
            file->append(Hole(m_state, file));
            break;
        case RouteMode:
            switch (m_state.gCode) {
            case G00:
            case G01:
                m_state.path.append(m_state.pos);
                break;
            case G02:
            case G03:
                circularRout();
                break;
            default:
                break;
            }
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseSlot(const QString& line)
{

    enum {
        X1 = 1,
        Y1,
        X2,
        Y2
    };

    QRegularExpression regex(
        "^(?:X([+-]?\\d*\\.?\\d+))?"
        "(?:Y([+-]?\\d*\\.?\\d+))?"
        "G85"
        "(?:X([+-]?\\d*\\.?\\d+))?"
        "(?:Y([+-]?\\d*\\.?\\d+))?"
        ".*$");
    if (auto match = regex.match(line); match.hasMatch()) {
        m_state.gCode = G85;
        m_state.path.clear();
        m_state.rawPosList.clear();
        {
            if (!match.captured(X1).isEmpty())
                m_state.rawPos.X = match.captured(X1);
            parseNumber(match.captured(X1), m_state.pos.rx());

            if (!match.captured(Y1).isEmpty())
                m_state.rawPos.Y = match.captured(Y1);
            parseNumber(match.captured(Y1), m_state.pos.ry());

            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
        }

        {
            if (!match.captured(X2).isEmpty())
                m_state.rawPos.X = match.captured(X2);
            parseNumber(match.captured(X2), m_state.pos.rx());

            if (!match.captured(Y2).isEmpty())
                m_state.rawPos.Y = match.captured(Y2);
            parseNumber(match.captured(Y2), m_state.pos.ry());

            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
        }

        file->append(Hole(m_state, file));
        m_state.path.clear();
        m_state.rawPosList.clear();
        m_state.gCode = G05;
        return true;
    }
    return false;
}

bool Parser::parseRepeat(const QString& line)
{

    QRegularExpression regex("^R(\\d+)"
                             "(?:X([+-]?\\d*\\.?\\d+))?"
                             "(?:Y([+-]?\\d*\\.?\\d+))?"
                             "$");
    if (auto match = regex.match(line); match.hasMatch()) {
        int count = match.captured(1).toInt();
        QPointF p;
        parseNumber(match.captured(2), p.rx());
        parseNumber(match.captured(3), p.ry());
        for (int i = 0; i < count; ++i) {
            m_state.pos += p;
            file->append(Hole(m_state, file));
        }
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& line)
{
    static const QVector<QString> unitMode({ QStringLiteral("INCH"), QStringLiteral("METRIC") });
    static const QVector<QString> zeroMode({ QStringLiteral("LZ"), QStringLiteral("TZ") });
    static const QRegularExpression regex("^(METRIC|INCH).*(LZ|TZ)?$");
    if (auto match = regex.match(line); match.hasMatch()) {
        switch (unitMode.indexOf(match.captured(1))) {
        case Inches:
            file->m_format.unitMode = Inches;
            break;
        case Millimeters:
            file->m_format.unitMode = Millimeters;
            break;
        default:
            break;
        }
        switch (zeroMode.indexOf(match.captured(2))) {
        case LeadingZeros:
            file->m_format.zeroMode = LeadingZeros;
            break;
        case TrailingZeros:
            file->m_format.zeroMode = TrailingZeros;
            break;
        default:
            break;
        }
        return true;
    }
    static const QRegularExpression regex2("^(FMAT).*(2)?$");
    if (auto match = regex2.match(line); match.hasMatch()) {
        file->m_format.unitMode = Inches;
        file->m_format.zeroMode = LeadingZeros;
        return true;
    }

    return false;
}

bool Parser::parseNumber(QString Str, double& val)
{
    bool flag = false;
    int sign = +1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = +1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }
            if (Str.length() < file->m_format.integer + file->m_format.decimal) {
                switch (file->m_format.zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(file->m_format.integer + file->m_format.decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(file->m_format.integer + file->m_format.decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -file->m_format.decimal) * sign;
        }
        if (file->m_format.unitMode == Inches)
            val *= 25.4;
        if (abs(val) > 1000.0)
            val = 1000.0;
        return true;
    }
    return flag;
}

void Parser::circularRout()
{

    double radius = 0.0;
    parseNumber(m_state.rawPos.A, radius);

    auto CalcCircleCenter = [this](QPointF a, QPointF b, float r) {
        //находим центр отрезка ab
        QPointF c = (a + b) / 2;
        //находим перпендикуляр, нормируем его
        QPointF n = QLineF(QPointF(), a - b).normalVector().unitVector().p2();
        //        n = new Vector2(n.Y, -n.X); //поворот на 90 градусов ;)
        //находим высоту искомого центра на отрезок
        double l = QLineF(QPointF(), a - b).length() / 2;
        double d = sqrt(r * r - l * l);
        //находм две точки
        QPointF x1 = c + n * d;
        QPointF x2 = c - n * d;
        return m_state.gCode == G03 ? (x1) : (x2);
    };

    QPointF center(CalcCircleCenter(m_state.path.last(), m_state.pos, radius));
    m_state.path.append(arc(m_state.path.last(), m_state.pos, center));
    m_state.path.last() = m_state.pos;
}

QPolygonF Parser::arc(QPointF p1, QPointF p2, QPointF center)
{

    double radius = sqrt(pow((center.x() - p1.x()), 2) + pow((center.y() - p1.y()), 2));
    double start = atan2(p1.y() - center.y(), p1.x() - center.x());
    double stop = atan2(p2.y() - center.y(), p2.x() - center.x());
    auto arc = [this](const QPointF& center, double radius, double start, double stop) {
        const double da_sign[4] = { 0, 0, -1.0, +1.0 };
        QPolygonF points;

        const int intSteps = App::settings().clpCircleSegments(radius * dScale); //MinStepsPerCircle;

        if (m_state.gCode == G02 && stop >= start)
            stop -= 2.0 * M_PI;
        else if (m_state.gCode == G03 && stop <= start)
            stop += 2.0 * M_PI;

        double angle = qAbs(stop - start);
        double steps = qMax(static_cast<int>(ceil(angle / (2.0 * M_PI) * intSteps)), 2);
        double delta_angle = da_sign[m_state.gCode] * angle * 1.0 / steps;
        for (int i = 0; i < steps; i++) {
            double theta = start + delta_angle * (i + 1);
            points.push_back(QPointF(
                center.x() + radius * cos(theta),
                center.y() + radius * sin(theta)));
        }
        return points;
    };
    return arc(center, radius, start, stop);
}

double Parser::parseNumber(QString Str, const State& state)
{
    double val = 0.0;
    int sign = +1;
    if (!Str.isEmpty()) {
        if (Str.contains('.')) {
            val = Str.toDouble();
        } else {

            if (Str.startsWith('+')) {
                Str.remove(0, 1);
                sign = +1;
            } else if (Str.startsWith('-')) {
                Str.remove(0, 1);
                sign = -1;
            }

            if (Str.length() < state.format->integer + state.format->decimal) {
                switch (state.format->zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(state.format->integer + state.format->decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(state.format->integer + state.format->decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -state.format->decimal) * sign;
        }
        if (state.format->unitMode == Inches)
            val *= 25.4;
        return val;
    }
    return val;
}
