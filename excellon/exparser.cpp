// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "exparser.h"
#include "exfile.h"

#include <QFile>
#include <cmath> // pow()
#include "settings.h"

using namespace Excellon;

Parser::Parser(QObject* parent)
    : FileParser(parent)
{
}

AbstractFile* Parser::parseFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    m_file = new File;
    m_file->setFileName(fileName);
    m_state.reset(&this->file()->m_format);

    QTextStream in(&file);
    QString line;
    while (in.readLineInto(&line)) {
        m_file->lines().append(line);
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
            emit fileError("", QFileInfo(fileName).fileName() + "\n" + errStr);
            delete m_file;
            return nullptr;
        } catch (...) {
            qWarning() << "exeption S:" << errno;
            emit fileError("", QFileInfo(fileName).fileName() + "\n" + "Unknown Error!");
            delete m_file;
            return nullptr;
        }
    }
    if (this->file()->isEmpty()) {
        delete m_file;
        m_file = nullptr;
    } else {
        m_file->createGi();
        emit fileReady(m_file);
    }
    return m_file;
}

bool Parser::isDrillFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    //    QString str(file.readAll().trimmed());

    //    if (str.startsWith("M48") && str.endsWith("M30"))
    //        return true;

    QTextStream in(&file);
    QString line;
    const QRegExp match("^T([0]?[0-9]{1})[FSC]((\\d*\\.?\\d+))?.*$");
    const QRegExp match2(".*Holesize.*");
    while (in.readLineInto(&line)) {
        if (match.exactMatch(line))
            return true;
        if (match2.exactMatch(line))
            return true;
    }

    return false;
}

bool Parser::parseComment(const QString& line)
{
    const QRegExp match("^;(.*)$");
    if (match.exactMatch(line)) {
        const QRegExp matchFormat(".*FORMAT.*([0-9]).([0-9]).*", Qt::CaseInsensitive);
        if (matchFormat.exactMatch(match.cap(1))) {
            file()->m_format.integer = matchFormat.cap(1).toInt();
            file()->m_format.decimal = matchFormat.cap(2).toInt();
        }
        const QRegExp matchTool("\\s*Holesize\\s*(\\d+\\.?\\d*)\\s*=\\s*(\\d+\\.?\\d*).*", Qt::CaseInsensitive);
        if (matchTool.exactMatch(match.cap(1))) {
            const int tCode = static_cast<int>(matchTool.cap(1).toDouble());
            file()->m_tools[tCode] = matchTool.cap(2).toDouble() * 0.0254 * (1.0 / 25.4);
            m_state.tCode = file()->m_tools.firstKey();
            qDebug() << m_state.tCode << matchTool.capturedTexts() << file()->m_tools;
        }
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& line)
{
    const QRegExp match("^G([0]?[0-9]{2}).*$");
    if (match.exactMatch(line)) {
        switch (match.cap(1).toInt()) {
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
    const QRegExp match("^M([0]?[0-9]{2})$");
    if (match.exactMatch(line)) {
        switch (match.cap(1).toInt()) {
        case M00: {
            QList<int> keys(file()->m_tools.keys());
            if (keys.indexOf(m_state.tCode) < (keys.size() - 1))
                m_state.tCode = keys[keys.indexOf(m_state.tCode) + 1];
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
            file()->append(Hole(m_state, file()));
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
            file()->m_format.unitMode = Millimeters;
            break;
        case M72:
            m_state.mCode = M72;
            file()->m_format.unitMode = Inches;
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
    const QRegExp match("^T([0-9]{1,2})"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        "(?:([CFS])(\\d*\\.?\\d+))?"
                        ".*$");
    //const QRegExp match("^T([0]?[0-9]{1})(?:C(\\d*\\.?\\d+))?.*$");
    if (match.exactMatch(line)) {
        const QStringList capturedTexts(match.capturedTexts());
        const int index = capturedTexts.indexOf("C");
        //        qDebug() << capturedTexts << index;
        m_state.tCode = match.cap(1).toInt();
        if (index > 0) {
            //            const double k = m_file->format.unitMode ? 1.0 : 25.4;
            file()->m_tools[m_state.tCode] = match.cap(index + 1).toDouble() /* * k*/;
            //            m_state.currentToolDiameter = m_file->m_tools[m_state.tCode];
        } /*else
            m_state.currentToolDiameter = m_file->m_tools[m_state.tCode];*/
        return true;
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

    QRegExp match("^(?:G(\\d+))?"
                  "(?:X([+-]?\\d*\\.?\\d*))?"
                  "(?:Y([+-]?\\d*\\.?\\d*))?"
                  "(?:A([+-]?\\d*\\.?\\d*))?"
                  ".*$");

    if (match.exactMatch(line)) {
        if (match.cap(X).isEmpty() && match.cap(Y).isEmpty())
            return false;

        if (!match.cap(X).isEmpty())
            m_state.rawPos.X = match.cap(X);
        if (!match.cap(Y).isEmpty())
            m_state.rawPos.Y = match.cap(Y);
        if (!match.cap(A).isEmpty())
            m_state.rawPos.A = match.cap(A);

        parseNumber(match.cap(X), m_state.pos.rx());
        parseNumber(match.cap(Y), m_state.pos.ry());

        switch (m_state.wm) {
        case DrillMode:
            file()->append(Hole(m_state, file()));
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

    QRegExp match(
        "^(?:X([+-]?\\d*\\.?\\d+))?"
        "(?:Y([+-]?\\d*\\.?\\d+))?"
        "G85"
        "(?:X([+-]?\\d*\\.?\\d+))?"
        "(?:Y([+-]?\\d*\\.?\\d+))?"
        ".*$");
    if (match.exactMatch(line)) {
        m_state.gCode = G85;
        m_state.path.clear();
        m_state.rawPosList.clear();
        {
            if (!match.cap(X1).isEmpty())
                m_state.rawPos.X = match.cap(X1);
            parseNumber(match.cap(X1), m_state.pos.rx());

            if (!match.cap(Y1).isEmpty())
                m_state.rawPos.Y = match.cap(Y1);
            parseNumber(match.cap(Y1), m_state.pos.ry());

            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
        }

        {
            if (!match.cap(X2).isEmpty())
                m_state.rawPos.X = match.cap(X2);
            parseNumber(match.cap(X2), m_state.pos.rx());

            if (!match.cap(Y2).isEmpty())
                m_state.rawPos.Y = match.cap(Y2);
            parseNumber(match.cap(Y2), m_state.pos.ry());

            m_state.rawPosList.append(m_state.rawPos);
            m_state.path.append(m_state.pos);
        }

        file()->append(Hole(m_state, file()));
        m_state.path.clear();
        m_state.rawPosList.clear();
        m_state.gCode = G05;
        return true;
    }
    return false;
}

bool Parser::parseRepeat(const QString& line)
{

    QRegExp match("^R(\\d+)"
                  "(?:X([+-]?\\d*\\.?\\d+))?"
                  "(?:Y([+-]?\\d*\\.?\\d+))?"
                  "$");
    if (match.exactMatch(line)) {
        int count = match.cap(1).toInt();
        QPointF p;
        parseNumber(match.cap(2), p.rx());
        parseNumber(match.cap(3), p.ry());
        for (int i = 0; i < count; ++i) {
            m_state.pos += p;
            file()->append(Hole(m_state, file()));
        }
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& line)
{
    static const QVector<QString> unitMode({ QStringLiteral("INCH"), QStringLiteral("METRIC") });
    static const QVector<QString> zeroMode({ QStringLiteral("LZ"), QStringLiteral("TZ") });
    static const QRegExp match("^(METRIC|INCH).*(LZ|TZ)?$");
    if (match.exactMatch(line)) {
        switch (unitMode.indexOf(match.cap(1))) {
        case Inches:
            file()->m_format.unitMode = Inches;
            break;
        case Millimeters:
            file()->m_format.unitMode = Millimeters;
            break;
        default:
            break;
        }
        switch (zeroMode.indexOf(match.cap(2))) {
        case LeadingZeros:
            file()->m_format.zeroMode = LeadingZeros;
            break;
        case TrailingZeros:
            file()->m_format.zeroMode = TrailingZeros;
            break;
        default:
            break;
        }
        return true;
    }
    static const QRegExp match2("^(FMAT).*(2)?$");
    if (match2.exactMatch(line)) {
        file()->m_format.unitMode = Inches;
        file()->m_format.zeroMode = LeadingZeros;
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
            if (Str.length() < file()->m_format.integer + file()->m_format.decimal) {
                switch (file()->m_format.zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(file()->m_format.integer + file()->m_format.decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(file()->m_format.integer + file()->m_format.decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -file()->m_format.decimal) * sign;
        }
        if (file()->m_format.unitMode == Inches)
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

        const int intSteps = GlobalSettings::gbrGcCircleSegments(radius * dScale); //MinStepsPerCircle;

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
