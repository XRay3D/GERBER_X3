// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "ex_parser.h"
#include "ex_file.h"

#include <ctre.hpp>

#include <QFile>
#include <cmath>

#include "utils.h"

namespace Excellon {

Parser::Parser(FilePlugin* const interface)
    : interface(interface) {
}

FileInterface* Parser::parseFile(const QString& fileName) {
    QFile file_(fileName);
    if (!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    file = new File;
    toolIt = file->tools_.end();
    file->setFileName(fileName);
    state_.reset(&file->format_);

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

bool Parser::parseComment(QString line) {
    if (line.startsWith(';')) {
        line = line.toUpper();
        if (auto [match, comment] = ctre::match<R"(^;(.*)$)">(toU16StrView(line)); match) { // regexComment

            if (auto [matchTool, tool, diam] = ctre::match<R"(\s*(?:HOLESIZE)\s*(\d+\.?\d*)\s*=\s*(\d+\.?\d*).*)">(comment); matchTool) { // tool
                qDebug() << __FUNCTION__ << tool << diam;
                const int tCode = static_cast<int>(CtreCapTo(tool).toDouble());
                if (toolIt == file->tools_.end()) {
                    toolIt = file->tools_.begin();
                    state_.toolId = tCode; // state_.tCode = file->tools_.firstKey();
                }
                file->tools_[tCode] = CtreCapTo(diam).toDouble() * (file->format_.unitMode == Inches ? (0.0254 * 1 / 25.4) : 1.0);
                //                file->tools_[tCode] *= 0.0254 * (1.0 / 25.4);
            }

            // static constexpr ctll::fixed_string regexFormat(R"(.*(?:FORMAT|format).*(\d).(\d))");
            // fixed_string(".*(?:FORMAT|format).*(\d).(\d)");
            // if (auto [matchFormat, integer, decimal] = ctre::match<regexFormat>(comment); matchFormat) {
            //     file->format_.integer = CtreCapTo(integer).toInt();
            //     file->format_.decimal = CtreCapTo(decimal).toInt();
            // }
            if (auto [match, integer, decimal] = ctre::match<R"(FILE_FORMAT=(\d).+(\d))">(comment); match) {
                file->format_.integer = CtreCapTo(integer).toInt();
                file->format_.decimal = CtreCapTo(decimal).toInt();
            }
            return true;
        }
    }
    return false;
}

bool Parser::parseGCode(const QString& line) {
    if (line.startsWith('G')) {
        static constexpr ctll::fixed_string regex(R"(^G([0]?[0-9]{2}).*$)"); // fixed_string("^G([0]?[0-9]{2}).*$");
        if (auto [whole, c1] = ctre::match<regex>(toU16StrView(line)); whole) {
            switch (CtreCapTo(c1).toInt()) {
            case G00:
                state_.gCode = G00;
                state_.wm = RouteMode;
                parsePos(line);
                break;
            case G01:
                state_.gCode = G01;
                parsePos(line);
                break;
            case G02:
                state_.gCode = G02;
                parsePos(line);
                break;
            case G03:
                state_.gCode = G03;
                parsePos(line);
                break;
            case G05:
                state_.gCode = G05;
                state_.wm = DrillMode;
                break;
            case G90:
                state_.gCode = G90;
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

bool Parser::parseMCode(const QString& line) {
    if (line.startsWith('M')) {
        static constexpr ctll::fixed_string regex(R"(^M([0]?[0-9]{2})$)"); // fixed_string("^M([0]?[0-9]{2})$");

        if (auto [whole, c1] = ctre::match<regex>(toU16StrView(line)); whole) {
            switch (CtreCapTo(c1).toInt()) {
            case M00: {
                //                auto tools = file->tools_;
                //                QList<int> keys;
                //                std::transform(begin(tools), end(tools), std::back_inserter(keys), [](auto& pair) { return pair.first; });
                //                if (keys.indexOf(state_.toolId) < (keys.size() - 1))
                //                    state_.toolId = keys[keys.indexOf(state_.toolId) + 1];
                state_.toolId = (++toolIt)->first;
            } break;
            case M15:
                state_.mCode = M15;
                state_.wm = RouteMode;
                state_.rawPosList = {state_.rawPos};
                state_.path = QPolygonF({state_.pos});
                break;
            case M16:
                state_.mCode = M16;
                state_.wm = RouteMode;
                state_.rawPosList.append(state_.rawPos);
                state_.path.append(state_.pos);
                file->append(Hole(state_, file));
                state_.path.clear();
                state_.rawPosList.clear();
                break;
            case M30:
                state_.mCode = M30;
                break;
            case M48:
                state_.mCode = M48;
                break;
            case M71:
                state_.mCode = M71;
                file->format_.unitMode = Millimeters;
                break;
            case M72:
                state_.mCode = M72;
                file->format_.unitMode = Inches;
                break;
            case M95:
                state_.mCode = M95;
                break;
            default:
                break;
            }
            return true;
        }
        if (line == "%" && state_.mCode == M48) {
            state_.mCode = M95;
            return true;
        }
        return false;
    }
    return false;
}

bool Parser::parseTCode(const QString& line) {
    if (line.startsWith('T')) {

        static constexpr ctll::fixed_string regex(R"(^T(\d+))"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"((?:([CFS])(\d*\.?\d+))?)"
                                                  R"(.*$)");
        static constexpr ctll::fixed_string regex2(R"(^.+C(\d*\.?\d+).*$)"); // fixed_string("^.+C(\d*\.?\d+).*$");
        if (auto [whole, tool, cfs1, diam1, cfs2, diam2, cfs3, diam3] = ctre::match<regex>(toU16StrView(line)); whole) {
            state_.toolId = CtreCapTo(tool).toInt();
            if (auto [whole, diam] = *ctre::range<regex2>(toU16StrView(line)).begin(); whole) {
                file->tools_[state_.toolId] = CtreCapTo(diam).toDouble();
                return true;
            }
            return true;
        }
    }
    return false;
}

bool Parser::parsePos(const QString& line) {

    //    enum {
    //        G = 1,
    //        X,
    //        Y,
    //        A
    //    };

    static constexpr ctll::fixed_string regex(R"(^(?:G(\d+))?)"
                                              R"((?:X([\+\-]?\d*\.?\d*))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d*))?)"
                                              R"((?:A([\+\-]?\d*\.?\d*))?)"
                                              R"(.*$)");

    if (auto [whole, G, X, Y, A] = ctre::match<regex>(toU16StrView(line)); whole) {
        if (!X && !Y)
            return false;

        if (X) {
            state_.rawPos.X = CtreCapTo(X);
            parseNumber(CtreCapTo(X), state_.pos.rx());
        }
        if (Y) {
            state_.rawPos.Y = CtreCapTo(Y);
            parseNumber(CtreCapTo(Y), state_.pos.ry());
        }
        if (A)
            state_.rawPos.A = CtreCapTo(A);

        switch (state_.wm) {
        case DrillMode:
            file->append(Hole(state_, file));
            break;
        case RouteMode:
            switch (state_.gCode) {
            case G00:
            case G01:
                state_.path.append(state_.pos);
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

bool Parser::parseSlot(const QString& line) {
    //    enum {
    //        X1 = 1,
    //        Y1,
    //        X2,
    //        Y2
    //    };

    static constexpr ctll::fixed_string regex(R"(^(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"(G85)"
                                              R"((?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"(.*$)");
    if (auto [whole, X1, Y1, X2, Y2] = ctre::match<regex>(toU16StrView(line)); whole) {
        state_.gCode = G85;
        state_.path.clear();
        state_.rawPosList.clear();

        if (X1) {
            state_.rawPos.X = QString {CtreCapTo(X1)};
            parseNumber(CtreCapTo(X1), state_.pos.rx());
        }

        if (Y1) {
            state_.rawPos.Y = QString {CtreCapTo(Y1)};
            parseNumber(CtreCapTo(Y1), state_.pos.ry());
        }

        state_.rawPosList.append(state_.rawPos);
        state_.path.append(state_.pos);

        if (X2) {
            state_.rawPos.X = QString {CtreCapTo(X2)};
            parseNumber(CtreCapTo(X2), state_.pos.rx());
        }

        if (Y2) {
            state_.rawPos.Y = QString {CtreCapTo(Y2)};
            parseNumber(CtreCapTo(Y2), state_.pos.ry());
        }

        state_.rawPosList.append(state_.rawPos);
        state_.path.append(state_.pos);

        file->append(Hole(state_, file));
        state_.path.clear();
        state_.rawPosList.clear();
        state_.gCode = G05;
        return true;
    }
    return false;
}

bool Parser::parseRepeat(const QString& line) {

    static constexpr ctll::fixed_string regex(R"(^R(\d+))"
                                              R"((?:X([\+\-]?\d*\.?\d+))?)"
                                              R"((?:Y([\+\-]?\d*\.?\d+))?)"
                                              R"($)");
    if (auto [whole, C1, C2, C3] = ctre::match<regex>(toU16StrView(line)); whole) {
        int count = CtreCapTo(C1).toInt();
        QPointF p;
        parseNumber(CtreCapTo(C2), p.rx());
        parseNumber(CtreCapTo(C3), p.ry());
        for (int i = 0; i < count; ++i) {
            state_.pos += p;
            file->append(Hole(state_, file));
        }
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& line) {
    static const QVector<QString> unitMode({QStringLiteral("INCH"), QStringLiteral("METRIC")});
    static const QVector<QString> zeroMode({QStringLiteral("LZ"), QStringLiteral("TZ")});
    if (auto [whole, C1, C2] = ctre::match<R"(^(METRIC|INCH).?(LZ|TZ)?$)">(toU16StrView(line)); whole) {
        if (C1)
            switch (unitMode.indexOf(CtreCapTo(C1))) {
            case Inches:
                file->format_.unitMode = Inches;
                break;
            case Millimeters:
                file->format_.unitMode = Millimeters;
                break;
            default:
                break;
            }
        if (C2)
            switch (zeroMode.indexOf(CtreCapTo(C2))) {
            case LeadingZeros:
                file->format_.zeroMode = LeadingZeros;
                break;
            case TrailingZeros:
                file->format_.zeroMode = TrailingZeros;
                break;
            default:
                break;
            }
        return true;
    }
    static constexpr ctll::fixed_string regex2(R"(^(FMAT).*(2)?$)"); // fixed_string("^(FMAT).*(2)?$");
    if (auto [whole, C1, C2] = ctre::match<regex2>(toU16StrView(line)); whole) {
        file->format_.unitMode = Inches;
        file->format_.zeroMode = LeadingZeros;
        return true;
    }
    return false;
}

bool Parser::parseNumber(QString Str, double& val) {
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
            if (Str.length() < file->format_.integer + file->format_.decimal) {
                switch (file->format_.zeroMode) {
                case LeadingZeros:
                    Str = Str + QString(file->format_.integer + file->format_.decimal - Str.length(), '0');
                    break;
                case TrailingZeros:
                    Str = QString(file->format_.integer + file->format_.decimal - Str.length(), '0') + Str;
                    break;
                }
            }
            val = Str.toDouble() * pow(10.0, -file->format_.decimal) * sign;
        }
        if (file->format_.unitMode == Inches)
            val *= 25.4;

        val = std::clamp(val, -1000.0, +1000.0); // one meter

        return true;
    }
    return flag;
}

void Parser::circularRout() {

    double radius = 0.0;
    parseNumber(state_.rawPos.A, radius);

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
        return state_.gCode == G03 ? (x1) : (x2);
    };

    QPointF center(CalcCircleCenter(state_.path.last(), state_.pos, radius));
    state_.path.append(arc(state_.path.last(), state_.pos, center));
    state_.path.last() = state_.pos;
}

QPolygonF Parser::arc(QPointF p1, QPointF p2, QPointF center) {
    double radius = sqrt(pow((center.x() - p1.x()), 2) + pow((center.y() - p1.y()), 2));
    double start = atan2(p1.y() - center.y(), p1.x() - center.x());
    double stop = atan2(p2.y() - center.y(), p2.x() - center.x());
    auto arc = [this](const QPointF& center, double radius, double start, double stop) {
        const double da_sign[4] = {0, 0, -1.0, +1.0};
        QPolygonF points;

        const int intSteps = App::settings().clpCircleSegments(radius * dScale); // MinStepsPerCircle;

        if (state_.gCode == G02 && stop >= start)
            stop -= 2.0 * pi;
        else if (state_.gCode == G03 && stop <= start)
            stop += 2.0 * pi;

        double angle = qAbs(stop - start);
        double steps = std::max(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
        double delta_angle = da_sign[state_.gCode] * angle * 1.0 / steps;
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

double Parser::parseNumber(QString Str, const State& state) {
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

} // namespace Excellon
