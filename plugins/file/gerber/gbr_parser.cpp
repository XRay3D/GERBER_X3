/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_parser.h"

#include "abstract_fileplugin.h"
#include "gbr_aperture.h"
#include "gbr_attraperfunction.h"
#include "gbr_attrfilefunction.h"
#include "gbr_file.h"
#include "myclipper.h"
#include "utils.h"
#include <QElapsedTimer>
#include <QMutex>
#include <algorithm>
#include <ctre.hpp>
// #include <st acktrace>
#include <boost/stacktrace.hpp>

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

QDebug operator<<(QDebug debug, const std::string_view& sw) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QByteArray(sw.data(), sw.size());
    return debug;
}

Parser::Parser(AbstractFilePlugin* afp)
    : afp{afp} {
}

void Parser::parseLines(const QString& gerberLines, const QString& fileName) {
    static std::mutex mutex;
    std::lock_guard lock{mutex};
    try {

        file = new File;
        file->setFileName(fileName);
        reset(); // clear parser data

        file->lines() = cleanAndFormatFile(gerberLines);
        file->graphicObjects_.reserve(file->lines().size());
        if(file->lines().empty())
            emit afp->fileError("", file->shortName() + "\n" + "Incorrect File!");

        emit afp->fileProgress(file->shortName(), static_cast<int>(file->lines().size()), 0);

        lineNum_ = 0;

        //        std::map<int, int> rel;
        QElapsedTimer t;
        t.start();
        for(const QString& gerberLine: file->lines()) {
            currentGerbLine_ = gerberLine;
            ++lineNum_;
            if(!(lineNum_ % 1000))
                emit afp->fileProgress(file->shortName(), 0, lineNum_);
            auto dummy = [](const QString& gLine) -> bool {
                auto data{std::u16string_view{gLine}};
                static constexpr ctll::fixed_string ptrnDummy(R"(^%(.{2})(.+)\*%$)"); // fixed_string("^%(.{2})(.+)\*%$");
                if(auto [whole, id, par] = ctre::match<ptrnDummy>(data); whole)       ///*regexp.match(gLine)); match.hasMatch()*/) {
                    return true;
                return false;
            };

            switch(gerberLine.front().toLatin1()) {
            case '%':
                if(parseAttributes(gerberLine)) continue;
                if(parseAperture(gerberLine)) continue;
                if(parseApertureBlock(gerberLine)) continue;
                if(parseApertureMacros(gerberLine)) continue;
                if(parseFormat(gerberLine)) continue;
                if(parseStepRepeat(gerberLine)) continue;
                if(parseTransformations(gerberLine)) continue;
                if(parseUnitMode(gerberLine)) continue;
                if(parseImagePolarity(gerberLine)) continue;
                if(parseLoadName(gerberLine)) continue;
                if(dummy(gerberLine)) continue;
                [[fallthrough]];
            case 'D': [[fallthrough]];
            case 'G':
                if(parseDCode(gerberLine)) continue;
                if(parseGCode(gerberLine)) continue;
                [[fallthrough]];
            case 'M':
                if(parseEndOfFile(gerberLine)) continue;
                [[fallthrough]];
            case 'X': [[fallthrough]];
            case 'Y':
            default:
                if(parseLineInterpolation(gerberLine)) continue;
                if(parseCircularInterpolation(gerberLine)) continue;
            }

            // Line didn`t match any pattern. Warn user.
            qWarning() << QString("Line ignored (%1): '" + gerberLine + "'").arg(lineNum_);

        } // End of file parsing

        qDebug() << file->shortName() << t.elapsed() << "ms";

        //        for (auto [key, val] : rel)

        if(file->graphicObjects_.empty()) {
            delete file;
        } else {

            if(attFile.function_ && attFile.function_->side_() == Attr::AbstrFileFunc::Side::Bot)
                file->setSide(Bottom);
            else if(file->shortName().contains("bot", Qt::CaseInsensitive))
                file->setSide(Bottom);
            else if(file->shortName().contains(".gb", Qt::CaseInsensitive) && !file->shortName().endsWith(".gbr", Qt::CaseInsensitive))
                file->setSide(Bottom);

            if(attFile.function_ && attFile.function_->function == Attr::File::Profile)
                file->setItemType(File::ApPaths);

            file->mergedPaths();
            file->components_ = components.values();
            file->groupedPaths();
            file->graphicObjects_.shrink_to_fit();
            emit afp->fileReady(file);
            emit afp->fileProgress(file->shortName(), 1, 1);
        }
    } catch(const QString& errStr) {
        qWarning() << "exeption Q:" << errStr;
        emit afp->fileError("", file->shortName() + "\n" + errStr);
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(const char* errStr) {
        qWarning() << "exeption Q:" << errStr;
        emit afp->fileError("", file->shortName() + "\n" + errStr);
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(const std::exception& e) {
        std::stringstream ss;
        auto trace = boost::stacktrace::stacktrace::from_current_exception();
        ss << trace;
        // ss << std::stacktrace::current();
        qWarning() << ss.str().c_str();
        qWarning() << "exeption E:" << e.what();
        emit afp->fileError("", file->shortName() + "\n" + e.what());
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(...) {
        QString errStr(QString("%1: %2").arg(errno).arg(strerror(errno)));
        qWarning() << "exeption S:" << errStr;
        emit afp->fileError("", file->shortName() + "\n" + errStr);
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    }
    reset(); // clear parser data
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
        switch(state) {
        case Macro:
            lastLine.push_back(val);
            if(lastLine.endsWith('%')) {
                gerberLines << lastLine;
                state = Data;
            }
            break;
        case Param:
            lastLine.push_back(val);
            if(lastLine.endsWith('%')) {
                for(QString& tmpline: lastLine.remove('%').split('*'))
                    if(!tmpline.isEmpty())
                        gerberLines << ('%' + tmpline + "*%");
                state = Data;
            }
            break;
        case Data:
            break;
        }
    };

    auto lastLineClose = [&gerberLines](State state, QString& val) -> void {
        switch(state) {
        case Macro:
            if(!val.endsWith('%'))
                val.push_back('%');
            if(!val.endsWith("*%"))
                val.insert(val.length() - 2, '*');
            gerberLines << val;
            break;
        case Param:
            for(QString& tmpline: val.remove('%').split('*'))
                if(!tmpline.isEmpty())
                    gerberLines << ('%' + tmpline + "*%");
            break;
        case Data:
            break;
        }
        val.clear();
    };

    auto dataClose = [&gerberLines](const QString& val) -> void {
        if(val.count('*') > 1) {
            for(QString& tmpline: val.split('*'))
                if(!tmpline.isEmpty())
                    gerberLines << (tmpline + '*');
        } else {
            gerberLines << val;
        }
    };
    for(QString& line: data.replace('\r', '\n').replace("\n\n", "\n").replace('\t', ' ').split('\n')) {
        line = line.trimmed();

        if(line.isEmpty())
            continue;
        if(line == '*')
            continue;

        if(line.startsWith('%') && line.endsWith('%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if(line.startsWith("%AM"))
                lastLineClose(Macro, line);
            else
                lastLineClose(Param, line);
            state = Data;
            continue;
        } else if(line.startsWith("%AM")) {
            lastLineClose(state, lastLine);
            state = Macro;
            lastLine = line;
            continue;
        } else if(line.startsWith('%')) {
            lastLineClose(state, lastLine);
            state = Param;
            lastLine = line;
            continue;
        } else if(line.endsWith('*') && line.length() > 1) {
            switch(state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                dataClose(line);
                continue;
            }
        } else {
            switch(state) {
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
    if(state_.interpolation() == CounterClockwiseCircular && stop <= start)
        stop += 2.0 * pi;
    if(state_.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    return abs(stop - start);
}

double Parser::toDouble(const QString& Str, bool scale, bool inchControl) {
    bool ok;
    double d = Str.toDouble(&ok);
    if(state_.file()->format().unitMode == Inches && inchControl)
        d *= 25.4;
    if(scale)
        d *= uScale;
    return d;
}

bool Parser::parseNumber(QString Str, /*Point::Type*/ int32_t& val, FormatDir dir) {
    bool flag = false;
    int sign = 1;
    if(!Str.isEmpty()) {
        const auto decimal = dir == FormatDir::X ? format.xDecimal : format.yDecimal;
        const auto integer = dir == FormatDir::X ? format.xInteger : format.yInteger;
        const auto maxLen = integer + decimal;

        if(Str.indexOf("+") == 0) {
            Str.remove(0, 1);
            sign = 1;
        }

        if(Str.indexOf("-") == 0) {
            Str.remove(0, 1);
            sign = -1;
        }

        if(Str.count('.'))
            Str.setNum(Str.split('.').first().toInt() + ("0." + Str.split('.').last()).toDouble());

        while(Str.length() < maxLen) {
            switch(format.zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QByteArray(maxLen - Str.length(), '0') + Str;
                // Str = "0" + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QByteArray(maxLen - Str.length(), '0');
                // Str += "0";
                break;
#endif
            }
        }
        val = static_cast</*Point::Type*/ int32_t>(toDouble(Str, true) * pow(10.0, -decimal) * sign);
        return true;
    }
    return flag;
}

void Parser::addPath() {
    if(path_.size() < 2) {
        resetStep();
        return;
    }

    int type = GrObject::FlDrawn;

    if(aperFunctionMap.contains(state_.aperture())
        && aperFunctionMap[state_.aperture()].function_->function == Attr::Aperture::ComponentOutline)
        components[refDes].addFootprint(~path_);

    switch(state_.region()) {
    case On:
        type |= GrObject::Polygon;
        state_.setType(Region);
        switch(abSrIdStack_.top().workingType) {
        case WorkingType::Normal: {
            auto& go = file->graphicObjects_.emplace_back(GrObject(goId_++, state_, createPolygon(), file, GrObject::Type(type), std::move(path_)));
            go.name = QString("D%1|Polygon").arg(state_.aperture()).toUtf8();
        } break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GrObject(goId_++, state_, createPolygon(), file, GrObject::Type(type), std::move(path_)));
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject(goId_++, state_, createPolygon(), file, GrObject::Type(type), std::move(path_)));
            break;
        }
        break;
    case Off:
        type |= GrObject::PolyLine;
        state_.setType(Line);
        switch(abSrIdStack_.top().workingType) {
        case WorkingType::Normal: {
            auto& go = file->graphicObjects_.emplace_back(GrObject(goId_++, state_, createLine(), file, GrObject::Type(type), std::move(path_)));
            go.name = QString("D%1|PolyLine").arg(state_.aperture()).toUtf8();
        } break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GrObject(stepRepeat_.storage.size(), state_, createLine(), file, GrObject::Type(type), std::move(path_)));
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject(apBlock(abSrIdStack_.top().apertureBlockId)->ApBlock::V::size(), state_, createLine(), file, GrObject::Type(type), std::move(path_)));
            break;
        }
        break;
    }

    resetStep();
}

void Parser::addFlash() {
    state_.setType(Aperture);
    if(!file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get() == nullptr) {
        QString str;
        for(const auto& [ap, apPtr]: file->apertures_)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    AbstractAperture* ap = file->apertures_[state_.aperture()].get();
    ap->setUsed();
    Paths paths(ap->draw(state_, abSrIdStack_.top().workingType != WorkingType::ApertureBlock));
    ////////////////////////////////// Draw Drill //////////////////////////////////
    if(ap->withHole())
        paths.emplace_back(ap->drawDrill(state_));

    int type = GrObject::FlStamp;

    switch(ap->type()) {
    case Circle:
        type |= GrObject::Circle;
        break;
    case Rectangle:
        type |= GrObject::Rect;
        break;
    case Obround:
        type |= GrObject::Elipse;
        break;
    case Polygon:
        type |= GrObject::Polygon;
        break;
    case Macro:
        type |= GrObject::Composite;
        break;
    case Block:
        type |= GrObject::Composite;
        break;
    default:
        break;
    }

    switch(abSrIdStack_.top().workingType) {
    case WorkingType::Normal: {
        auto& go = file->graphicObjects_.emplace_back(GrObject(goId_++,
            state_, std::move(paths), file, GrObject::Type(type)));
        go.name = QString("D%1|%2").arg(state_.aperture()).arg(ap->name()).toUtf8();
        go.pos = state_.curPos();
    } break;
    case WorkingType::StepRepeat:
        stepRepeat_.storage.append(GrObject(stepRepeat_.storage.size(),
            state_, std::move(paths), file, GrObject::Type(type)));
        break;
    case WorkingType::ApertureBlock:
        apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject(apBlock(abSrIdStack_.top().apertureBlockId)->ApBlock::V::size(), //
            state_, std::move(paths), file, GrObject::Type(type)));
        break;
    }
    if(aperFunctionMap.contains(state_.aperture()) && !refDes.isEmpty()) {
        switch(aperFunctionMap[state_.aperture()].function_->function) {
        case Attr::Aperture::ComponentPin:
            components[refDes].pins().back().pos = ~state_.curPos();
            break;
        case Attr::Aperture::ComponentMain:
            components[refDes].setReferencePoint(~state_.curPos());
            break;
        default:
            break;
        }
    }

    resetStep();
}

void Parser::reset() {
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
    ProgressCancel::reset();
}

void Parser::resetStep() {
    currentGerbLine_.clear();
    path_.clear();
    path_.push_back(state_.curPos());
}

Point Parser::parsePosition(const QString& xyStr) {
    auto data{std::u16string_view{xyStr}};
    static constexpr ctll::fixed_string ptrnPosition(R"((?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+)"); // fixed_string("(?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+");
    if(auto [whole, x, y] = ctre::match<ptrnPosition>(data /*xyStr*/); whole) {
        /*Point::Type*/ int32_t tmp = 0;
        if(x && parseNumber(CtreCapTo(x), tmp, FormatDir::X))
            format.coordValueNotation == AbsoluteNotation
                ? state_.curPos().x = tmp
                : state_.curPos().x += tmp;
        tmp = 0;
        if(y && parseNumber(CtreCapTo(y), tmp, FormatDir::Y))
            format.coordValueNotation == AbsoluteNotation
                ? state_.curPos().y = tmp
                : state_.curPos().y += tmp;
    }

    if(2.0e-310 > state_.curPos().x && state_.curPos().x > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.")
            .arg(QString::number(lineNum_), QString(currentGerbLine_));
    if(2.0e-310 > state_.curPos().y && state_.curPos().y > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.")
            .arg(QString::number(lineNum_), QString(currentGerbLine_));

    return state_.curPos();
}

Paths Parser::createLine() {
    if(file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get())
        file->apertures_[state_.aperture()].get()->setUsed();
    Paths solution;
    if(!file->apertures_.contains(state_.aperture())) {
        QString str;
        for(const auto& [ap, apPtr]: file->apertures_)
            str += QString::number(ap) + ", ";
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    if(file->apertures_[state_.aperture()]->type() == Rectangle) {
        if(Settings::wireMinkowskiSum()) {
            solution = Clipper2Lib::MinkowskiSum(file->apertures_[state_.aperture()]->draw(State{file}).front(), path_, {});
            std::ranges::for_each(std::views::join(solution), &SetZs);
        } else {
            auto rect = std::static_pointer_cast<ApRectangle>(file->apertures_[state_.aperture()]);
            if(!qFuzzyCompare(rect->width_, rect->height_)) // only square Aperture
                throw GbrObj::tr("Aperture D%1 (%2) not supported!\n"
                                 "Only square Aperture or use Minkowski Sum")
                    .arg(state_.aperture())
                    .arg(rect->name());
            double size = rect->width_ * uScale * state_.scaling();
            if(qFuzzyIsNull(size))
                return {};
            solution = Inflate({path_}, size, JoinType::Square, EndType::Square);
            std::ranges::for_each(std::views::join(solution), &SetZs);
        }
        if(state_.imgPolarity() == Negative)
            ReversePaths(solution);
    } else {
        double size = file->apertures_[state_.aperture()]->size() * uScale * state_.scaling();
        if(qFuzzyIsNull(size))
            return {};

        std::ranges::for_each(path_, &SetZs);
        // for(auto& pt: path_) SetZ(pt);
        if(Settings::wireMinkowskiSum())
            solution = Clipper2Lib::MinkowskiSum(CirclePath(size), path_, {});
        else {
            // if(path_.front() != path_.back())
            solution = Inflate({path_}, size, JoinType::Round, EndType::Round, 2.0, uScale / 1000);
            // else {
            //     Clipper clipper;
            //     clipper.AddSubject(
            //         Inflate({path_}, +size, JoinType::Round, EndType::Polygon, 2.0, uScale / 1000));
            //     clipper.AddClip(
            //         Inflate({path_}, -size, JoinType::Round, EndType::Polygon, 2.0, uScale / 1000));
            //     clipper.Execute(CT::Difference, FR::NonZero, solution);
            // }
        }

        //        ClipperOffset offset;
        //        offset.AddPath(path_, JoinType::Round, EndType::Round);
        //        solution = offset.Execute(size);
        if(state_.imgPolarity() == Negative)
            ReversePaths(solution);
    }
    return solution;
}

Paths Parser::createPolygon() {
    if(Area(path_) > 0.0) {
        if(state_.imgPolarity() == Negative)
            ReversePath(path_);
    } else {
        if(state_.imgPolarity() == Positive)
            ReversePath(path_);
    }
    std::ranges::for_each(path_, &SetZs);
    return {path_};
}

bool Parser::parseAperture(const QString& gLine) {
    /*
     *    Parse gerber aperture definition into dictionary of apertures.
     *    The following kinds and their attributes are supported:
     *    * Circular  (C)*: size (float)
     *    * Rectangle (R)*: width (float), height (float)
     *    * Obround   (O)*: width (float), height (float).
     *    * Polygon   (P)*: diameter{float}, vertices(int), [rotation(float)]
     *    * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)
     */
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnAperture(R"(^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$)"); // fixed_string("^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$");
    if(auto [whole, apId, apType, paramList_] = ctre::match<ptrnAperture>(data); whole) {
        int aperture{CtreCapTo(apId)};
        auto paramList{CtreCapTo(paramList_).toString().split('X')};
        double hole{}, rotation{};
        auto& apertures = file->apertures_;
        if(apType.size() == 1) {
            switch(*apType.data()) {
            case 'C': // Circle
                if(paramList.size() > 1)
                    hole = toDouble(paramList[1]);
                apertures[aperture] = std::make_shared<ApCircle>(toDouble(paramList[0]), hole, file);
                break;
            case 'R': // Rectangle
                if(paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                if(paramList.size() < 2)
                    paramList << paramList[0];
                apertures.try_emplace(aperture, std::make_shared<ApRectangle>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case 'O': // Obround
                if(paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                apertures.try_emplace(aperture, std::make_shared<ApObround>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case 'P': // Polygon
                if(paramList.length() > 2)
                    rotation = toDouble(paramList[2], false, false);
                if(paramList.length() > 3)
                    hole = toDouble(paramList[3]);
                apertures.try_emplace(aperture, std::make_shared<ApPolygon>(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, file));
                break;
            }
        } else {
            VarMap macroCoeff;
            for(int i = 0; i < paramList.size(); ++i)
                macroCoeff.emplace(QString("$%1").arg(i + 1), toDouble(paramList[i], false, false));
            apertures.try_emplace(aperture, std::make_shared<ApMacro>(CtreCapTo(apType).operator QString(), apertureMacro_[CtreCapTo(apType)].split('*'), macroCoeff, file));
        }
        if(attAper.function_)
            aperFunctionMap[aperture] = attAper;
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnApertureBlock(R"(^%ABD(\d+)\*%$)"); // fixed_string("^%ABD(\d+)\*%$");
    if(auto [whole, id] = ctre::match<ptrnApertureBlock>(data); whole) {
        abSrIdStack_.push({WorkingType::ApertureBlock, int(CtreCapTo(id))});
        file->apertures_.try_emplace(abSrIdStack_.top().apertureBlockId, std::make_shared<ApBlock>(file));
        return true;
    }
    if(gLine == "%AB*%") {
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
    auto data{std::u16string_view{gLine}};
    static const QVector<char> slTransformations{'P', 'M', 'R', 'S'};
    static const QVector<char> slLevelPolarity{'D', 'C'};
    static const QVector<QString> slLoadMirroring{"N", "X", "Y", "XY"};
    if(auto [whole, tr, val] = ctre::match<R"(^%L([PMRS])(.+)\*%$)">(data); whole) {
        const char trType = tr.data()[0];
        switch(slTransformations.indexOf(trType)) {
        case trPolarity:
            addPath();
            switch(slLevelPolarity.indexOf(val.data()[0])) {
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
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnStepRepeat(R"(^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$)"); // fixed_string("^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$");
    if(auto [whole, srx, sry, sri, srj] = ctre::match<ptrnStepRepeat>(data); whole) {
        if(abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        stepRepeat_.reset();
        stepRepeat_.x = CtreCapTo(srx);
        stepRepeat_.y = CtreCapTo(sry);
        stepRepeat_.i = CtreCapTo(sri), stepRepeat_.i *= uScale;
        stepRepeat_.j = CtreCapTo(srj), stepRepeat_.j *= uScale;
        if(format.unitMode == Inches) {
            stepRepeat_.i *= 25.4;
            stepRepeat_.j *= 25.4;
        }
        if(stepRepeat_.x > 1 || stepRepeat_.y > 1)
            abSrIdStack_.push({WorkingType::StepRepeat, 0});
        return true;
    }

    static constexpr ctll::fixed_string ptrnStepRepeatEnd(R"(^%SR\*%$)"); // fixed_string("^%SR\*%$");
    if(ctre::match<ptrnStepRepeatEnd>(data)) {
        if(abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        return true;
    }

    return false;
}

void Parser::closeStepRepeat() {
    addPath();
    for(int y = 0; y < stepRepeat_.y; ++y) {
        for(int x = 0; x < stepRepeat_.x; ++x) {
            const Point pt(static_cast</*Point::Type*/ int32_t>(stepRepeat_.i * x), static_cast</*Point::Type*/ int32_t>(stepRepeat_.j * y));
            for(GrObject& go: stepRepeat_.storage) {
                Paths paths(go.fill);
                for(Path& path: paths)
                    TranslatePath(path, pt);
                Path path(go.path);
                TranslatePath(path, pt);
                auto state = go.state;
                state.setCurPos({state.curPos().x + pt.x, state.curPos().y + pt.y});
                file->graphicObjects_.emplace_back(GrObject(goId_++, state, std::move(paths), go.gFile, go.type, std::move(path)));
            }
        }
    }
    stepRepeat_.reset();
    abSrIdStack_.pop();
}

ApBlock* Parser::apBlock(int32_t id) { return static_cast<ApBlock*>(file->apertures_[id].get()); }

bool Parser::parseApertureMacros(const QString& gLine) {
    // Start macro if(match, else not an AM, carry on.
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnApertureMacros(R"(^%AM([^\*]+)\*([^%]+)?(%)?$)"); // fixed_string("^%AM([^\*]+)\*([^%]+)?(%)?$");
    if(auto [whole, c1, c2, c3] = ctre::match<ptrnApertureMacros>(data); whole) {
        if(c1.size() && c2.size()) {
            apertureMacro_[CtreCapTo(c1)] = QString{CtreCapTo(c2)};
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnAttributes(R"(^%(T[FAOD])(\.?)(.*)\*%$)"); // fixed_string("^%(T[FAOD])(\.?)(.*)\*%$");
    if(auto [whole, c1, c2, c3] = ctre::match<ptrnAttributes>(data); whole) {
        QString cap[]{CtreCapTo(whole), CtreCapTo(c1), CtreCapTo(c2), CtreCapTo(c3)};
        switch(Attr::Command::value(cap[1])) {
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
            //                
            //                }
            //                //apertureAttributesStrings.append(matchAttr.cap(2));
            //            }
        case Attr::Command::TO: {
            for(int i = cap[3].indexOf('"'); i > -1; i = cap[3].indexOf('"'))
                cap[3].remove(i, 1);
            auto sl(cap[3].split(',')); // remove symbol "
            switch(int index = Comp::Component::value1(sl.first()); index) {
            case Comp::Component::N: // The CAD net name of a conducting object, e.g. Clk13.
                break;
            case Comp::Component::P: // Pins
                components[sl.value(1)].addPin({sl.value(2), sl.value(3), {}});
                break;
            case Comp::Component::C:
                switch(int key = Comp::Component::value2(sl.first())) {
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
    if(!(gLine.startsWith('G') || gLine.startsWith('X') || gLine.startsWith('Y')))
        return false;

    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnCircularInterpolation(R"(^(?:G0?([23]))?)"
                                                                  R"(X?([\+\-]?\d+)*)"
                                                                  R"(Y?([\+\-]?\d+)*)"
                                                                  R"(I?([\+\-]?\d+)*)"
                                                                  R"(J?([\+\-]?\d+)*)"
                                                                  R"([^D]*(?:D0?([12]))?\*$)");
    auto [whole, cg, cx, cy, ci, cj, cd] = ctre::match<ptrnCircularInterpolation>(data);
    if(!whole) return false;

    if(!cg && state_.gCode() != G02 && state_.gCode() != G03) return false;
    int32_t x{}, y{}, i{}, j{};
    cx ? parseNumber(CtreCapTo(cx), x, FormatDir::X)
       : x = state_.curPos().x;
    cy ? parseNumber(CtreCapTo(cy), y, FormatDir::Y)
       : y = state_.curPos().y;
    parseNumber(CtreCapTo(ci), i, FormatDir::X);
    parseNumber(CtreCapTo(cj), j, FormatDir::Y);
    // Set operation code if provided
    if(cd)
        state_.setDCode(static_cast<Operation>(CtreCapTo(cd).toInt()));
    int gc = cg ? int(CtreCapTo(cg)) : state_.gCode();
    switch(gc) {
    case G02:
        state_.setInterpolation(ClockwiseCircular);
        state_.setGCode(G02);
        break;
    case G03:
        state_.setInterpolation(CounterClockwiseCircular);
        state_.setGCode(G03);
        break;
    default:
        if(state_.interpolation() != ClockwiseCircular && state_.interpolation() != CounterClockwiseCircular) {
            qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(lineNum_);
            qWarning() << QString(gLine);
            state_.setCurPos({x, y});
            state_.setGCode(G01);
            return false;
        }
        break;
    }

    if(state_.quadrant() == Undef) {
        qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(lineNum_);
        qWarning() << QString(gLine);
        return true;
    }

    switch(state_.dCode()) {
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

    const Point arcStartPos = state_.curPos();

    const std::array centerPos{
        Point{arcStartPos.x + i, arcStartPos.y + j},
        Point{arcStartPos.x - i, arcStartPos.y + j},
        Point{arcStartPos.x + i, arcStartPos.y - j},
        Point{arcStartPos.x - i, arcStartPos.y - j}
    };

    bool valid = false;

    auto constructArc = [this, x, y](Point center, double radius, double start, double stop) {
        auto arcPath = arc(center, radius, start, stop, state_.interpolation());
        state_.setCurPos({x, y});
        //  Последняя точка в вычисленной дуге может иметь числовые ошибки.
        //  Точной конечной точкой является указанная (x, y). Замена.
        if(arcPath.size()) arcPath.back() = state_.curPos();
        else arcPath.emplace_back(state_.curPos());
        // SetZ(arcPath.back(), center);
        return arcPath;
    };

    Path arcPath;
    switch(state_.quadrant()) {
    case Multi: { // G75
        const double radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
        const double start = atan2(-j, -i); // Start angle
        const auto& center = centerPos.front();
        // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
        // Ч­то должно привести к образованию дуги в 360 градусов.
        const double stop = (arcStartPos == Point{x, y})
            ? start
            : atan2(-center.y + y, -center.x + x); // Stop angle

        arcPath = constructArc(center, radius1, start, stop);
    } break;
    case Single: // G74
        for(auto&& center: centerPos) {
            const double radius1 = sqrt(static_cast<double>(i) * i + static_cast<double>(j) * j);
            const double radius2 = sqrt(pow(center.x - x, 2.0) + pow(center.y - y, 2.0));
            // Убеждаемся, что радиус начала совпадает с радиусом конца.
            if(abs(radius2 - radius1) > (5e-4 * uScale)) continue; // Недействительный центр.
            // Correct i and j and return true; as with multi-quadrant.
            i = center.x - arcStartPos.x;
            j = center.y - arcStartPos.y;
            // Углы
            const double start = atan2(-j, -i);
            const double stop = atan2(-center.y + y, -center.x + x);
            const double angle = arcAngle(start, stop);
            if(angle < (pi + 1e-5) * 0.5) {
                arcPath = constructArc(center, radius1, start, stop);
                valid = true;
                break;
            }
        }
        if(valid) break;
        [[fallthrough]];
    default:
        if((path_.size() && (path_.back() != arcStartPos)) || path_.empty())
            path_.emplace_back(arcStartPos);
        SetZs(path_.back());
        state_.setCurPos({x, y});
        path_.emplace_back(state_.curPos());
        SetZs(path_.back());
        qWarning() << QString("Invalid arc in line %1.").arg(lineNum_) << gLine;
    }

    if(arcPath.size() && path_.size() && path_.back() == arcPath.front())
        path_.erase(--path_.end());
    path_ += std::move(arcPath);

    return true;
}

bool Parser::parseEndOfFile(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnEndOfFile1(R"(^M[0]?[0123]\*)"); // fixed_string("^M[0]?[0123]\*");
    static constexpr ctll::fixed_string ptrnEndOfFile2(R"(^D0?2M0?[02]\*)"); // fixed_string("^D0?2M0?[02]\*");
    if(ctre::match<ptrnEndOfFile1>(data) || ctre::match<ptrnEndOfFile2>(data)) {
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& gLine) {
    // Number format
    // Example: %FSLAX24Y24*%
    // TODO: This is ignoring most of the format-> Implement the rest.

    auto data{std::u16string_view{gLine}};
    static const QVector<QChar> zeroOmissionModeList{'L', 'T'};
    static const QVector<QChar> coordinateValuesNotationList{'A', 'I'};
    static constexpr ctll::fixed_string ptrnFormat(R"(^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$)"); // fixed_string("^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$");
    if(auto [whole, c1, c2, c3, c4, c5, c6] = ctre::match<ptrnFormat>(data); whole) {
        switch(zeroOmissionModeList.indexOf(c1.data()[0])) {
        case OmitLeadingZeros:
            format.zeroOmisMode = OmitLeadingZeros;
            file->format() = format;
            break;
#ifdef DEPRECATED
        case OmitTrailingZeros:
            format.zeroOmisMode = OmitTrailingZeros;
            file->format() = format;
            break;
#endif
        }
        switch(coordinateValuesNotationList.indexOf(c2.data()[0])) {
        case AbsoluteNotation:
            format.coordValueNotation = AbsoluteNotation;
            file->format() = format;
            break;
#ifdef DEPRECATED
        case IncrementalNotation:
            format.coordValueNotation = IncrementalNotation;
            file->format() = format;
            break;
#endif
        }
        format.xInteger = CtreCapTo(c3);
        format.xDecimal = CtreCapTo(c4);
        format.yInteger = CtreCapTo(c5);
        format.yDecimal = CtreCapTo(c6);

        file->format() = format;

        int intVal = format.xInteger;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = format.xDecimal;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = format.yInteger;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = format.yDecimal;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnGCode(R"(^G([0]?[0-9]{2})\*$)"); // fixed_string("^G([0]?[0-9]{2})\*$");
    if(auto [whole, c1] = ctre::match<ptrnGCode>(data); whole) {
        switch(int{CtreCapTo(c1)}) {
        case G01:
            state_.setInterpolation(Linear);
            state_.setGCode(G01);
            break;
        case G02:
            state_.setInterpolation(ClockwiseCircular);
            state_.setGCode(G02);
            break;
        case G03:
            state_.setInterpolation(CounterClockwiseCircular);
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
            format.unitMode = Inches;
            file->format() = format;
            state_.setGCode(G70);
            break;
        case G71:
            format.unitMode = Millimeters;
            file->format() = format;
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
            format.coordValueNotation = AbsoluteNotation;
            file->format() = format;
            state_.setGCode(G90);
            break;
        case G91:
            format.coordValueNotation = IncrementalNotation;
            file->format() = format;
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
    if(ctre::match<ptrnGCodeComment>(data)) {
        state_.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static const mvector<QString> slImagePolarity{"POS", "NEG"};
    static constexpr ctll::fixed_string ptrnImagePolarity(R"(^%IP(POS|NEG)\*%$)"); // fixed_string("^%IP(POS|NEG)\*%$");
    if(auto [whole, c1] = ctre::match<ptrnImagePolarity>(data); whole) {
        switch(slImagePolarity.indexOf(CtreCapTo(c1))) {
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
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnLineInterpolation(R"(^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$)"); // fixed_string("^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$");
    if(auto [whole, c1, c2, c3, c4] = ctre::match<ptrnLineInterpolation>(data); whole) {
        parsePosition(gLine);
        Operation dcode = state_.dCode();
        if(c4.size())
            dcode = static_cast<Operation>(CtreCapTo(c4).toInt());

        switch(dcode) {
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
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnLoadName(R"(^%LN(.+)\*%$)"); // fixed_string("^%LN(.+)\*%$");
    if(ctre::match<ptrnLoadName>(data))

        return true;
    return false;
}

bool Parser::parseDCode(const QString& gLine) {
    auto data{std::u16string_view{gLine}};
    static constexpr ctll::fixed_string ptrnDCode(R"(^D0?([123])\*$)"); // fixed_string("^D0?([123])\*$");
    if(auto [whole, c1] = ctre::match<ptrnDCode>(data); whole) {
        switch(int{CtreCapTo(c1)}) {
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
    if(auto [whole, c1] = ctre::match<ptrnDCodeAperture>(data); whole) {
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
    auto data{std::u16string_view{gLine}};
    static const QVector<QString> slUnitType{"IN", "MM"};
    static constexpr ctll::fixed_string ptrnUnitMode(R"(^%MO(IN|MM)\*%$)"); // fixed_string("^%MO(IN|MM)\*%$");
    if(auto [whole, c1] = ctre::match<ptrnUnitMode>(data); whole) {
        switch(slUnitType.indexOf(QString{CtreCapTo(c1)})) {
        case Inches:
            format.unitMode = Inches;
            file->format() = format;
            break;
        case Millimeters:
            format.unitMode = Millimeters;
            file->format() = format;
            break;
        }
        return true;
    }
    return false;
}

} // namespace Gerber
