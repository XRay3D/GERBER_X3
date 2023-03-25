// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_parser.h"

#include "file_plugin.h"
#include "gbr_aperture.h"
#include "gbr_attraperfunction.h"
#include "gbr_attrfilefunction.h"
#include "gbr_file.h"
#include "utils.h"
#include <QElapsedTimer>
#include <QMutex>
#include <ctre.hpp>

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
        file->graphicObjects_.reserve(file->lines().size());
        if (file->lines().empty())
            emit interface->fileError("", file->shortName() + "\n" + "Incorrect File!");

        emit interface->fileProgress(file->shortName(), static_cast<int>(file->lines().size()), 0);

        lineNum_ = 0;

        //        std::map<int, int> rel;
        QElapsedTimer t;
        t.start();
        for (const QString& gerberLine : file->lines()) {
            currentGerbLine_ = gerberLine;
            ++lineNum_;
            if (!(lineNum_ % 1000))
                emit interface->fileProgress(file->shortName(), 0, lineNum_);
            auto dummy = [](const QString& gLine) -> bool {
                auto data {toU16StrView(gLine)};
                static constexpr ctll::fixed_string ptrnDummy(R"(^%(.{2})(.+)\*%$)"); // fixed_string("^%(.{2})(.+)\*%$");
                if (auto [whole, id, par] = ctre::match<ptrnDummy>(data); whole) {    ///*regexp.match(gLine)); match.hasMatch()*/) {
                    // qDebug() << "dummy" << gLine << id.data() << par.data();
                    return true;
                }
                return false;
            };

            switch (gerberLine.front().toLatin1()) {
            case '%':
                if (parseAttributes(gerberLine))
                    continue;
                if (parseAperture(gerberLine))
                    continue;
                if (parseApertureBlock(gerberLine))
                    continue;
                if (parseApertureMacros(gerberLine))
                    continue;
                if (parseFormat(gerberLine))
                    continue;
                if (parseStepRepeat(gerberLine))
                    continue;
                if (parseTransformations(gerberLine))
                    continue;
                if (parseUnitMode(gerberLine))
                    continue;
                if (parseImagePolarity(gerberLine))
                    continue;
                if (parseLoadName(gerberLine))
                    continue;
                if (dummy(gerberLine))
                    continue;
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
            qWarning() << QString("Line ignored (%1): '" + gerberLine + "'").arg(lineNum_);

        } // End of file parsing

        qDebug() << file->shortName() << t.elapsed() << "ms";

        //        for (auto [key, val] : rel)
        //            qDebug() << key << '\t' << val;

        if (file->graphicObjects_.empty()) {
            delete file;
        } else {

            if (attFile.function_ && attFile.function_->side_() == Attr::AbstrFileFunc::Side::Bot) {
                file->setSide(Bottom);
            } else if (file->shortName().contains("bot", Qt::CaseInsensitive))
                file->setSide(Bottom);
            else if (file->shortName().contains(".gb", Qt::CaseInsensitive) && !file->shortName().endsWith(".gbr", Qt::CaseInsensitive))
                file->setSide(Bottom);

            if (attFile.function_ && attFile.function_->function == Attr::File::Profile)
                file->setItemType(File::ApPaths);

            file->mergedPaths();
            file->components_ = components.values();
            file->groupedPaths();
            file->graphicObjects_.shrink_to_fit();
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
                // qWarning() << "Хрен его знает:" << line;
                continue;
            }
        }
    }
    gerberLines.shrink_to_fit();
    return gerberLines;
}

double Parser::arcAngle(double start, double stop) {
    if (state_.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;
    if (state_.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    return qAbs(stop - start);
}

double Parser::toDouble(const QString& Str, bool scale, bool inchControl) {
    bool ok;
    double d = Str.toDouble(&ok);
    if (state_.file()->format().unitMode == Inches && inchControl)
        d *= 25.4;
    if (scale)
        d *= uScale;
    return d;
}

bool Parser::parseNumber(QString Str, Point::Type& val, int integer, int decimal) {
    bool flag = false;
    int sign = 1;
    if (!Str.isEmpty()) {
        if (!decimal)
            decimal = file->format().xDecimal;

        if (!integer)
            integer = file->format().xInteger;

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
            switch (file->format().zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QByteArray(integer + decimal - Str.length(), '0') + Str;
                // Str = "0" + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QByteArray(integer + decimal - Str.length(), '0');
                // Str += "0";
                break;
#endif
            }
        }
        val = static_cast<Point::Type>(toDouble(Str, true) * pow(10.0, -decimal) * sign);
        return true;
    }
    return flag;
}

void Parser::addPath() {
    if (path_.size() < 2) {
        resetStep();
        return;
    }
    switch (state_.region()) {
    case On:
        state_.setType(Region);
        switch (abSrIdStack_.top().workingType) {
        case WorkingType::Normal:
            file->graphicObjects_.emplace_back(GraphicObject(goId_++, state_, createPolygon(), file, path_));
            break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GraphicObject(goId_++, state_, createPolygon(), file, path_));
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GraphicObject(goId_++, state_, createPolygon(), file, path_));
            break;
        }
        break;
    case Off:
        state_.setType(Line);
        switch (abSrIdStack_.top().workingType) {
        case WorkingType::Normal:
            file->graphicObjects_.emplace_back(GraphicObject(goId_++, state_, createLine(), file, path_));
            break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GraphicObject(stepRepeat_.storage.size(), state_, createLine(), file, path_));
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GraphicObject(apBlock(abSrIdStack_.top().apertureBlockId)->size(), state_, createLine(), file, path_));
            break;
        }
        break;
    }
    if (aperFunctionMap.contains(state_.aperture()) && aperFunctionMap[state_.aperture()].function_->function == Attr::Aperture::ComponentOutline) {
        components[refDes].addFootprint(path_);
    }
    resetStep();
}

void Parser::addFlash() {
    state_.setType(Aperture);
    if (!file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get() == nullptr) {
        QString str;
        for (const auto& [ap, apPtr] : file->apertures_)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    AbstractAperture* ap = file->apertures_[state_.aperture()].get();
    ap->setUsed();
    Paths paths(ap->draw(state_, abSrIdStack_.top().workingType != WorkingType::ApertureBlock));
    ////////////////////////////////// Draw Drill //////////////////////////////////
    if (ap->withHole())
        paths.emplace_back(ap->drawDrill(state_));

    switch (abSrIdStack_.top().workingType) {
    case WorkingType::Normal:
        file->graphicObjects_.emplace_back(GraphicObject(goId_++, state_, paths, file));
        break;
    case WorkingType::StepRepeat:
        stepRepeat_.storage.append(GraphicObject(stepRepeat_.storage.size(), state_, paths, file));
        break;
    case WorkingType::ApertureBlock:
        apBlock(abSrIdStack_.top().apertureBlockId)->append(GraphicObject(apBlock(abSrIdStack_.top().apertureBlockId)->size(), state_, paths, file));
        break;
    }
    if (aperFunctionMap.contains(state_.aperture()) && !refDes.isEmpty()) {
        switch (aperFunctionMap[state_.aperture()].function_->function) {
        case Attr::Aperture::ComponentPin:
            components[refDes].pins().back().pos = state_.curPos();
            break;
        case Attr::Aperture::ComponentMain:
            components[refDes].setReferencePoint(state_.curPos());
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
    abSrIdStack_.clear();
    abSrIdStack_.push({WorkingType::Normal, 0});
    apertureMacro_.clear();
    currentGerbLine_.clear();
    goId_ = 0;
    path_.clear();
    state_ = State(file);
    stepRepeat_.reset();
    refDes.clear();
}

void Parser::resetStep() {
    currentGerbLine_.clear();
    path_.clear();
    path_.push_back(state_.curPos());
}

Point Parser::parsePosition(const QString& xyStr) {
    auto data {toU16StrView(xyStr)};
    static constexpr ctll::fixed_string ptrnPosition(R"((?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+)"); // fixed_string("(?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+");
    if (auto [whole, x, y] = ctre::match<ptrnPosition>(data /*xyStr*/); whole) {
        Point::Type tmp = 0;
        if (x && parseNumber(CtreCapTo(x), tmp, file->format().xInteger, file->format().xDecimal))
            file->format().coordValueNotation == AbsoluteNotation ? state_.curPos().x = tmp : state_.curPos().x += tmp;
        tmp = 0;
        if (y && parseNumber(CtreCapTo(y), tmp, file->format().yInteger, file->format().yDecimal))
            file->format().coordValueNotation == AbsoluteNotation ? state_.curPos().y = tmp : state_.curPos().y += tmp;
    }

    if (2.0e-310 > state_.curPos().x && state_.curPos().x > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.").arg(QString::number(lineNum_), QString(currentGerbLine_));
    if (2.0e-310 > state_.curPos().y && state_.curPos().y > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.").arg(QString::number(lineNum_), QString(currentGerbLine_));

    return state_.curPos();
}

Path Parser::arc(const Point& center, double radius, double start, double stop) {
    const double da_sign[4] = {0, 0, -1.0, +1.0};
    Path points;

    const int intSteps = App::settings().clpCircleSegments(radius * dScale); // MinStepsPerCircle;

    if (state_.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    else if (state_.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;

    double angle = qAbs(stop - start);
    double steps = std::max(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
    double delta_angle = da_sign[state_.interpolation()] * angle * 1.0 / steps;
    for (int i = 0; i < steps; i++) {
        double theta = start + delta_angle * (i + 1);
        points.emplace_back(Point(
            static_cast<Point::Type>(center.x + radius * cos(theta)),
            static_cast<Point::Type>(center.y + radius * sin(theta))));
    }

    return points;
}

Path Parser::arc(Point p1, Point p2, Point center) {
    double radius = sqrt(pow((center.x - p1.x), 2) + pow((center.y - p1.y), 2));
    double start = atan2(p1.y - center.y, p1.x - center.x);
    double stop = atan2(p2.y - center.y, p2.x - center.x);
    return arc(center, radius, start, stop);
}

Paths Parser::createLine() {
    if (file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get())
        file->apertures_[state_.aperture()].get()->setUsed();
    Paths solution;
    if (!file->apertures_.contains(state_.aperture())) {
        QString str;
        for (const auto& [ap, apPtr] : file->apertures_)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    if (file->apertures_[state_.aperture()]->type() == Rectangle) {
        if (Settings::wireMinkowskiSum())
            solution = Clipper2Lib::MinkowskiSum(file->apertures_[state_.aperture()]->draw(State {file}).front(), path_, {});
        else {
            auto rect = std::static_pointer_cast<ApRectangle>(file->apertures_[state_.aperture()]);
            if (!qFuzzyCompare(rect->width_, rect->height_)) // only square Aperture
                throw GbrObj::tr("Aperture D%1 (%2) not supported!\n"
                                 "Only square Aperture or use Minkowski Sum")
                    .arg(state_.aperture())
                    .arg(rect->name());
            double size = rect->width_ * uScale * state_.scaling();
            if (qFuzzyIsNull(size))
                return {};
            solution = Clipper2Lib::InflatePaths({path_}, size, JoinType::Square, EndType::Square);
        }

        if (state_.imgPolarity() == Negative)
            ReversePaths(solution);
    } else {
        double size = file->apertures_[state_.aperture()]->apSize() * uScale * state_.scaling();
        if (qFuzzyIsNull(size))
            return {};
        if (Settings::wireMinkowskiSum())
            solution = Clipper2Lib::MinkowskiSum(CirclePath(size), path_, {});
        else
            solution = Clipper2Lib::InflatePaths({path_}, size, JoinType::Round, EndType::Round);

        //        ClipperOffset offset;
        //        offset.AddPath(path_, JoinType::Round, EndType::Round);
        //        solution = offset.Execute(size);
        if (state_.imgPolarity() == Negative)
            ReversePaths(solution);
    }
    return solution;
}

Paths Parser::createPolygon() {
    if (Area(path_) > 0.0) {
        if (state_.imgPolarity() == Negative)
            ReversePath(path_);
    } else {
        if (state_.imgPolarity() == Positive)
            ReversePath(path_);
    }
    return {path_};
}

bool Parser::parseAperture(const QString& gLine) {
    /*
     *    Parse gerber aperture definition into dictionary of apertures.
     *    The following kinds and their attributes are supported:
     *    * Circular  (C)*: size (float)
     *    * Rectangle (R)*: width (float), height (float)
     *    * Obround   (O)*: width (float), height (float).
     *    * Polygon   (P)*: diameter(float), vertices(int), [rotation(float)]
     *    * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)
     */
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnAperture(R"(^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$)"); // fixed_string("^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$");
    if (auto [whole, apId, apType, paramList_] = ctre::match<ptrnAperture>(data); whole) {
        int aperture {CtreCapTo(apId)};
        auto paramList {CtreCapTo(paramList_).toString().split('X')};
        double hole {}, rotation {};
        auto& apertures = file->apertures_;
        if (apType.size() == 1) {
            switch (*apType.data()) {
            case 'C': // Circle
                if (paramList.size() > 1)
                    hole = toDouble(paramList[1]);
                apertures[aperture] = std::make_shared<ApCircle>(toDouble(paramList[0]), hole, file);
                break;
            case 'R': // Rectangle
                if (paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                if (paramList.size() < 2)
                    paramList << paramList[0];
                apertures.emplace(aperture, std::make_shared<ApRectangle>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case 'O': // Obround
                if (paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                apertures.emplace(aperture, std::make_shared<ApObround>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case 'P': // Polygon
                if (paramList.length() > 2)
                    rotation = toDouble(paramList[2], false, false);
                if (paramList.length() > 3)
                    hole = toDouble(paramList[3]);
                apertures.emplace(aperture, std::make_shared<ApPolygon>(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, file));
                break;
            }
        } else {
            VarMap macroCoeff;
            for (int i = 0; i < paramList.size(); ++i)
                macroCoeff.emplace(QString("$%1").arg(i + 1), toDouble(paramList[i], false, false));
            apertures.emplace(aperture, std::make_shared<ApMacro>(CtreCapTo(apType).operator QString(), apertureMacro_[CtreCapTo(apType)].split('*'), macroCoeff, file));
        }
        if (attAper.function_)
            aperFunctionMap[aperture] = attAper;
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnApertureBlock(R"(^%ABD(\d+)\*%$)"); // fixed_string("^%ABD(\d+)\*%$");
    if (auto [whole, id] = ctre::match<ptrnApertureBlock>(data); whole) {
        abSrIdStack_.push({WorkingType::ApertureBlock, int(CtreCapTo(id))});
        file->apertures_.emplace(abSrIdStack_.top().apertureBlockId, std::make_shared<ApBlock>(file));
        return true;
    }
    if (gLine == "%AB*%") {
        addPath();
        abSrIdStack_.pop();
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
    auto data {toU16StrView(gLine)};
    static const QVector<char> slTransformations {'P', 'M', 'R', 'S'};
    static const QVector<char> slLevelPolarity {'D', 'C'};
    static const QVector<QString> slLoadMirroring {"N", "X", "Y", "XY"};
    if (auto [whole, tr, val] = ctre::match<R"(^%L([PMRS])(.+)\*%$)">(data); whole) {
        const char trType = tr.data()[0];
        switch (slTransformations.indexOf(trType)) {
        case trPolarity:
            addPath();
            switch (slLevelPolarity.indexOf(val.data()[0])) {
            case Positive:
                state_.setImgPolarity(Positive);
                break;
            case Negative:
                state_.setImgPolarity(Negative);
                break;
            default:
                throw "bool Parser::parseTransformations(const SLI & gLine) - Polarity error!";
            }
            return true;
        case trMirror:
            state_.setMirroring(static_cast<Mirroring>(slLoadMirroring.indexOf(CtreCapTo(val))));
            return true;
        case trRotate:
            state_.setRotating(CtreCapTo(val));
            return true;
        case trScale:
            state_.setScaling(CtreCapTo(val));
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
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnStepRepeat(R"(^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$)"); // fixed_string("^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$");
    if (auto [whole, srx, sry, sri, srj] = ctre::match<ptrnStepRepeat>(data); whole) {
        if (abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        stepRepeat_.reset();
        stepRepeat_.x = CtreCapTo(srx);
        stepRepeat_.y = CtreCapTo(sry);
        stepRepeat_.i = CtreCapTo(sri), stepRepeat_.i *= uScale;
        stepRepeat_.j = CtreCapTo(srj), stepRepeat_.j *= uScale;
        if (file->format().unitMode == Inches) {
            stepRepeat_.i *= 25.4;
            stepRepeat_.j *= 25.4;
        }
        if (stepRepeat_.x > 1 || stepRepeat_.y > 1)
            abSrIdStack_.push({WorkingType::StepRepeat, 0});
        return true;
    }

    static constexpr ctll::fixed_string ptrnStepRepeatEnd(R"(^%SR\*%$)"); // fixed_string("^%SR\*%$");
    if (ctre::match<ptrnStepRepeatEnd>(data)) {
        if (abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        return true;
    }

    return false;
}

void Parser::closeStepRepeat() {
    addPath();
    for (int y = 0; y < stepRepeat_.y; ++y) {
        for (int x = 0; x < stepRepeat_.x; ++x) {
            const Point pt(static_cast<Point::Type>(stepRepeat_.i * x), static_cast<Point::Type>(stepRepeat_.j * y));
            for (GraphicObject& go : stepRepeat_.storage) {
                Paths paths(go.paths());
                for (Path& path : paths)
                    TranslatePath(path, pt);
                Path path(go.path());
                TranslatePath(path, pt);
                auto state = go.state();
                state.setCurPos({state.curPos().x + pt.x, state.curPos().y + pt.y});
                file->graphicObjects_.emplace_back(GraphicObject(goId_++, state, paths, go.gFile(), path));
            }
        }
    }
    stepRepeat_.reset();
    abSrIdStack_.pop();
}

ApBlock* Parser::apBlock(int id) { return static_cast<ApBlock*>(file->apertures_[id].get()); }

bool Parser::parseApertureMacros(const QString& gLine) {
    // Start macro if(match, else not an AM, carry on.
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnApertureMacros(R"(^%AM([^\*]+)\*([^%]+)?(%)?$)"); // fixed_string("^%AM([^\*]+)\*([^%]+)?(%)?$");
    if (auto [whole, c1, c2, c3] = ctre::match<ptrnApertureMacros>(data); whole) {
        if (c1.size() && c2.size()) {
            apertureMacro_[CtreCapTo(c1)] = QString {CtreCapTo(c2)};
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnAttributes(R"(^%(T[FAOD])(\.?)(.*)\*%$)"); // fixed_string("^%(T[FAOD])(\.?)(.*)\*%$");
    if (auto [whole, c1, c2, c3] = ctre::match<ptrnAttributes>(data); whole) {
        QString cap[] {CtreCapTo(whole), CtreCapTo(c1), CtreCapTo(c2), CtreCapTo(c3)};
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
            //                        case Attr::AperFunction::Main:
            //                        case Attr::AperFunction::Outline:
            //                        case Attr::AperFunction::Pin:
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
            switch (int index = Comp::Component::value1(sl.first()); index) {
            case Comp::Component::N: // The CAD net name of a conducting object, e.g. Clk13.
                break;
            case Comp::Component::P: // Pins
                components[sl.value(1)].addPin({sl.value(2), sl.value(3), {}});
                break;
            case Comp::Component::C:
                switch (int key = Comp::Component::value2(sl.first())) {
                case Comp::Component::Rot:
                case Comp::Component::Mfr:
                case Comp::Component::MPN:
                case Comp::Component::Val:
                case Comp::Component::Mnt:
                case Comp::Component::Ftp:
                case Comp::Component::PgN:
                case Comp::Component::Hgt:
                case Comp::Component::LbN:
                case Comp::Component::LbD:
                case Comp::Component::Sup:
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

    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnCircularInterpolation(R"(^(?:G0?([23]))?)"
                                                                  R"(X?([\+\-]?\d+)*)"
                                                                  R"(Y?([\+\-]?\d+)*)"
                                                                  R"(I?([\+\-]?\d+)*)"
                                                                  R"(J?([\+\-]?\d+)*)"
                                                                  R"([^D]*(?:D0?([12]))?\*$)");
    if (auto [whole, cg, cx, cy, ci, cj, cd] = ctre::match<ptrnCircularInterpolation>(data); whole) {
        if (!cg.size() && state_.gCode() != G02 && state_.gCode() != G03)
            return false;
        Point::Type x = 0, y = 0, i = 0, j = 0;
        cx.size() ? parseNumber(CtreCapTo(cx), x, file->format().xInteger, file->format().xDecimal) : x = state_.curPos().x;
        cy.size() ? parseNumber(CtreCapTo(cy), y, file->format().yInteger, file->format().yDecimal) : y = state_.curPos().y;
        parseNumber(CtreCapTo(ci), i, file->format().xInteger, file->format().xDecimal);
        parseNumber(CtreCapTo(cj), j, file->format().yInteger, file->format().yDecimal);
        // Set operation code if provided
        if (cd.size())
            state_.setDCode(static_cast<Operation>(CtreCapTo(cd).toInt()));
        int gc = cg ? int(CtreCapTo(cg)) : state_.gCode();
        switch (gc) {
        case G02:
            state_.setInterpolation(ClockwiseCircular);
            state_.setGCode(G02);
            break;
        case G03:
            state_.setInterpolation(CounterclockwiseCircular);
            state_.setGCode(G03);
            break;
        default:
            if (state_.interpolation() != ClockwiseCircular && state_.interpolation() != CounterclockwiseCircular) {
                qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(lineNum_);
                qWarning() << QString(gLine);
                state_.setCurPos({x, y});
                state_.setGCode(G01);
                return false;
            }
            break;
        }

        if (state_.quadrant() == Undef) {
            qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(lineNum_);
            qWarning() << QString(gLine);
            return true;
        }

        switch (state_.dCode()) {
        case D01:
            break;
        case D02: // Nothing created! Pen Up.
            state_.setDCode(D01);
            addPath();
            state_.setCurPos({x, y});
            return true;
        case D03: // Flash should not happen here
            state_.setCurPos({x, y});
            qWarning() << QString("Trying to flash within arc. (%1)").arg(lineNum_);
            return true;
        }

        const Point& curPos = state_.curPos();

        const Point centerPos[4] = {
            {curPos.x + i, curPos.y + j},
            {curPos.x - i, curPos.y + j},
            {curPos.x + i, curPos.y - j},
            {curPos.x - i, curPos.y - j}
        };

        bool valid = false;

        path_.push_back(state_.curPos());
        Path arcPolygon;
        switch (state_.quadrant()) {
        case Multi: // G75
        {
            const double radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
            const double start = atan2(-j, -i); // Start angle
            // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
            // Ч­то должно привести к образованию дуги в 360 градусов.
            const double stop = (state_.curPos() == Point(x, y)) ? start : atan2(-centerPos[0].y + y, -centerPos[0].x + x); // Stop angle

            arcPolygon = arc(Point(centerPos[0].x, centerPos[0].y), radius1, start, stop);
            // arcPolygon = arc(curPos, Point(x, y), centerPos[0]);
            //  Последняя точка в вычисленной дуге может иметь числовые ошибки.
            //  Точной конечной точкой является указанная (x, y). Заменить.
            state_.curPos() = Point {x, y};
            if (arcPolygon.size())
                arcPolygon.back() = state_.curPos();
            else
                arcPolygon.push_back(state_.curPos());
        } break;
        case Single: // G74
            for (int c = 0; c < 4; ++c) {
                const double radius1 = sqrt(static_cast<double>(i) * static_cast<double>(i) + static_cast<double>(j) * static_cast<double>(j));
                const double radius2 = sqrt(pow(centerPos[c].x - x, 2.0) + pow(centerPos[c].y - y, 2.0));
                // Убеждаемся, что радиус начала совпадает с радиусом конца.
                if (qAbs(radius2 - radius1) > (5e-4 * uScale)) // Недействительный центр.
                    continue;
                // Correct i and j and return true; as with multi-quadrant.
                i = centerPos[c].x - state_.curPos().x;
                j = centerPos[c].y - state_.curPos().y;
                // Углы
                const double start = atan2(-j, -i);
                const double stop = atan2(-centerPos[c].y + y, -centerPos[c].x + x);
                const double angle = arcAngle(start, stop);
                if (angle < (pi + 1e-5) * 0.5) {
                    arcPolygon = arc(Point(centerPos[c].x, centerPos[c].y), radius1, start, stop);
                    // Replace with exact values
                    state_.setCurPos({x, y});
                    if (arcPolygon.size())
                        arcPolygon.back() = state_.curPos();
                    else
                        arcPolygon.push_back(state_.curPos());
                    valid = true;
                }
            }
            if (!valid)
                qWarning() << QString("Invalid arc in line %1.").arg(lineNum_) << gLine;
            break;
        default:
            state_.setCurPos({x, y});
            path_.push_back(state_.curPos());
            return true;
            // break;
        }
        path_.append(arcPolygon);
        return true;
    }
    return false;
}

bool Parser::parseEndOfFile(const QString& gLine) {
    auto data {toU16StrView(gLine)};
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

    auto data {toU16StrView(gLine)};
    static const QVector<QChar> zeroOmissionModeList {'L', 'T'};
    static const QVector<QChar> coordinateValuesNotationList {'A', 'I'};
    static constexpr ctll::fixed_string ptrnFormat(R"(^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$)"); // fixed_string("^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$");
    if (auto [whole, c1, c2, c3, c4, c5, c6] = ctre::match<ptrnFormat>(data); whole) {
        switch (zeroOmissionModeList.indexOf(c1.data()[0])) {
        case OmitLeadingZeros:
            file->format().zeroOmisMode = OmitLeadingZeros;
            break;
#ifdef DEPRECATED
        case OmitTrailingZeros:
            file->format().zeroOmisMode = OmitTrailingZeros;
            break;
#endif
        }
        switch (coordinateValuesNotationList.indexOf(c2.data()[0])) {
        case AbsoluteNotation:
            file->format().coordValueNotation = AbsoluteNotation;
            break;
#ifdef DEPRECATED
        case IncrementalNotation:
            file->format().coordValueNotation = IncrementalNotation;
            break;
#endif
        }
        file->format().xInteger = CtreCapTo(c3);
        file->format().xDecimal = CtreCapTo(c4);
        file->format().yInteger = CtreCapTo(c5);
        file->format().yDecimal = CtreCapTo(c6);

        int intVal = file->format().xInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = file->format().xDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = file->format().yInteger;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        intVal = file->format().yDecimal;
        if (intVal < 0 || intVal > 8) {
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        }
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnGCode(R"(^G([0]?[0-9]{2})\*$)"); // fixed_string("^G([0]?[0-9]{2})\*$");
    if (auto [whole, c1] = ctre::match<ptrnGCode>(data); whole) {
        switch (int {CtreCapTo(c1)}) {
        case G01:
            state_.setInterpolation(Linear);
            state_.setGCode(G01);
            break;
        case G02:
            state_.setInterpolation(ClockwiseCircular);
            state_.setGCode(G02);
            break;
        case G03:
            state_.setInterpolation(CounterclockwiseCircular);
            state_.setGCode(G03);
            break;
        case G04:
            state_.setGCode(G04);
            break;
        case G36:
            addPath();
            state_.setRegion(On);
            state_.setGCode(G36);
            state_.setDCode(D02);
            break;
        case G37:
            addPath();
            state_.setRegion(Off);
            state_.setGCode(G37);
            break;
#ifdef DEPRECATED
        case G70:
            file->format().unitMode = Inches;
            state_.setGCode(G70);
            break;
        case G71:
            file->format().unitMode = Millimeters;
            state_.setGCode(G71);
            break;
#endif
        case G74:
            state_.setQuadrant(Single);
            state_.setGCode(G74);
            break;
        case G75:
            state_.setQuadrant(Multi);
            state_.setGCode(G75);
            break;
#ifdef DEPRECATED
        case G90:
            file->format().coordValueNotation = AbsoluteNotation;
            state_.setGCode(G90);
            break;
        case G91:
            file->format().coordValueNotation = IncrementalNotation;
            state_.setGCode(G91);
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
        state_.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static const mvector<QString> slImagePolarity {"POS", "NEG"};
    static constexpr ctll::fixed_string ptrnImagePolarity(R"(^%IP(POS|NEG)\*%$)"); // fixed_string("^%IP(POS|NEG)\*%$");
    if (auto [whole, c1] = ctre::match<ptrnImagePolarity>(data); whole) {
        switch (slImagePolarity.indexOf(CtreCapTo(c1))) {
        case Positive:
            state_.setImgPolarity(Positive);
            break;
        case Negative:
            state_.setImgPolarity(Negative);
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
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnLineInterpolation(R"(^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$)"); // fixed_string("^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$");
    if (auto [whole, c1, c2, c3, c4] = ctre::match<ptrnLineInterpolation>(data); whole) {
        parsePosition(gLine);
        Operation dcode = state_.dCode();
        if (c4.size())
            dcode = static_cast<Operation>(CtreCapTo(c4).toInt());

        switch (dcode) {
        case D01: // перемещение в указанную точку x-y с открытым затвором засветки
            state_.setDCode(dcode);
            path_.push_back(state_.curPos());
            break;
        case D02: // перемещение в указанную точку x-y с закрытым затвором засветки
            addPath();
            state_.setDCode(dcode);
            break;
        case D03: // перемещение в указанную точку x-y с закрытым затвором засветки и вспышка
            addPath();
            state_.setDCode(dcode);
            addFlash();
            break;
        }

        return true;
    }
    return false;
}

bool Parser::parseLoadName(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnLoadName(R"(^%LN(.+)\*%$)"); // fixed_string("^%LN(.+)\*%$");
    if (ctre::match<ptrnLoadName>(data)) {
        // qDebug() << gLine << match.capturedTexts();
        return true;
    }
    return false;
}

bool Parser::parseDCode(const QString& gLine) {
    auto data {toU16StrView(gLine)};
    static constexpr ctll::fixed_string ptrnDCode(R"(^D0?([123])\*$)"); // fixed_string("^D0?([123])\*$");
    if (auto [whole, c1] = ctre::match<ptrnDCode>(data); whole) {
        switch (int {CtreCapTo(c1)}) {
        case D01:
            state_.setDCode(D01);
            break;
        case D02:
            addPath();
            state_.setDCode(D02);
            break;
        case D03:
            addPath();
            state_.setDCode(D03);
            addFlash();
            break;
        }
        return true;
    }

    static constexpr ctll::fixed_string ptrnDCodeAperture(R"(^(?:G54)?D(\d+)\*$)"); // fixed_string("^(?:G54)?D(\d+)\*$");
    if (auto [whole, c1] = ctre::match<ptrnDCodeAperture>(data); whole) {
        addPath();
        state_.setAperture(CtreCapTo(c1));
        state_.setDCode(D02);
#ifdef DEPRECATED
        state_.setGCode(G54);
#endif
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseUnitMode(const QString& gLine) {
    // Mode (IN/MM)
    // Example: %MOIN*%
    auto data {toU16StrView(gLine)};
    static const QVector<QString> slUnitType {"IN", "MM"};
    static constexpr ctll::fixed_string ptrnUnitMode(R"(^%MO(IN|MM)\*%$)"); // fixed_string("^%MO(IN|MM)\*%$");
    if (auto [whole, c1] = ctre::match<ptrnUnitMode>(data); whole) {
        switch (slUnitType.indexOf(QString {CtreCapTo(c1)})) {
        case Inches:
            file->format().unitMode = Inches;
            break;
        case Millimeters:
            file->format().unitMode = Millimeters;
            break;
        }
        return true;
    }
    return false;
}

} // namespace Gerber
