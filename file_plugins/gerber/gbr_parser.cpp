// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "gbr_parser.h"

#include "gbr_aperture.h"
#include "gbr_attraperfunction.h"
#include "gbr_attrfilefunction.h"
#include "gbr_file.h"

#include <ctre.hpp> //

#include "pluginfile.h"

#include "utils.h"
#include <QElapsedTimer>
#include <QMutex>

/*
.WHL Aperture Wheel File.PLC Silk Screen Component side
.CMP Copper Component side        .STC Solder Stop mask Component side
.SOL Copper Solder side          .STS Solder Stop mask Solder side

Then you need to zip the following files and deliver it to PCB Manufacturer

Gerber Files                    Extension
Top (copper) Layer              .GTL
Bottom (copper) Layer           .GBL
Top Overlay                     .GTO
Bottom Overlay                  .GBO
Top Paste Mask                  .GTP
Bottom Paste Mask               .GBP
Top Solder Mask                 .GTS
Bottom Solder Mask              .GBS
Keep-Out Layer                  .GKO
Drill Drawing                   .GD1
Drill Guide                     .GG1
Internal Plane Layer1,2,...,16  .GP1, .GP2, ... , .GP16

*The GTP file isn’t necessary for the PCB fabrication, because it is used to create a stencil(if your design had SMD parts).
*/
namespace Gerber {

struct QRegularExpression {
};

QDebug operator<<(QDebug debug, const std::string_view& sw) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QByteArray(sw.data(), sw.size());
    return debug;
}

Parser::Parser(FilePlugin* interface)
    : interface(interface) {
}

void Parser::parseLines(const QString& gerberLines, const QString& fileName) {
    static QMutex mutex;
    mutex.lock();
    try {

        reset(fileName);

        file->lines() = cleanAndFormatFile(gerberLines);
        file->m_graphicObjects.reserve(file->lines().size());
        if (file->lines().empty())
            emit interface->fileError("", file->shortName() + "\n" + "Incorrect File!");

        emit interface->fileProgress(file->shortName(), static_cast<int>(file->lines().size()), 0);

        m_lineNum = 0;

        //        std::map<int, int> rel;
        QElapsedTimer t;
        t.start();
        for (const QString& gerberLine : file->lines()) {
            m_currentGerbLine = gerberLine;
            ++m_lineNum;
            if (!(m_lineNum % 1000))
                emit interface->fileProgress(file->shortName(), 0, m_lineNum);
            auto dummy = [](const QString& gLine) -> bool {
                auto data { toU16StrView(gLine) };
                static constexpr ctll::fixed_string ptrnDummy(R"(^%(.{2})(.+)\*%$)"); // fixed_string("^%(.{2})(.+)\*%$");
                if (auto [whole, id, par] = ctre::match<ptrnDummy>(data); whole) {    ///*regexp.match(gLine)); match.hasMatch()*/) {
                    //qDebug() << "dummy" << gLine << id.data() << par.data();
                    return true;
                }
                return false;
            };

            switch (gerberLine.front().toLatin1()) {
            case '%':
                if (parseAttributes(gerberLine)) {
                    //++rel[4];
                    continue;
                }
                if (parseAperture(gerberLine)) {
                    //++rel[1];
                    continue;
                }
                if (parseApertureBlock(gerberLine)) {
                    //++rel[2];
                    continue;
                }
                if (parseApertureMacros(gerberLine)) {
                    //++rel[3];
                    continue;
                }
                if (parseFormat(gerberLine)) {
                    //++rel[5];
                    continue;
                }
                if (parseStepRepeat(gerberLine)) {
                    //++rel[6];
                    continue;
                }
                if (parseTransformations(gerberLine)) {
                    //++rel[7];
                    continue;
                }
                if (parseUnitMode(gerberLine)) {
                    //++rel[8];
                    continue;
                }
                if (parseImagePolarity(gerberLine)) {
                    //++rel[9];
                    continue;
                }
                if (parseLoadName(gerberLine)) {
                    //++rel[10];
                    continue;
                }
                if (dummy(gerberLine)) {
                    //++rel[11];
                    continue;
                }
            case 'D':
            case 'G':
                if (parseDCode(gerberLine))
                    continue;
                if (parseGCode(gerberLine))
                    continue;
            case 'M':
                if (parseEndOfFile(gerberLine))
                    continue;
            case 'X':
            case 'Y':
            default:
                if (parseLineInterpolation(gerberLine))
                    continue;
                if (parseCircularInterpolation(gerberLine))
                    continue;
            }

            // Line didn`t match any pattern. Warn user.
            qWarning() << QString("Line ignored (%1): '" + gerberLine + "'").arg(m_lineNum);

        } // End of file parsing

        qDebug() << file->shortName() << t.elapsed() << "ms";

        //        for (auto [key, val] : rel)
        //            qDebug() << key << '\t' << val;

        if (file->m_graphicObjects.empty()) {
            delete file;
        } else {

            if (attFile.m_function && attFile.m_function->side_() == Attr::AbstrFileFunc::Side::Bot) {
                file->setSide(Bottom);
            } else if (file->shortName().contains("bot", Qt::CaseInsensitive))
                file->setSide(Bottom);
            else if (file->shortName().contains(".gb", Qt::CaseInsensitive) && !file->shortName().endsWith(".gbr", Qt::CaseInsensitive))
                file->setSide(Bottom);

            if (attFile.m_function && attFile.m_function->function == Attr::File::Profile)
                file->setItemType(File::ApPaths);

            file->mergedPaths();
            file->m_components = components.values();
            file->groupedPaths();
            file->m_graphicObjects.shrink_to_fit();
            emit interface->fileReady(file);
            emit interface->fileProgress(file->shortName(), 1, 1);
        }
        reset(""); // clear parser data
    } catch (const QString& errStr) {
        qWarning() << "exeption Q:" << errStr;
        emit interface->fileError("", file->shortName() + "\n" + errStr);
        emit interface->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch (const char* errStr) {
        qWarning() << "exeption Q:" << errStr;
        emit interface->fileError("", file->shortName() + "\n" + errStr);
        emit interface->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch (...) {
        //        QString errStr(QString("%1: %2").arg(errno).arg(strerror(errno)));
        //        qWarning() << "exeption S:" << errStr;
        //        emit interface->fileError("", file->shortName() + "\n" + errStr);
        //        emit interface->fileProgress(file->shortName(), 1, 1);
        delete file;
    }
    mutex.unlock();
}

mvector<QString> Parser::cleanAndFormatFile(QString data) {
    mvector<QString> gerberLines;
    gerberLines.reserve(100000);

    enum State {
        Param,
        Macro,
        Data,
    };

    State state = Data;
    QString lastLine;

    auto gerberLinesAppend = [&gerberLines, &lastLine](State& state, const QString& val) -> void {
        switch (state) {
        case Macro:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                gerberLines << lastLine;
                state = Data;
            }
            break;
        case Param:
            lastLine.push_back(val);
            if (lastLine.endsWith('%')) {
                for (QString& tmpline : lastLine.remove('%').split('*')) {
                    if (!tmpline.isEmpty()) {
                        gerberLines << ('%' + tmpline + "*%");
                    }
                }
                state = Data;
            }
            break;
        case Data:
            break;
        }
    };

    auto lastLineClose = [&gerberLines](State state, QString& val) -> void {
        switch (state) {
        case Macro:
            if (!val.endsWith('%'))
                val.push_back('%');
            if (!val.endsWith("*%"))
                val.insert(val.length() - 2, '*');
            gerberLines << val;
            break;
        case Param:
            for (QString& tmpline : val.remove('%').split('*')) {
                if (!tmpline.isEmpty()) {
                    gerberLines << ('%' + tmpline + "*%");
                }
            }
            break;
        case Data:
            break;
        }
        val.clear();
    };

    auto dataClose = [&gerberLines](const QString& val) -> void {
        if (val.count('*') > 1) {
            for (QString& tmpline : val.split('*')) {
                if (!tmpline.isEmpty()) {
                    gerberLines << (tmpline + '*');
                }
            }
        } else {
            gerberLines << val;
        }
    };
    for (QString& line : data.replace('\r', '\n').replace("\n\n", "\n").replace('\t', ' ').split('\n')) {
        line = line.trimmed();

        if (line.isEmpty())
            continue;
        if (line == '*')
            continue;

        if (line.startsWith('%') && line.endsWith('%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if (line.startsWith("%AM")) {
                lastLineClose(Macro, line);
            } else {
                lastLineClose(Param, line);
            }
            state = Data;
            continue;
        } else if (line.startsWith("%AM")) {
            lastLineClose(state, lastLine);
            state = Macro;
            lastLine = line;
            continue;
        } else if (line.startsWith('%')) {
            lastLineClose(state, lastLine);
            state = Param;
            lastLine = line;
            continue;
        } else if (line.endsWith('*') && line.length() > 1) {
            switch (state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                dataClose(line);
                continue;
            }
        } else {
            switch (state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                //qWarning() << "Хрен его знает:" << line;
                continue;
            }
        }
    }
    gerberLines.shrink_to_fit();
    return gerberLines;
}

double Parser::arcAngle(double start, double stop) {
    if (m_state.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;
    if (m_state.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    return qAbs(stop - start);
}

double Parser::toDouble(const QString& Str, bool scale, bool inchControl) {
    bool ok;
    double d = Str.toDouble(&ok);
    if (m_state.format()->unitMode == Inches && inchControl)
        d *= 25.4;
    if (scale)
        d *= uScale;
    return d;
}

bool Parser::parseNumber(QString Str, cInt& val, int integer, int decimal) {
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (!decimal)
            decimal = m_state.format()->xDecimal;

        if (!integer)
            integer = m_state.format()->xInteger;

        if (Str.indexOf("+") == 0) {
            Str.remove(0, 1);
            sign = 1;
        }

        if (Str.indexOf("-") == 0) {
            Str.remove(0, 1);
            sign = -1;
        }

        if (Str.count('.'))
            Str.setNum(Str.split('.').first().toInt() + ("0." + Str.split('.').last()).toDouble());

        while (Str.length() < integer + decimal) {
            switch (m_state.format()->zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QByteArray(integer + decimal - Str.length(), '0') + Str;
                //Str = "0" + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QByteArray(integer + decimal - Str.length(), '0');
                //Str += "0";
                break;
#endif
            }
        }
        val = static_cast<cInt>(toDouble(Str, true) * pow(10.0, -decimal) * sign);
        return true;
    }
    return flag;
}

void Parser::addPath() {
    if (m_path.size() < 2) {
        resetStep();
        return;
    }
    switch (m_state.region()) {
    case On:
        m_state.setType(Region);
        switch (m_abSrIdStack.top().workingType) {
        case WorkingType::Normal:
            file->m_graphicObjects.push_back(GraphicObject(m_goId++, m_state, createPolygon(), file, m_path));
            break;
        case WorkingType::StepRepeat:
            m_stepRepeat.storage.append(GraphicObject(m_goId++, m_state, createPolygon(), file, m_path));
            break;
        case WorkingType::ApertureBlock:
            apBlock(m_abSrIdStack.top().apertureBlockId)->append(GraphicObject(m_goId++, m_state, createPolygon(), file, m_path));
            break;
        }
        break;
    case Off:
        m_state.setType(Line);
        switch (m_abSrIdStack.top().workingType) {
        case WorkingType::Normal:
            file->m_graphicObjects.push_back(GraphicObject(m_goId++, m_state, createLine(), file, m_path));
            break;
        case WorkingType::StepRepeat:
            m_stepRepeat.storage.append(GraphicObject(m_stepRepeat.storage.size(), m_state, createLine(), file, m_path));
            break;
        case WorkingType::ApertureBlock:
            apBlock(m_abSrIdStack.top().apertureBlockId)->append(GraphicObject(apBlock(m_abSrIdStack.top().apertureBlockId)->size(), m_state, createLine(), file, m_path));
            break;
        }
        break;
    }
    if (aperFunctionMap.contains(m_state.aperture()) && aperFunctionMap[m_state.aperture()].m_function->function == Attr::Aperture::ComponentOutline) {
        components[refDes].addFootprint(m_path);
    }
    resetStep();
}

void Parser::addFlash() {
    m_state.setType(Aperture);
    if (!file->m_apertures.contains(m_state.aperture()) && file->m_apertures[m_state.aperture()].get() == nullptr) {
        QString str;
        for (const auto& [ap, apPtr] : file->m_apertures)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(m_state.aperture()).arg(str);
    }

    AbstractAperture* ap = file->m_apertures[m_state.aperture()].get();
    ap->setUsed();
    Paths paths(ap->draw(m_state, m_abSrIdStack.top().workingType != WorkingType::ApertureBlock));
    ////////////////////////////////// Draw Drill //////////////////////////////////
    if (ap->withHole())
        paths.push_back(ap->drawDrill(m_state));

    switch (m_abSrIdStack.top().workingType) {
    case WorkingType::Normal:
        file->m_graphicObjects.push_back(GraphicObject(m_goId++, m_state, paths, file));
        break;
    case WorkingType::StepRepeat:
        m_stepRepeat.storage.append(GraphicObject(m_stepRepeat.storage.size(), m_state, paths, file));
        break;
    case WorkingType::ApertureBlock:
        apBlock(m_abSrIdStack.top().apertureBlockId)->append(GraphicObject(apBlock(m_abSrIdStack.top().apertureBlockId)->size(), m_state, paths, file));
        break;
    }
    if (aperFunctionMap.contains(m_state.aperture()) && !refDes.isEmpty()) {
        switch (aperFunctionMap[m_state.aperture()].m_function->function) {
        case Attr::Aperture::ComponentPin:
            components[refDes].pins().back().pos = m_state.curPos();
            break;
        case Attr::Aperture::ComponentMain:
            components[refDes].setReferencePoint(m_state.curPos());
            break;
        default:
            break;
        }
    }

    resetStep();
}

void Parser::reset(const QString& fileName) {
    if (!fileName.isEmpty())
        file = new File(fileName);
    aperFunctionMap.clear();
    attAper = {};
    components.clear();
    m_abSrIdStack.clear();
    m_abSrIdStack.push({ WorkingType::Normal, 0 });
    m_apertureMacro.clear();
    m_currentGerbLine.clear();
    m_goId = 0;
    m_path.clear();
    m_state = State(file->format());
    m_stepRepeat.reset();
    refDes.clear();
}

void Parser::resetStep() {
    m_currentGerbLine.clear();
    m_path.clear();
    m_path.push_back(m_state.curPos());
}

IntPoint Parser::parsePosition(const QString& xyStr) {
    auto data { toU16StrView(xyStr) };
    static constexpr ctll::fixed_string ptrnPosition(R"((?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+)"); // fixed_string("(?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+");
    if (auto [whole, x, y] = ctre::match<ptrnPosition>(data /*xyStr*/); whole) {
        cInt tmp = 0;
        if (x && parseNumber(CtreCapTo(x), tmp, m_state.format()->xInteger, m_state.format()->xDecimal))
            m_state.format()->coordValueNotation == AbsoluteNotation ? m_state.curPos().X = tmp : m_state.curPos().X += tmp;
        tmp = 0;
        if (y && parseNumber(CtreCapTo(y), tmp, m_state.format()->yInteger, m_state.format()->yDecimal))
            m_state.format()->coordValueNotation == AbsoluteNotation ? m_state.curPos().Y = tmp : m_state.curPos().Y += tmp;
    }

    if (2.0e-310 > m_state.curPos().X && m_state.curPos().X > 0.0) {
        throw GbrObj::tr("line num %1: '%2', error value.").arg(QString::number(m_lineNum), QString(m_currentGerbLine));
    }
    if (2.0e-310 > m_state.curPos().Y && m_state.curPos().Y > 0.0) {
        throw GbrObj::tr("line num %1: '%2', error value.").arg(QString::number(m_lineNum), QString(m_currentGerbLine));
    }
    return m_state.curPos();
}

Path Parser::arc(const IntPoint& center, double radius, double start, double stop) {
    const double da_sign[4] = { 0, 0, -1.0, +1.0 };
    Path points;

    const int intSteps = App::settings().clpCircleSegments(radius * dScale); //MinStepsPerCircle;

    if (m_state.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    else if (m_state.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;

    double angle = qAbs(stop - start);
    double steps = qMax(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
    double delta_angle = da_sign[m_state.interpolation()] * angle * 1.0 / steps;
    for (int i = 0; i < steps; i++) {
        double theta = start + delta_angle * (i + 1);
        points.push_back(IntPoint(
            static_cast<cInt>(center.X + radius * cos(theta)),
            static_cast<cInt>(center.Y + radius * sin(theta))));
    }

    return points;
}

Path Parser::arc(IntPoint p1, IntPoint p2, IntPoint center) {
    double radius = sqrt(pow((center.X - p1.X), 2) + pow((center.Y - p1.Y), 2));
    double start = atan2(p1.Y - center.Y, p1.X - center.X);
    double stop = atan2(p2.Y - center.Y, p2.X - center.X);
    return arc(center, radius, start, stop);
}

Paths Parser::createLine() {
    if (file->m_apertures.contains(m_state.aperture()) && file->m_apertures[m_state.aperture()].get())
        file->m_apertures[m_state.aperture()].get()->setUsed();
    Paths solution;
    if (!file->m_apertures.contains(m_state.aperture())) {
        QString str;
        for (const auto& [ap, apPtr] : file->m_apertures)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(m_state.aperture()).arg(str);
    }

    if (file->m_apertures[m_state.aperture()]->type() == Rectangle) {
        auto rect = std::static_pointer_cast<ApRectangle>(file->m_apertures[m_state.aperture()]);
        if (!qFuzzyCompare(rect->m_width, rect->m_height)) // only square Aperture
            throw GbrObj::tr("Aperture D%1 (%2) not supported!").arg(m_state.aperture()).arg(rect->name());
        double size = rect->m_width * uScale * 0.5 * m_state.scaling();
        if (qFuzzyIsNull(size))
            return {};
        ClipperOffset offset;
        offset.AddPath(m_path, jtSquare, etOpenSquare);
        offset.Execute(solution, size);
        if (m_state.imgPolarity() == Negative)
            ReversePaths(solution);
    } else {
        double size = file->m_apertures[m_state.aperture()]->apertureSize() * uScale * 0.5 * m_state.scaling();
        if (qFuzzyIsNull(size))
            return {};
        ClipperOffset offset;
        offset.AddPath(m_path, jtRound, etOpenRound);
        offset.Execute(solution, size);
        if (m_state.imgPolarity() == Negative)
            ReversePaths(solution);
    }
    return solution;
}

Paths Parser::createPolygon() {
    if (Area(m_path) > 0.0) {
        if (m_state.imgPolarity() == Negative)
            ReversePath(m_path);
    } else {
        if (m_state.imgPolarity() == Positive)
            ReversePath(m_path);
    }
    return { m_path };
}

bool Parser::parseAperture(const QString& gLine) {
    /*
     *    Parse gerber aperture definition into dictionary of apertures.
     *    The following kinds and their attributes are supported:
     *    * Circular (C)*: size (float)
     *    * Rectangle (R)*: width (float), height (float)
     *    * Obround (O)*: width (float), height (float).
     *    * Polygon (P)*: diameter(float), vertices(int), [rotation(float)]
     *    * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)
     */
    auto data { toU16StrView(gLine) };
    static const QVector<QString> slApertureType { "C", "R", "O", "P", "M" };
    static constexpr ctll::fixed_string ptrnAperture(R"(^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$)"); // fixed_string("^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$");
    if (auto [whole, apId, apType, paramList_] = ctre::match<ptrnAperture>(data); whole) {
        int aperture { CtreCapTo(apId) };
        auto paramList { QString(CtreCapTo(paramList_)).split('X') };
        double hole = 0.0, rotation = 0.0;
        auto& apertures = file->m_apertures;
        switch (slApertureType.indexOf(QString { CtreCapTo(apType) })) {
        case Circle:
            if (paramList.size() > 1)
                hole = toDouble(paramList[1]);
            apertures[aperture] = std::make_shared<ApCircle>(toDouble(paramList[0]), hole, file->format());
            break;
        case Rectangle:
            if (paramList.size() > 2)
                hole = toDouble(paramList[2]);
            if (paramList.size() < 2)
                paramList << paramList[0];
            apertures.emplace(aperture, std::make_shared<ApRectangle>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file->format()));
            break;
        case Obround:
            if (paramList.size() > 2)
                hole = toDouble(paramList[2]);
            apertures.emplace(aperture, std::make_shared<ApObround>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file->format()));

            break;
        case Polygon:
            if (paramList.length() > 2)
                rotation = toDouble(paramList[2], false, false);
            if (paramList.length() > 3)
                hole = toDouble(paramList[3]);
            apertures.emplace(aperture, std::make_shared<ApPolygon>(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, file->format()));
            break;
        case Macro:
        default:
            VarMap macroCoeff;
            for (int i = 0; i < paramList.size(); ++i)
                macroCoeff.emplace(QString("$%1").arg(i + 1), toDouble(paramList[i], false, false));
            apertures.emplace(aperture, std::make_shared<ApMacro>(CtreCapTo(apType).operator QString(), m_apertureMacro[CtreCapTo(apType)].split('*'), macroCoeff, file->format()));
            break;
        }
        if (attAper.m_function)
            aperFunctionMap[aperture] = attAper;
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnApertureBlock(R"(^%ABD(\d+)\*%$)"); // fixed_string("^%ABD(\d+)\*%$");
    if (auto [whole, id] = ctre::match<ptrnApertureBlock>(data); whole) {
        m_abSrIdStack.push({ WorkingType::ApertureBlock, int(CtreCapTo(id)) });
        file->m_apertures.emplace(m_abSrIdStack.top().apertureBlockId, std::make_shared<ApBlock>(file->format()));
        return true;
    }
    if (gLine == "%AB*%") {
        addPath();
        m_abSrIdStack.pop();
        return true;
    }
    return false;
}

bool Parser::parseTransformations(const QString& gLine) {
    enum {
        trPolarity,
        trMirror,
        trRotate,
        trScale,
    };
    auto data { toU16StrView(gLine) };
    static const QVector<char> slTransformations { 'P', 'M', 'R', 'S' };
    static const QVector<char> slLevelPolarity { 'D', 'C' };
    static const QVector<QString> slLoadMirroring { "N", "X", "Y", "XY" };
    if (auto [whole, tr, val] = ctre::match<R"(^%L([PMRS])(.+)\*%$)">(data); whole) {
        const char trType = tr.data()[0];
        switch (slTransformations.indexOf(trType)) {
        case trPolarity:
            addPath();
            switch (slLevelPolarity.indexOf(val.data()[0])) {
            case Positive:
                m_state.setImgPolarity(Positive);
                break;
            case Negative:
                m_state.setImgPolarity(Negative);
                break;
            default:
                throw "bool Parser::parseTransformations(const SLI & gLine) - Polarity error!";
            }
            return true;
        case trMirror:
            m_state.setMirroring(static_cast<Mirroring>(slLoadMirroring.indexOf(CtreCapTo(val))));
            return true;
        case trRotate:
            m_state.setRotating(CtreCapTo(val));
            return true;
        case trScale:
            m_state.setScaling(CtreCapTo(val));
            return true;
        }
    }
    return false;
}

bool Parser::parseStepRepeat(const QString& gLine) {
    /*
     *     <SR open>      = %SRX<Repeats>Y<Repeats>I<Step>J<Step>*%
     *     <SR close>     = %SR*%
     *     <SR statement> = <SR open>{<single command>|<region statement>}<SR close>
     */
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnStepRepeat(R"(^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$)"); // fixed_string("^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$");
    if (auto [whole, srx, sry, sri, srj] = ctre::match<ptrnStepRepeat>(data); whole) {
        if (m_abSrIdStack.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        m_stepRepeat.reset();
        m_stepRepeat.x = CtreCapTo(srx);
        m_stepRepeat.y = CtreCapTo(sry);
        m_stepRepeat.i = CtreCapTo(sri), m_stepRepeat.i *= uScale;
        m_stepRepeat.j = CtreCapTo(srj), m_stepRepeat.j *= uScale;
        if (m_state.format()->unitMode == Inches) {
            m_stepRepeat.i *= 25.4;
            m_stepRepeat.j *= 25.4;
        }
        if (m_stepRepeat.x > 1 || m_stepRepeat.y > 1)
            m_abSrIdStack.push({ WorkingType::StepRepeat, 0 });
        return true;
    }

    static constexpr ctll::fixed_string ptrnStepRepeatEnd(R"(^%SR\*%$)"); // fixed_string("^%SR\*%$");
    if (ctre::match<ptrnStepRepeatEnd>(data)) {
        if (m_abSrIdStack.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        return true;
    }

    return false;
}

void Parser::closeStepRepeat() {
    addPath();
    for (int y = 0; y < m_stepRepeat.y; ++y) {
        for (int x = 0; x < m_stepRepeat.x; ++x) {
            const IntPoint pt(static_cast<cInt>(m_stepRepeat.i * x), static_cast<cInt>(m_stepRepeat.j * y));
            for (GraphicObject& go : m_stepRepeat.storage) {
                Paths paths(go.paths());
                for (Path& path : paths)
                    TranslatePath(path, pt);
                Path path(go.path());
                TranslatePath(path, pt);
                auto state = go.state();
                state.setCurPos({ state.curPos().X + pt.X, state.curPos().Y + pt.Y });
                file->m_graphicObjects.push_back(GraphicObject(m_goId++, state, paths, go.gFile(), path));
            }
        }
    }
    m_stepRepeat.reset();
    m_abSrIdStack.pop();
}

ApBlock* Parser::apBlock(int id) { return static_cast<ApBlock*>(file->m_apertures[id].get()); }

bool Parser::parseApertureMacros(const QString& gLine) {
    // Start macro if(match, else not an AM, carry on.
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnApertureMacros(R"(^%AM([^\*]+)\*([^%]+)?(%)?$)"); // fixed_string("^%AM([^\*]+)\*([^%]+)?(%)?$");
    if (auto [whole, c1, c2, c3] = ctre::match<ptrnApertureMacros>(data); whole) {
        if (c1.size() && c2.size()) {
            m_apertureMacro[CtreCapTo(c1)] = QString { CtreCapTo(c2) };
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnAttributes(R"(^%(T[FAOD])(\.?)(.*)\*%$)"); // fixed_string("^%(T[FAOD])(\.?)(.*)\*%$");
    if (auto [whole, c1, c2, c3] = ctre::match<ptrnAttributes>(data); whole) {
        QString cap[] { CtreCapTo(whole), CtreCapTo(c1), CtreCapTo(c2), CtreCapTo(c3) };
        switch (Attr::Command::value(cap[1])) {
        case Attr::Command::TF:
            attFile.parse(cap[3].split(','));
            break;
        case Attr::Command::TA:
            attAper.parse(cap[3].split(','));
            break;
            //            break;
            //            {
            //                QStringList sl(matchAttr.cap(3).split(','));
            //                int index = Attr::Aperture::value(sl.first());
            //                switch (index) {
            //                case Attr::Aperture::AperFunction:
            //                    if (sl.size() > 1) {
            //                        switch (int key = Attr::AperFunction::value(sl[1])) {
            //                        case Attr::AperFunction::ComponentMain:
            //                        case Attr::AperFunction::ComponentOutline:
            //                        case Attr::AperFunction::ComponentPin:
            //                            aperFunction = key;
            //                            break;
            //                        default:
            //                            aperFunction = -1;
            //                        }
            //                    }
            //                    break;
            //                case Attr::Aperture::DrillTolerance:
            //                case Attr::Aperture::FlashText:
            //                default:
            //                    ;
            //                }
            //                //apertureAttributesStrings.append(matchAttr.cap(2));
            //            }
        case Attr::Command::TO: {
            for (int i = cap[3].indexOf('"'); i > -1; i = cap[3].indexOf('"'))
                cap[3].remove(i, 1);
            auto sl(cap[3].split(',')); // remove symbol "
            switch (int index = Component::value1(sl.first()); index) {
            case Component::N: // The CAD net name of a conducting object, e.g. Clk13.
                break;
            case Component::P: // Pins
                components[sl.value(1)].addPin({ sl.value(2), sl.value(3), {} });
                break;
            case Component::C:
                switch (int key = Component::value2(sl.first())) {
                case Component::Rot:
                case Component::Mfr:
                case Component::MPN:
                case Component::Val:
                case Component::Mnt:
                case Component::Ftp:
                case Component::PgN:
                case Component::Hgt:
                case Component::LbN:
                case Component::LbD:
                case Component::Sup:
                    components[refDes].setData(key, sl);
                    break;
                default:
                    //                    static const QRegularExpression rx("(\\[0-9a-fA-F]{4})");
                    //                    int pos = 0;
                    //                    auto match(rx.match(sl.last(), pos));
                    //                    while (match.hasMatch()) { //(pos = rx.indexIn(sl.last(), pos)) != -1) {
                    //                        sl.last().replace(pos++, 5, QChar(match.captured(1).right(4).toUShort(nullptr, 16)));
                    //                        auto match(rx.match(sl.last(), pos));
                    //                    }
                    //                    while ((pos = rx.indexIn(sl.last(), pos)) != -1) {
                    //                        sl.last().replace(pos++, 5, QChar(rx.cap(1).right(4).toUShort(nullptr, 16)));
                    //                    }
                    refDes = sl.last();
                    components[refDes].setRefdes(refDes);
                }
                break;
            default:
                qDebug() << gLine << cap[0];
            }
        } break;
        case Attr::Command::TD:
            break;
            {
                enum {
                    Command,
                    AttributeName
                };
                refDes.clear();
                attAper = {};
            }
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseCircularInterpolation(const QString& gLine) {
    // G02/G03 - Circular interpolation
    // 2-clockwise, 3-counterclockwise
    if (!(gLine.startsWith('G') || gLine.startsWith('X') || gLine.startsWith('Y')))
        return false;

    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnCircularInterpolation(R"(^(?:G0?([23]))?)"
                                                                  R"(X?([\+\-]?\d+)*)"
                                                                  R"(Y?([\+\-]?\d+)*)"
                                                                  R"(I?([\+\-]?\d+)*)"
                                                                  R"(J?([\+\-]?\d+)*)"
                                                                  R"([^D]*(?:D0?([12]))?\*$)");
    if (auto [whole, cg, cx, cy, ci, cj, cd] = ctre::match<ptrnCircularInterpolation>(data); whole) {
        if (!cg.size() && m_state.gCode() != G02 && m_state.gCode() != G03)
            return false;
        cInt x = 0, y = 0, i = 0, j = 0;
        cx.size() ? parseNumber(CtreCapTo(cx), x, m_state.format()->xInteger, m_state.format()->xDecimal) : x = m_state.curPos().X;
        cy.size() ? parseNumber(CtreCapTo(cy), y, m_state.format()->yInteger, m_state.format()->yDecimal) : y = m_state.curPos().Y;
        parseNumber(CtreCapTo(ci), i, m_state.format()->xInteger, m_state.format()->xDecimal);
        parseNumber(CtreCapTo(cj), j, m_state.format()->yInteger, m_state.format()->yDecimal);
        // Set operation code if provided
        if (cd.size())
            m_state.setDCode(static_cast<Operation>(CtreCapTo(cd).toInt()));
        int gc = cg ? int(CtreCapTo(cg)) : m_state.gCode();
        switch (gc) {
        case G02:
            m_state.setInterpolation(ClockwiseCircular);
            m_state.setGCode(G02);
            break;
        case G03:
            m_state.setInterpolation(CounterclockwiseCircular);
            m_state.setGCode(G03);
            break;
        default:
            if (m_state.interpolation() != ClockwiseCircular && m_state.interpolation() != CounterclockwiseCircular) {
                qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(m_lineNum);
                qWarning() << QString(gLine);
                m_state.setCurPos({ x, y });
                m_state.setGCode(G01);
                return false;
            }
            break;
        }

        if (m_state.quadrant() == Undef) {
            qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(m_lineNum);
            qWarning() << QString(gLine);
            return true;
        }

        switch (m_state.dCode()) {
        case D01:
            break;
        case D02: // Nothing created! Pen Up.
            m_state.setDCode(D01);
            addPath();
            m_state.setCurPos({ x, y });
            return true;
        case D03: // Flash should not happen here
            m_state.setCurPos({ x, y });
            qWarning() << QString("Trying to flash within arc. (%1)").arg(m_lineNum);
            return true;
        }

        const IntPoint& curPos = m_state.curPos();

        const IntPoint centerPos[4] = {
            { curPos.X + i, curPos.Y + j },
            { curPos.X - i, curPos.Y + j },
            { curPos.X + i, curPos.Y - j },
            { curPos.X - i, curPos.Y - j }
        };

        bool valid = false;

        m_path.push_back(m_state.curPos());
        Path arcPolygon;
        switch (m_state.quadrant()) {
        case Multi: //G75
        {
            const double radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
            const double start = atan2(-j, -i); // Start angle
            // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
            // Ч­то должно привести к образованию дуги в 360 градусов.
            const double stop = (m_state.curPos() == IntPoint(x, y)) ? start : atan2(-centerPos[0].Y + y, -centerPos[0].X + x); // Stop angle

            arcPolygon = arc(IntPoint(centerPos[0].X, centerPos[0].Y), radius1, start, stop);
            //arcPolygon = arc(curPos, IntPoint(x, y), centerPos[0]);
            // Последняя точка в вычисленной дуге может иметь числовые ошибки.
            // Точной конечной точкой является указанная (x, y). Заменить.
            m_state.curPos() = IntPoint { x, y };
            if (arcPolygon.size())
                arcPolygon.back() = m_state.curPos();
            else
                arcPolygon.push_back(m_state.curPos());
        } break;
        case Single: //G74
            for (int c = 0; c < 4; ++c) {
                const double radius1 = sqrt(static_cast<double>(i) * static_cast<double>(i) + static_cast<double>(j) * static_cast<double>(j));
                const double radius2 = sqrt(pow(centerPos[c].X - x, 2.0) + pow(centerPos[c].Y - y, 2.0));
                // Убеждаемся, что радиус начала совпадает с радиусом конца.
                if (qAbs(radius2 - radius1) > (5e-4 * uScale)) // Недействительный центр.
                    continue;
                // Correct i and j and return true; as with multi-quadrant.
                i = centerPos[c].X - m_state.curPos().X;
                j = centerPos[c].Y - m_state.curPos().Y;
                // Углы
                const double start = atan2(-j, -i);
                const double stop = atan2(-centerPos[c].Y + y, -centerPos[c].X + x);
                const double angle = arcAngle(start, stop);
                if (angle < (pi + 1e-5) * 0.5) {
                    arcPolygon = arc(IntPoint(centerPos[c].X, centerPos[c].Y), radius1, start, stop);
                    // Replace with exact values
                    m_state.setCurPos({ x, y });
                    if (arcPolygon.size())
                        arcPolygon.back() = m_state.curPos();
                    else
                        arcPolygon.push_back(m_state.curPos());
                    valid = true;
                }
            }
            if (!valid)
                qWarning() << QString("Invalid arc in line %1.").arg(m_lineNum) << gLine;
            break;
        default:
            m_state.setCurPos({ x, y });
            m_path.push_back(m_state.curPos());
            return true;
            // break;
        }
        m_path.append(arcPolygon);
        return true;
    }
    return false;
}

bool Parser::parseEndOfFile(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnEndOfFile1(R"(^M[0]?[0123]\*)"); // fixed_string("^M[0]?[0123]\*");
    static constexpr ctll::fixed_string ptrnEndOfFile2(R"(^D0?2M0?[02]\*)"); // fixed_string("^D0?2M0?[02]\*");
    if (ctre::match<ptrnEndOfFile1>(data) || ctre::match<ptrnEndOfFile2>(data)) {
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& gLine) {
    // Number format
    // Example: %FSLAX24Y24*%
    // TODO: This is ignoring most of the format-> Implement the rest.

    auto data { toU16StrView(gLine) };
    static const QVector<QChar> zeroOmissionModeList { 'L', 'T' };
    static const QVector<QChar> coordinateValuesNotationList { 'A', 'I' };
    static constexpr ctll::fixed_string ptrnFormat(R"(^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$)"); // fixed_string("^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$");
    if (auto [whole, c1, c2, c3, c4, c5, c6] = ctre::match<ptrnFormat>(data); whole) {
        switch (zeroOmissionModeList.indexOf(CtreCapTo(c1).operator QString()[0])) {
        case OmitLeadingZeros:
            m_state.format()->zeroOmisMode = OmitLeadingZeros;
            break;
#ifdef DEPRECATED
        case OmitTrailingZeros:
            m_state.format()->zeroOmisMode = OmitTrailingZeros;
            break;
#endif
        }
        switch (coordinateValuesNotationList.indexOf(CtreCapTo(c2).operator QString()[0])) {
        case AbsoluteNotation:
            m_state.format()->coordValueNotation = AbsoluteNotation;
            break;
#ifdef DEPRECATED
        case IncrementalNotation:
            m_state.format()->coordValueNotation = IncrementalNotation;
            break;
#endif
        }
        m_state.format()->xInteger = CtreCapTo(c3);
        m_state.format()->xDecimal = CtreCapTo(c4);
        m_state.format()->yInteger = CtreCapTo(c5);
        m_state.format()->yDecimal = CtreCapTo(c6);

        int intVal = m_state.format()->xInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = m_state.format()->xDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = m_state.format()->yInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = m_state.format()->yDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnGCode(R"(^G([0]?[0-9]{2})\*$)"); // fixed_string("^G([0]?[0-9]{2})\*$");
    if (auto [whole, c1] = ctre::match<ptrnGCode>(data); whole) {
        switch (int { CtreCapTo(c1) }) {
        case G01:
            m_state.setInterpolation(Linear);
            m_state.setGCode(G01);
            break;
        case G02:
            m_state.setInterpolation(ClockwiseCircular);
            m_state.setGCode(G02);
            break;
        case G03:
            m_state.setInterpolation(CounterclockwiseCircular);
            m_state.setGCode(G03);
            break;
        case G04:
            m_state.setGCode(G04);
            break;
        case G36:
            addPath();
            m_state.setRegion(On);
            m_state.setGCode(G36);
            m_state.setDCode(D02);
            break;
        case G37:
            addPath();
            m_state.setRegion(Off);
            m_state.setGCode(G37);
            break;
#ifdef DEPRECATED
        case G70:
            m_state.format()->unitMode = Inches;
            m_state.setGCode(G70);
            break;
        case G71:
            m_state.format()->unitMode = Millimeters;
            m_state.setGCode(G71);
            break;
#endif
        case G74:
            m_state.setQuadrant(Single);
            m_state.setGCode(G74);
            break;
        case G75:
            m_state.setQuadrant(Multi);
            m_state.setGCode(G75);
            break;
#ifdef DEPRECATED
        case G90:
            m_state.format()->coordValueNotation = AbsoluteNotation;
            m_state.setGCode(G90);
            break;
        case G91:
            m_state.format()->coordValueNotation = IncrementalNotation;
            m_state.setGCode(G91);
#endif
            break;
        default:
            qWarning() << "Erroror, unknown G-code " << gLine; //<< match.capturedTexts();
            break;
        }
        return true;
    }
    static constexpr ctll::fixed_string ptrnGCodeComment(R"(^G0?4(.*)$)"); // fixed_string("^G0?4(.*)$");
    if (ctre::match<ptrnGCodeComment>(data)) {
        m_state.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static const mvector<QString> slImagePolarity { "POS", "NEG" };
    static constexpr ctll::fixed_string ptrnImagePolarity(R"(^%IP(POS|NEG)\*%$)"); // fixed_string("^%IP(POS|NEG)\*%$");
    if (auto [whole, c1] = ctre::match<ptrnImagePolarity>(data); whole) {
        switch (slImagePolarity.indexOf(CtreCapTo(c1))) {
        case Positive:
            m_state.setImgPolarity(Positive);
            break;
        case Negative:
            m_state.setImgPolarity(Negative);
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseLineInterpolation(const QString& gLine) {
    // G01 - Linear interpolation plus flashes
    // Operation code (D0x) missing is deprecated... oh well I will support it.
    // REGEX: r"^(?:G0?(1))?(?:X(-?\d+))?(?:Y(-?\d+))?(?:D0([123]))?\*$"
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnLineInterpolation(R"(^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$)"); // fixed_string("^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$");
    if (auto [whole, c1, c2, c3, c4] = ctre::match<ptrnLineInterpolation>(data); whole) {
        parsePosition(gLine);
        Operation dcode = m_state.dCode();
        if (c4.size())
            dcode = static_cast<Operation>(CtreCapTo(c4).toInt());

        switch (dcode) {
        case D01: // перемещение в указанную точку x-y с открытым затвором засветки
            m_state.setDCode(dcode);
            m_path.push_back(m_state.curPos());
            break;
        case D02: // перемещение в указанную точку x-y с закрытым затвором засветки
            addPath();
            m_state.setDCode(dcode);
            break;
        case D03: // перемещение в указанную точку x-y с закрытым затвором засветки и вспышка
            addPath();
            m_state.setDCode(dcode);
            addFlash();
            break;
        }

        return true;
    }
    return false;
}

bool Parser::parseLoadName(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnLoadName(R"(^%LN(.+)\*%$)"); // fixed_string("^%LN(.+)\*%$");
    if (ctre::match<ptrnLoadName>(data)) {
        //qDebug() << gLine << match.capturedTexts();
        return true;
    }
    return false;
}

bool Parser::parseDCode(const QString& gLine) {
    auto data { toU16StrView(gLine) };
    static constexpr ctll::fixed_string ptrnDCode(R"(^D0?([123])\*$)"); // fixed_string("^D0?([123])\*$");
    if (auto [whole, c1] = ctre::match<ptrnDCode>(data); whole) {
        switch (int { CtreCapTo(c1) }) {
        case D01:
            m_state.setDCode(D01);
            break;
        case D02:
            addPath();
            m_state.setDCode(D02);
            break;
        case D03:
            addPath();
            m_state.setDCode(D03);
            addFlash();
            break;
        }
        return true;
    }

    static constexpr ctll::fixed_string ptrnDCodeAperture(R"(^(?:G54)?D(\d+)\*$)"); // fixed_string("^(?:G54)?D(\d+)\*$");
    if (auto [whole, c1] = ctre::match<ptrnDCodeAperture>(data); whole) {
        addPath();
        m_state.setAperture(CtreCapTo(c1));
        m_state.setDCode(D02);
#ifdef DEPRECATED
        m_state.setGCode(G54);
#endif
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseUnitMode(const QString& gLine) {
    // Mode (IN/MM)
    // Example: %MOIN*%
    auto data { toU16StrView(gLine) };
    static const QVector<QString> slUnitType { "IN", "MM" };
    static constexpr ctll::fixed_string ptrnUnitMode(R"(^%MO(IN|MM)\*%$)"); // fixed_string("^%MO(IN|MM)\*%$");
    if (auto [whole, c1] = ctre::match<ptrnUnitMode>(data); whole) {
        switch (slUnitType.indexOf(QString { CtreCapTo(c1) })) {
        case Inches:
            m_state.format()->unitMode = Inches;
            break;
        case Millimeters:
            m_state.format()->unitMode = Millimeters;
            break;
        }
        return true;
    }
    return false;
}
} // namespace Gerber
