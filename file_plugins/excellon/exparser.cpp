// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <ctre.hpp>

#include <QFile>
#include <cmath>

#include "leakdetector.h"
#include "utils.h"

namespace Excellon {

struct QRegularExpression {
    QRegularExpression() { }
};

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
    if (line.startsWith(';')) {
        qDebug() << "line" << line;
        auto data { toU16StrView(line) };
        static constexpr ctll::fixed_string regexComment(R"(^;(.*)$)"); // fixed_string("^;(.*)$");
        //        static constexpr ctll::fixed_string regexFormat(R"(.*(?:FORMAT|format).*(\d).(\d))"); // fixed_string(".*(?:FORMAT|format).*(\d).(\d)");
        static constexpr ctll::fixed_string regexTool(R"(\s*(?:HOLESIZE|holesize)\s*(\d+\.?\d*)\s*=\s*(\d+\.?\d*).*)"); // fixed_string("\s*(?:HOLESIZE|holesize)\s*(\d+\.?\d*)\s*=\s*(\d+\.?\d*).*");
        if (auto [match, comment] = ctre::match<regexComment>(data); match) {
            //            if (auto [matchFormat, integer, decimal] = ctre::match<regexFormat>(comment); matchFormat) {
            //                file->m_format.integer = CtreCapTo(integer).toInt();
            //                file->m_format.decimal = CtreCapTo(decimal).toInt();
            //            }
            if (auto [matchTool, tool, diam] = ctre::match<regexTool>(comment); matchTool) {
                qDebug() << "regexTool" << matchTool.data();
                const int tCode = static_cast<int>(CtreCapTo(tool).toDouble());
                file->m_tools[tCode] = CtreCapTo(diam).toDouble();
                file->m_tools[tCode] *= 0.0254 * (1.0 / 25.4);
                m_state.tCode = tCode; //m_state.tCode = file->m_tools.firstKey();
            }
            return true;
        }
    }
    return false;
}

bool Parser::parseGCode(const QString& line)
{
    if (line.startsWith('G')) {
        auto data { toU16StrView(line) };
        static constexpr ctll::fixed_string regex(R"(^G([0]?[0-9]{2}).*$)"); // fixed_string("^G([0]?[0-9]{2}).*$");
        if (auto [whole, c1] = ctre::match<regex>(data); whole) {
            switch (CtreCapTo(c1).toInt()) {
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
    return false;
}

bool Parser::parseMCode(const QString& line)
{
    if (line.startsWith('M')) {
        static constexpr ctll::fixed_string regex(R"(^M([0]?[0-9]{2})$)"); // fixed_string("^M([0]?[0-9]{2})$");
        auto data { toU16StrView(line) };
        if (auto [whole, c1] = ctre::match<regex>(data); whole) {
            switch (CtreCapTo(c1).toInt()) {
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
    return false;
}

bool Parser::parseTCode(const QString& line)
{
    if (line.startsWith('T')) {
        auto data { toU16StrView(line) };
        static constexpr ctll::fixed_string regex(R"(^T(\d+))"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"(.*$)");
        static constexpr ctll::fixed_string regex2(R"(^.+C(\d*\.?\d+).*$)"); // fixed_string("^.+C(\d*\.?\d+).*$");
        if (auto [whole, tool, cfs1, diam1, cfs2, diam2, cfs3, diam3] = ctre::match<regex>(data); whole) {
            m_state.tCode = CtreCapTo(tool).toInt();
            if (auto [whole, diam] = *ctre::range<regex2>(data).begin(); whole) {
                file->m_tools[m_state.tCode] = CtreCapTo(diam).toDouble();
                return true;
            }
            return true;
        }
    }
    return false;
}

bool Parser::parsePos(const QString& line)
{

    //    enum {
    //        G = 1,
    //        X,
    //        Y,
    //        A
    //    };

    auto data { toU16StrView(line) };
    static constexpr ctll::fixed_string regex(R"(^(?:G(\d+))?)"
                                              R"((?:X([\+\-]?\d*\.?\d*))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d*))?)"
                                              R"((?:A([\+\-]?\d*\.?\d*))?)"
                                              R"(.*$)");

    if (auto [whole, G, X, Y, A] = ctre::match<regex>(data); whole) {
        if (!X.size() && !Y.size())
            return false;

        if (X.size())
            m_state.rawPos.X = QString { CtreCapTo(X) };
        if (Y.size())
            m_state.rawPos.Y = QString { CtreCapTo(Y) };
        if (A.size())
            m_state.rawPos.A = QString { CtreCapTo(A) };

        parseNumber(CtreCapTo(X), m_state.pos.rx());
        parseNumber(CtreCapTo(Y), m_state.pos.ry());

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
    //    enum {
    //        X1 = 1,
    //        Y1,
    //        X2,
    //        Y2
    //    };
    auto data { toU16StrView(line) };
    static constexpr ctll::fixed_string regex(R"(^(?:X([\+\-]?\d*\.?\d+))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"(G85)"
                                              R"((?:X([\+\-]?\d*\.?\d+))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"(.*$)");
    if (auto [whole, X1, Y1, X2, Y2] = ctre::match<regex>(data); whole) {
        m_state.gCode = G85;
        m_state.path.clear();
        m_state.rawPosList.clear();
        {
            if (X1.size())
                m_state.rawPos.X = QString { CtreCapTo(X1) };
            parseNumber(CtreCapTo(X1), m_state.pos.rx());

            if (Y1.size())
                m_state.rawPos.Y = QString { CtreCapTo(Y1) };
            parseNumber(CtreCapTo(Y1), m_state.pos.ry());

            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
        }

        {
            if (X2.size())
                m_state.rawPos.X = QString { CtreCapTo(X2) };
            parseNumber(CtreCapTo(X2), m_state.pos.rx());

            if (Y2.size())
                m_state.rawPos.Y = QString { CtreCapTo(Y2) };
            parseNumber(CtreCapTo(Y2), m_state.pos.ry());

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
    auto data { toU16StrView(line) };
    static constexpr ctll::fixed_string regex(R"(^R(\d+))"
                                              R"((?:X([\+\-]?\d*\.?\d+))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"($)");
    if (auto [whole, c1, c2, c3] = ctre::match<regex>(data); whole) {
        int count = CtreCapTo(c1).toInt();
        QPointF p;
        parseNumber(CtreCapTo(c2), p.rx());
        parseNumber(CtreCapTo(c3), p.ry());
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
    auto data { toU16StrView(line) };
    static const QVector<QString> unitMode({ QStringLiteral("INCH"), QStringLiteral("METRIC") });
    static const QVector<QString> zeroMode({ QStringLiteral("LZ"), QStringLiteral("TZ") });
    static constexpr ctll::fixed_string regex(R"(^(METRIC|INCH).*(LZ|TZ)?$)"); // fixed_string("^(METRIC|INCH).*(LZ|TZ)?$");
    if (auto [whole, c1, c2] = ctre::match<regex>(data); whole) {
        switch (unitMode.indexOf(CtreCapTo(c1))) {
        case Inches:
            file->m_format.unitMode = Inches;
            break;
        case Millimeters:
            file->m_format.unitMode = Millimeters;
            break;
        default:
            break;
        }
        switch (zeroMode.indexOf(CtreCapTo(c2))) {
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
    static constexpr ctll::fixed_string regex2(R"(^(FMAT).*(2)?$)"); // fixed_string("^(FMAT).*(2)?$");
    if (auto [whole, c1, c2] = ctre::match<regex2>(data); whole) {
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
}
