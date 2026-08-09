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

#include "geo/boolean.h"
#include "geo/util.h"

#include "abstract_fileplugin.h"
#include "gbr_aperture.h"
#include "gbr_attraperfunction.h"
#include "gbr_attrfilefunction.h"
#include "gbr_file.h"
#include "gi_dbg.h"

#include "utils.h"
#include <QElapsedTimer>
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

namespace {

// Точки контура: дуги дробятся на хорды. Нужно лишь для следа ПРЯМОУГОЛЬНОЙ
// апертуры -- у круглой след строится точно, суммой Минковского с диском
// (Geo::Inflate), и дробить там нечего.
std::vector<QPointF> flatten(const Geo::Polyline& path) {
    std::vector<QPointF> points;
    if(path.empty()) return points;

    points.push_back(path.front());
    for(auto&& [from, to]: Geo::segments(path)) {
        if(const auto arc = Geo::arcOf(from, to, from.bulge)) {
            const int steps = std::max(2, static_cast<int>(std::ceil(std::abs(arc->theta) / (pi / 8))));
            for(int i = 1; i <= steps; ++i) points.push_back(arc->pointAt(double(i) / steps));
        } else
            points.push_back(to);
    }
    return points;
}

// Выпуклая оболочка (обход Эндрю), против часовой стрелки.
Geo::Polyline convexHull(std::vector<QPointF> points) {
    std::ranges::sort(points, [](QPointF l, QPointF r) {
        return l.x() != r.x() ? l.x() < r.x() : l.y() < r.y();
    });
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if(points.size() < 3) return {};

    auto cross = [](QPointF o, QPointF a, QPointF b) {
        return (a.x() - o.x()) * (b.y() - o.y()) - (a.y() - o.y()) * (b.x() - o.x());
    };

    std::vector<QPointF> hull(points.size() * 2);
    std::size_t k{};
    for(QPointF p: points) {
        while(k >= 2 && cross(hull[k - 2], hull[k - 1], p) <= 0.0) --k;
        hull[k++] = p;
    }
    for(std::size_t i = points.size() - 1, lower = k + 1; i > 0; --i) {
        QPointF p = points[i - 1];
        while(k >= lower && cross(hull[k - 2], hull[k - 1], p) <= 0.0) --k;
        hull[k++] = p;
    }
    hull.resize(k - 1);

    Geo::Polyline contour{std::from_range, hull};
    contour.closed = true;
    return contour;
}

// Сумма Минковского контура с ПРЯМОУГОЛЬНОЙ апертурой: каждый отрезок
// заметает выпуклую оболочку прямоугольника, поставленного в оба его конца, --
// шестиугольник, который заодно накрывает и стык. Круглым офсетом такое не
// заменить: у квадратной апертуры след шире по диагоналям, чем полоса
// постоянной ширины.
Geo::Polygons sweepRectangle(const Geo::Polyline& path, double width, double height) {
    const std::vector<QPointF> points = flatten(path);
    if(points.empty() || width <= 0.0 || height <= 0.0) return {};

    const double w = width / 2.0, h = height / 2.0;
    auto corners = [w, h](QPointF c) {
        return std::array{
            QPointF{c.x() - w, c.y() - h},
            QPointF{c.x() + w, c.y() - h},
            QPointF{c.x() + w, c.y() + h},
            QPointF{c.x() - w, c.y() + h}
        };
    };

    Geo::Polylines swept;
    if(points.size() == 1) {
        swept.push_back(Geo::rectangle(width, height, points.front()));
    } else {
        swept.reserve(points.size() - 1);
        for(std::size_t i = 1; i < points.size(); ++i) {
            std::vector<QPointF> box;
            box.reserve(8);
            for(QPointF p: corners(points[i - 1])) box.push_back(p);
            for(QPointF p: corners(points[i])) box.push_back(p);
            if(Geo::Polyline hull = convexHull(std::move(box)); !hull.empty())
                swept.push_back(std::move(hull));
        }
    }
    // Все куски -- тела, так что регион собирается одним объединением, а не
    // цепочкой из стольких же булевых операций.
    return Geo::Polygons{swept};
}

} // namespace

QDebug operator<<(QDebug debug, const std::string_view& sw) {
    QDebugStateSaver saver{debug};
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
            emit afp->fileError({}, file->shortName() + u'\n' + u"Incorrect File!");

        emit afp->fileProgress(file->shortName(), static_cast<int>(file->lines().size()), 0);

        lineNum_ = 0;

        // std::map<int, int> rel;
        QElapsedTimer t;
        t.start();
        for(const QString& gerberLine: file->lines()) {
            currentGerbLine_ = gerberLine;
            ++lineNum_;
            if(!(lineNum_ % 1000))
                emit afp->fileProgress(file->shortName(), 0, lineNum_);
            auto dummy = [](const QString& gLine) -> bool {
                static constexpr ctll::fixed_string ptrnDummy{R"(^%(.{2})(.+)\*%$)"};
                if(auto [whole, id, par] = ctre::match<ptrnDummy>(std::u16string_view{gLine}); whole) ///*regexp.match(gLine)); match.hasMatch()*/) {
                    return true;
                return false;
            };

            switch(gerberLine.front().unicode()) {
            case u'%':
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
            case u'D': [[fallthrough]];
            case u'G':
                if(parseDCode(gerberLine)) continue;
                if(parseGCode(gerberLine)) continue;
                [[fallthrough]];
            case u'M':
                if(parseEndOfFile(gerberLine)) continue;
                [[fallthrough]];
            case u'X': [[fallthrough]];
            case u'Y':
            default:
                if(parseLineInterpolation(gerberLine)) continue;
                if(parseCircularInterpolation(gerberLine)) continue;
            }

            // Line didn`t match any pattern. Warn user.
            qWarning() << u"Line ignored (%1): '"_s + gerberLine + u"'"_s.arg(lineNum_);
        } // End of file parsing

        qDebug()
            << file->shortName() << t.elapsed() << u"ms"_s;

        // for (auto [key, val] : rel)

        if(file->graphicObjects_.empty()) {
            delete file;
        } else {

            if(attFile.function_ && attFile.function_->side_() == Attr::AbstrFileFunc::Side::Bot)
                file->setSide(Bottom);
            else if(file->shortName().contains(u"bot"_s, Qt::CaseInsensitive))
                file->setSide(Bottom);
            else if(file->shortName().contains(u".gb"_s, Qt::CaseInsensitive)
                && !file->shortName().endsWith(u".gbr"_s, Qt::CaseInsensitive))
                file->setSide(Bottom);

            if(attFile.function_ && attFile.function_->function == Attr::File::Profile)
                file->setItemType(File::ApPaths);

            file->mergedCurves();
            file->components_ = components.values();
            file->groupedPaths();
            file->graphicObjects_.shrink_to_fit();
            emit afp->fileReady(file);
            emit afp->fileProgress(file->shortName(), 1, 1);
        }
    } catch(const QString& errStr) {
        qWarning() << u"exeption Q:"_s << errStr;
        emit afp->fileError({}, file->shortName() + u'\n' + errStr);
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(const char* errStr) {
        qWarning() << u"exeption Q:"_s << errStr;
        emit afp->fileError({}, file->shortName() + u'\n' + QString::fromUtf8(errStr));
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(const std::exception& e) {
        std::stringstream ss;
        auto trace = boost::stacktrace::stacktrace::from_current_exception();
        ss << trace;
        // ss << std::stacktrace::current();
        qWarning() << ss.str().c_str();
        qWarning() << u"exeption E:"_s << e.what();
        emit afp->fileError({}, file->shortName() + u'\n' + QString::fromUtf8(e.what()));
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    } catch(...) {
        QString errStr(u"%1: %2"_s.arg(errno).arg(strerror(errno)));
        qWarning() << u"exeption S:"_s << errStr;
        emit afp->fileError({}, file->shortName() + u'\n' + errStr);
        emit afp->fileProgress(file->shortName(), 1, 1);
        delete file;
    }
    reset(); // clear parser data
}

std::vector<QString> Parser::cleanAndFormatFile(QString data) {
    std::vector<QString> gerberLines;
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
            if(lastLine.endsWith(u'%')) {
                gerberLines.emplace_back(lastLine);
                state = Data;
            }
            break;
        case Param:
            lastLine.push_back(val);
            if(lastLine.endsWith(u'%')) {
                for(QString& tmpline: lastLine.remove(u'%').split(u'*'))
                    if(!tmpline.isEmpty())
                        gerberLines.emplace_back(u'%' + tmpline + u"*%"_s);
                state = Data;
            }
            break;
        case Data: break;
        }
    };

    auto lastLineClose = [&gerberLines](State state, QString& val) -> void {
        switch(state) {
        case Macro:
            if(!val.endsWith(u'%'))
                val.push_back(u'%');
            if(!val.endsWith(u"*%"_s))
                val.insert(val.length() - 2, u'*');
            gerberLines.emplace_back(val);
            break;
        case Param:
            for(QString& tmpline: val.remove(u'%').split(u'*'))
                if(!tmpline.isEmpty())
                    gerberLines.emplace_back(u'%' + tmpline + u"*%"_s);
            break;
        case Data: break;
        }
        val.clear();
    };

    auto dataClose = [&gerberLines](const QString& val) -> void {
        if(val.count(u'*') > 1) {
            for(QString& tmpline: val.split(u'*'))
                if(!tmpline.isEmpty())
                    gerberLines.emplace_back(tmpline + u'*');
        } else {
            gerberLines.emplace_back(val);
        }
    };
    for(QString& line: data
            .replace(u'\r', u'\n')
            .replace(u"\n\n"_s, u"\n"_s)
            .replace(u'\t', u' ')
            .split(u'\n')) {
        line = line.trimmed();

        if(line.isEmpty())
            continue;
        if(line == u'*')
            continue;

        if(line.startsWith(u'%') && line.endsWith(u'%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if(line.startsWith(u"%AM"_s))
                lastLineClose(Macro, line);
            else
                lastLineClose(Param, line);
            state = Data;
            continue;
        } else if(line.startsWith(u"%AM"_s)) {
            lastLineClose(state, lastLine);
            state    = Macro;
            lastLine = line;
            continue;
        } else if(line.startsWith(u'%')) {
            lastLineClose(state, lastLine);
            state    = Param;
            lastLine = line;
            continue;
        } else if(line.endsWith(u'*') && line.length() > 1) {
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
                // qWarning() << u"Хрен его знает:"_s << line;
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
    return d;
}

bool Parser::parseNumber(QString Str, /*PType*/ double& val, FormatDir dir) {
    bool flag{};
    int sign = 1;
    if(!Str.isEmpty()) {
        const auto decimal = dir == FormatDir::X ? file->format().xDecimal
                                                 : file->format().yDecimal;
        const auto integer = dir == FormatDir::X ? file->format().xInteger
                                                 : file->format().yInteger;
        const auto maxLen  = integer + decimal;

        if(Str.indexOf(u"+"_s) == 0) {
            Str.remove(0, 1);
            sign = 1;
        }

        if(Str.indexOf(u"-"_s) == 0) {
            Str.remove(0, 1);
            sign = -1;
        }

        if(Str.count(u'.'))
            Str.setNum(Str.split(u'.').first().toInt()
                + u"0.%1"_s.arg(Str.split(u'.').last()).toDouble());

        while(Str.length() < maxLen) {
            switch(file->format().zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QString(maxLen - Str.length(), u'0') + Str;
                // Str = u"0"_s + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QString(maxLen - Str.length(), u'0');
                // Str += u"0"_s;
                break;
#endif
            }
        }
        val = toDouble(Str, true) * pow(10.0, -decimal) * sign;
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
        components[refDes].addFootprint(path_);

    switch(state_.region()) {
    case On:
        type |= GrObject::Polygon;
        state_.setType(Region);
        switch(abSrIdStack_.top().workingType) {
        case WorkingType::Normal: {
            auto& go = file->graphicObjects_.emplace_back(GrObject{
                goId_++,
                state_,
                createPolygon(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            go.name  = u"D%1|Polygon"_s.arg(state_.aperture());
        } break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GrObject{
                goId_++,
                state_,
                createPolygon(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject{
                goId_++,
                state_,
                createPolygon(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            break;
        }
        break;
    case Off:
        type |= GrObject::PolyLine;
        state_.setType(Line);
        switch(abSrIdStack_.top().workingType) {
        case WorkingType::Normal: {
            auto& go = file->graphicObjects_.emplace_back(GrObject{
                goId_++,
                state_,
                createLine(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            go.name  = u"D%1|PolyLine"_s.arg(state_.aperture());
        } break;
        case WorkingType::StepRepeat:
            stepRepeat_.storage.append(GrObject{
                static_cast<int32_t>(stepRepeat_.storage.size()),
                state_,
                createLine(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            break;
        case WorkingType::ApertureBlock:
            apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject{
                static_cast<int32_t>(apBlock(abSrIdStack_.top().apertureBlockId)->ApBlock::V::size()),
                state_,
                createLine(),
                file,
                GrObject::Type(type),
                std::move(path_),
            });
            break;
        }
        break;
    }

    resetStep();
} // namespace Gerber

void Parser::addFlash() {
    state_.setType(Aperture);
    if(!file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get() == nullptr) {
        QString str;
        for(const auto& [ap, apPtr]: file->apertures_)
            str += QString::number(ap) + u", "_s;
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    AbstractAperture* ap = file->apertures_[state_.aperture()].get();
    ap->setUsed();
    Geo::Polygons paths(ap->draw(state_, abSrIdStack_.top().workingType != WorkingType::ApertureBlock));
    ////////////////////////////////// Draw Drill //////////////////////////////////

    // Отверстие апертуры -- дырка в её собственном теле, поэтому вычитается
    // всегда, независимо от полярности кадра: полярность применяется уже ко
    // всей вспышке при слиянии слоя. Прежде это выражалось контуром обратного
    // обхода, подложенным в общий плоский список.
    if(ap->withHole())
        paths -= Geo::Polygons{Geo::Polylines{ap->drawDrill(state_)}};

    int type = GrObject::FlStamp;

    switch(ap->type()) {
    case Circle   : type |= GrObject::Circle; break;
    case Rectangle: type |= GrObject::Rect; break;
    case Obround  : type |= GrObject::Elipse; break;
    case Polygon  : type |= GrObject::Polygon; break;
    case Macro    : type |= GrObject::Composite; break;
    case Block    : type |= GrObject::Composite; break;
    default       : break;
    }

    switch(abSrIdStack_.top().workingType) {
    case WorkingType::Normal: {
        auto& go = file->graphicObjects_.emplace_back(
            GrObject{
                goId_++,
                state_,
                std::move(paths),
                file,
                GrObject::Type(type),
            });
        go.name = u"D%1|%2"_s.arg(state_.aperture()).arg(ap->name());
        go.pos  = state_.curPos();
    } break;
    case WorkingType::StepRepeat:
        stepRepeat_.storage.append(
            GrObject{
                static_cast<int32_t>(stepRepeat_.storage.size()),
                state_,
                std::move(paths),
                file,
                GrObject::Type(type),
            });
        break;
    case WorkingType::ApertureBlock:
        apBlock(abSrIdStack_.top().apertureBlockId)->append(GrObject{
            static_cast<int32_t>(apBlock(abSrIdStack_.top().apertureBlockId)->ApBlock::V::size()), //
            state_,
            std::move(paths),
            file,
            GrObject::Type(type),
        });
        break;
    }
    if(aperFunctionMap.contains(state_.aperture()) && !refDes.isEmpty()) {
        switch(aperFunctionMap[state_.aperture()].function_->function) {
        case Attr::Aperture::ComponentPin : components[refDes].pins().back().pos = state_.curPos(); break;
        case Attr::Aperture::ComponentMain: components[refDes].setReferencePoint(state_.curPos()); break;
        default                           : break;
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
    // ProgressCancel::reset();
}

void Parser::resetStep() {
    currentGerbLine_.clear();
    // path_.clear() -- метод БАЗОВОГО std::vector, свои поля Polyline
    // (closed, width) не трогает. Область (G36...G37) помечает path_
    // замкнутым через close() (см. createPolygon); без явного сброса здесь
    // эта отметка переживала бы clear() и утекала в КАЖДЫЙ следующий трек
    // до конца файла -- один closed=true на всё, что после первой область.
    // Тогда штрих накопленного пути читается как готовое тело контура
    // (Polygons(Polylines) в Geo::Inflate), и след трассы вместо тонкой
    // ленты вдоль пути заливается целиком, будто она сама была областью.
    path_ = Geo::Polyline{};
    path_.push_back(state_.curPos());
}

QPointF Parser::parsePosition(const QString& xyStr) {
    static constexpr ctll::fixed_string ptrnPosition{R"((?:G[01]{1,2})?(?:X([\+\-]?\d*\.?\d+))?(?:Y([\+\-]?\d*\.?\d+))?.+)"};
    if(auto [whole, x, y] = ctre::match<ptrnPosition>(std::u16string_view{xyStr}); whole) {
        double tmp{};
        if(x && parseNumber(CtreCapTo(x), tmp, FormatDir::X))
            file->format().coordValueNotation == AbsoluteNotation
                ? state_.curPos().rx() = tmp
                : state_.curPos().rx() += tmp;
        tmp = 0;
        if(y && parseNumber(CtreCapTo(y), tmp, FormatDir::Y))
            file->format().coordValueNotation == AbsoluteNotation
                ? state_.curPos().ry() = tmp
                : state_.curPos().ry() += tmp;
    }

    if(2.0e-310 > state_.curPos().x() && state_.curPos().x() > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.")
            .arg(QString::number(lineNum_), QString(currentGerbLine_));
    if(2.0e-310 > state_.curPos().y() && state_.curPos().y() > 0.0)
        throw GbrObj::tr("line num %1: '%2', error value.")
            .arg(QString::number(lineNum_), QString(currentGerbLine_));

    return state_.curPos();
}

Geo::Polygons Parser::createLine() {
    if(file->apertures_.contains(state_.aperture()) && file->apertures_[state_.aperture()].get())
        file->apertures_[state_.aperture()].get()->setUsed();
    Geo::Polygons solution;
    if(!file->apertures_.contains(state_.aperture())) {
        QString str;
        for(const auto& [ap, apPtr]: file->apertures_)
            str += QString::number(ap) + u", "_s;
        throw GbrObj::tr("Aperture %1 not found! Available %2").arg(state_.aperture()).arg(str);
    }

    AbstractAperture* ap = file->apertures_[state_.aperture()].get();
    if(ap->type() == Rectangle) {
        auto rect       = static_cast<ApRectangle*>(ap);
        const double sc = state_.scaling();
        solution        = sweepRectangle(path_, rect->width_ * sc, rect->height_ * sc);
    } else {
        // Круглая апертура: след -- сумма Минковского контура с диском, а это
        // ровно раздувание. delta у Geo::Inflate -- ПОЛНАЯ ширина, как и у
        // прежнего Inflate64, который делил её пополам внутри.
        const double size = ap->size() * state_.scaling();
        if(qFuzzyIsNull(size)) return {};
        solution = Geo::Inflate(Geo::Polylines{path_}, size);
    }

    // Полярность здесь НЕ применяется -- ровно как у вспышек
    // (AbstractAperture::draw): она живёт в state графического объекта, и
    // вычитание отрицательных штрихов делает слой при слиянии (File::merge).
    //
    // Прежний разворот контуров выражал то же самое в плоском списке и был
    // безобиден. Инверсия точного региона -- НЕ он: она считает настоящее
    // дополнение и уходит в неограниченную часть плоскости, у которой нет
    // внешней границы. Такой объект merge() потом разбирает по контурам,
    // видит одни лишь дырки -- и не вычитает ничего вовсе.

    // new Gi::Debug{solution};

    return {solution};
}

Geo::Polygons Parser::createPolygon() {
    // Ориентацию контура приводит к канону сам конструктор Polygon, а
    // полярность применяет слой при слиянии -- разворачивать здесь нечего.
    path_.close();
    Geo::Polygon polygon{path_};
    // polygon.invert();
    return {polygon};
}

bool Parser::parseAperture(const QString& gLine) {
    // Parse gerber aperture definition into dictionary of apertures.
    // The following kinds and their attributes are supported:
    // * Circular  (C)*: size (float)
    // * Rectangle (R)*: width (float), height (float)
    // * Obround   (O)*: width (float), height (float).
    // * Polygon   (P)*: diameter{float}, vertices(int), [rotation(float)]
    // * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)
    static constexpr ctll::fixed_string ptrnAperture{R"(^%ADD(\d\d+)([a-zA-Z_$\.][a-zA-Z0-9_$\.\-]*),?(.*)\*%$)"};
    if(auto [whole, apId, apType, paramList_] = ctre::match<ptrnAperture>(std::u16string_view{gLine}); whole) {
        int aperture{CtreCapTo(apId)};
        auto paramList{CtreCapTo(paramList_).toString().split(u'X')};
        double hole{}, rotation{};
        auto& apertures = file->apertures_;
        if(apType.size() == 1) {
            switch(*apType.data()) {
            case u'C': // Circle
                if(paramList.size() > 1)
                    hole = toDouble(paramList[1]);
                apertures[aperture] = std::make_shared<ApCircle>(toDouble(paramList[0]), hole, file);
                break;
            case u'R': // Rectangle
                if(paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                if(paramList.size() < 2)
                    paramList << paramList[0];
                apertures.try_emplace(aperture, std::make_shared<ApRectangle>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case u'O': // Obround
                if(paramList.size() > 2)
                    hole = toDouble(paramList[2]);
                apertures.try_emplace(aperture, std::make_shared<ApObround>(toDouble(paramList[0]), toDouble(paramList[1]), hole, file));
                break;
            case u'P': // Polygon
                if(paramList.length() > 2)
                    rotation = toDouble(paramList[2], false, false);
                if(paramList.length() > 3)
                    hole = toDouble(paramList[3]);
                apertures.try_emplace(aperture, std::make_shared<ApPolygon>(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, file));
                break;
            }
        } else {
            VarMap macroCoeff;
            for(int i{}; i < paramList.size(); ++i)
                macroCoeff.emplace(u"$%1"_s.arg(i + 1), toDouble(paramList[i], false, false));
            apertures.try_emplace(aperture, std::make_shared<ApMacro>(CtreCapTo(apType).operator QString(), apertureMacro_[CtreCapTo(apType)].split(u'*'), macroCoeff, file));
        }
        if(attAper.function_)
            aperFunctionMap[aperture] = attAper;
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine) {
    static constexpr ctll::fixed_string ptrnApertureBlock{R"(^%ABD(\d+)\*%$)"};
    if(auto [whole, id]
        = ctre::match<ptrnApertureBlock>(std::u16string_view{gLine});
        whole) {
        abSrIdStack_.push({WorkingType::ApertureBlock, int(CtreCapTo(id))});
        file->apertures_.try_emplace(abSrIdStack_.top().apertureBlockId, std::make_shared<ApBlock>(file));
        return true;
    }
    if(gLine == u"%AB*%"_s) {
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
    static const QVector<char> slTransformations{'P', 'M', 'R', 'S'};
    static const QVector<char> slLevelPolarity{'D', 'C'};
    static const QVector<QString> slLoadMirroring{u"N"_s, u"X"_s, u"Y"_s, u"XY"_s};
    if(auto [whole, tr, val]
        = ctre::match<R"(^%L([PMRS])(.+)\*%$)">(std::u16string_view{gLine});
        whole) {
        const char trType = tr.data()[0];
        switch(slTransformations.indexOf(trType)) {
        case trPolarity:
            addPath();
            switch(slLevelPolarity.indexOf(val.data()[0])) {
            case Positive: state_.setImgPolarity(Positive); break;
            case Negative: state_.setImgPolarity(Negative); break;
            default      : throw u"bool Parser::parseTransformations(const SLI & gLine) - Polarity error!"_s;
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
    std::u16string_view data{gLine};
    static constexpr ctll::fixed_string ptrnStepRepeat{R"(^%SRX(\d+)Y(\d+)I(.\d*\.?\d*)J(.\d*\.?\d*)\*%$)"};
    if(auto [whole, x, y, i, j] = ctre::match<ptrnStepRepeat>(data); whole) {
        if(abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        stepRepeat_.reset();
        stepRepeat_.x = CtreCapTo(x);
        stepRepeat_.y = CtreCapTo(y);
        stepRepeat_.i = CtreCapTo(i);
        stepRepeat_.j = CtreCapTo(j);
        if(file->format().unitMode == Inches) {
            stepRepeat_.i *= 25.4;
            stepRepeat_.j *= 25.4;
        }
        if(stepRepeat_.x > 1 || stepRepeat_.y > 1)
            abSrIdStack_.push({WorkingType::StepRepeat, 0});
        return true;
    }

    static constexpr ctll::fixed_string ptrnStepRepeatEnd{R"(^%SR\*%$)"};
    if(ctre::match<ptrnStepRepeatEnd>(data)) {
        if(abSrIdStack_.top().workingType == WorkingType::StepRepeat)
            closeStepRepeat();
        return true;
    }

    return false;
}

void Parser::closeStepRepeat() {
    addPath();

    for(int y{}; y < stepRepeat_.y; ++y) {
        for(int x{}; x < stepRepeat_.x; ++x) {
            const QPointF pt{stepRepeat_.i * x, stepRepeat_.j * y};
            const QTransform shift = QTransform::fromTranslate(pt.x(), pt.y());
            for(GrObject& go: stepRepeat_.storage) {
                // Заливка живёт в точном домене и на месте не правится --
                // пересобирается из перенесённых контуров; контур же обычная
                // полилиния, ему хватает сдвига.
                Geo::Polygons fill = Geo::transformed(go.fill, shift);
                Geo::Polyline path = go.path;
                Geo::translate(path, pt);

                State state = go.state;
                state.setCurPos(state.curPos() + pt);

                file->graphicObjects_.emplace_back(GrObject{
                    goId_++,
                    state,
                    std::move(fill),
                    go.gFile,
                    go.type,
                    std::move(path),
                });
            }
        }
    }

    stepRepeat_.reset();
    abSrIdStack_.pop();
}

ApBlock* Parser::apBlock(int32_t id) {
    return static_cast<ApBlock*>(file->apertures_[id].get());
}

bool Parser::parseApertureMacros(const QString& gLine) {
    // Start macro if(match, else not an AM, carry on.
    static constexpr ctll::fixed_string ptrnApertureMacros{R"(^%AM([^\*]+)\*([^%]+)?(%)?$)"};
    if(auto [whole, c1, c2, c3]
        = ctre::match<ptrnApertureMacros>(std::u16string_view{gLine});
        whole) {
        if(c1 && c2) {
            apertureMacro_[QString{c1}] = QString{c2};
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine) {
    static constexpr ctll::fixed_string ptrnAttributes{R"(^%(T[FAOD])(\.?)(.*)\*%$)"};
    if(auto [whole, c1, c2, c3]
        = ctre::match<ptrnAttributes>(std::u16string_view{gLine});
        whole) {
        QStringList cap{QString{whole}, QString{c1}, QString{c2}, QString{c3}};
        switch(Attr::Command::value(cap[1])) {
        case Attr::Command::TF: attFile.parse(cap[3].split(u',')); break;
        case Attr::Command::TA:
            attAper.parse(cap[3].split(u','));
            break;
            // break;
            // {
            // QStringList sl(matchAttr.cap(3).split(u','));
            // int index = Attr::Aperture::value(sl.first());
            // switch (index) {
            // case Attr::Aperture::AperFunction:
            // if (sl.size() > 1) {
            // switch (int key = Attr::AperFunction::value(sl[1])) {
            // case Attr::AperFunction::Main:
            // case Attr::AperFunction::Outline:
            // case Attr::AperFunction::Pin:
            // aperFunction = key;
            // break;
            // default: // aperFunction = -1;
            // }
            // }
            // break;
            // case Attr::Aperture::DrillTolerance:
            // case Attr::Aperture::FlashText:
            // default: // ;
            // }
            // //apertureAttributesStrings.append(matchAttr.cap(2));
            // }
        case Attr::Command::TO: {
            for(int i = cap[3].indexOf(u'"'); i > -1; i = cap[3].indexOf(u'"'))
                cap[3].remove(i, 1);
            auto sl(cap[3].split(u',')); // remove symbol "
            switch(int index = Comp::Component::value1(sl.first()); index) {
            case Comp::Component::N: break;                                                                 // The CAD net name of a conducting object, e.g. Clk13.
            case Comp::Component::P: components[sl.value(1)].addPin({sl.value(2), sl.value(3), {}}); break; // Pins
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
                case Comp::Component::Sup: components[refDes].setData(key, sl); break;
                default: // static const QRegularExpression rx(u"(\\[0-9a-fA-F]{4})"_s);
                    // int pos{};
                    // auto match(rx.match(sl.last(), pos));
                    // while (match.hasMatch()) { //(pos = rx.indexIn(sl.last(), pos)) != -1) {
                    // sl.last()                    .replace(pos++, 5, QChar(match.captured(1).right(4).toUShort(nullptr, 16)));
                    // auto match(rx.match(sl.last(), pos));
                    // }
                    // while ((pos = rx.indexIn(sl.last(), pos)) != -1) {
                    // sl.last().replace(pos++, 5, QChar(rx.cap(1).right(4).toUShort(nullptr, 16)));
                    // }
                    refDes = sl.last();
                    components[refDes].setRefdes(refDes);
                }
                break;
            default: qDebug() << gLine << cap[0];
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
    if(!(gLine.startsWith(u'G') || gLine.startsWith(u'X') || gLine.startsWith(u'Y')))
        return false;

    static constexpr ctll::fixed_string ptrnCircularInterpolation{
        R"(^(?:G0?([23]))?)"
        R"(X?([\+\-]?\d+)*)"
        R"(Y?([\+\-]?\d+)*)"
        R"(I?([\+\-]?\d+)*)"
        R"(J?([\+\-]?\d+)*)"
        R"([^D]*(?:D0?([12]))?\*$)"};
    auto [whole, cg, cx, cy, ci, cj, cd]
        = ctre::match<ptrnCircularInterpolation>(std::u16string_view{gLine});
    if(!whole) return false;

    if(!cg && state_.gCode() != G02 && state_.gCode() != G03) return false;
    double x{}, y{}, i{}, j{};
    cx ? parseNumber(CtreCapTo(cx), x, FormatDir::X)
       : x = state_.curPos().x();
    cy ? parseNumber(CtreCapTo(cy), y, FormatDir::Y)
       : y = state_.curPos().y();
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
            qWarning() << u"Found arc without circular interpolation mode defined. (%1)"_s.arg(lineNum_);
            qWarning() << QString(gLine);
            state_.setCurPos({x, y});
            state_.setGCode(G01);
            return false;
        }
        break;
    }

    if(state_.quadrant() == Undef) {
        qWarning() << u"Found arc without preceding quadrant specification G74 or G75. (%1)"_s.arg(lineNum_);
        qWarning() << QString(gLine);
        return true;
    }

    switch(state_.dCode()) {
    case D01: break;
    case D02: // Nothing created! Pen Up.
        state_.setDCode(D01);
        addPath();
        state_.setCurPos({x, y});
        return true;
    case D03: // Flash should not happen here
        state_.setCurPos({x, y});
        qWarning() << u"Trying to flash within arc. (%1)"_s.arg(lineNum_);
        return true;
    }

    const QPointF arcStartPos = state_.curPos();

    const std::array centerPos{
        QPointF{arcStartPos.x() + i, arcStartPos.y() + j},
        QPointF{arcStartPos.x() - i, arcStartPos.y() + j},
        QPointF{arcStartPos.x() + i, arcStartPos.y() - j},
        QPointF{arcStartPos.x() - i, arcStartPos.y() - j}
    };

    bool valid{};

    auto constructArc = [this, x, y](QPointF center, double radius, double start, double stop) {
        // Geo::arc ждёт ЗНАКОВЫЙ размах, а у Gerber направление задано режимом
        // интерполяции отдельно от углов. Совпадение начала с концом -- полный
        // оборот, а не вырожденная дуга: нормализация в непустой полуинтервал
        // даёт ровно это.
        double sweep = stop - start;
        if(state_.interpolation() == CounterClockwiseCircular)
            while(sweep <= 0.0) sweep += 2.0 * pi;
        else
            while(sweep >= 0.0) sweep -= 2.0 * pi;

        Geo::Polyline arcPath = Geo::arc(center, radius, start, sweep);

        state_.setCurPos({x, y});
        // Последняя точка вычисленной дуги может нести числовую погрешность --
        // точный конец задан в кадре.
        //
        // Полный оборот приходит ЗАМКНУТЫМ, а замкнутость у полилинии -- флаг,
        // и он потеряется: дуга уходит в общий путь кадра (path_) простым
        // добавлением вершин, а тот замкнут не всегда. Поэтому оборот здесь
        // размыкается явно -- повтором начальной точки, чтобы оба полукруга
        // остались сегментами пути; иначе от окружности доезжает половина.
        if(arcPath.closed) {
            arcPath.closed = false;
            arcPath.emplace_back(static_cast<const QPointF&>(arcPath.front()));
        } else if(arcPath.size())
            static_cast<QPointF&>(arcPath.back()) = state_.curPos();
        else
            arcPath.emplace_back(state_.curPos());

        return arcPath;
    };

    Geo::Polyline arcPath;
    switch(state_.quadrant()) {
    case Multi: { // G75
        const double radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
        const double start   = atan2(-j, -i); // Start angle
        const auto& center   = centerPos.front();
        // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
        // Ч­то должно привести к образованию дуги в 360 градусов.
        const double stop = (arcStartPos == QPointF{x, y})
            ? start
            : atan2(-center.y() + y, -center.x() + x); // Stop angle

        arcPath = constructArc(center, radius1, start, stop);
    } break;
    case Single: // G74
        for(auto&& center: centerPos) {
            const double radius1 = sqrt(static_cast<double>(i) * i + static_cast<double>(j) * j);
            const double radius2 = sqrt(pow(center.x() - x, 2.0) + pow(center.y() - y, 2.0));
            // Убеждаемся, что радиус начала совпадает с радиусом конца.
            if(abs(radius2 - radius1) > 5e-4) continue; // Недействительный центр.
            // Correct i and j and return true; as with multi-quadrant.
            i = center.x() - arcStartPos.x();
            j = center.y() - arcStartPos.y();
            // Углы
            const double start = atan2(-j, -i);
            const double stop  = atan2(-center.y() + y, -center.x() + x);
            const double angle = arcAngle(start, stop);
            if(angle < (pi + 1e-5) * 0.5) {
                arcPath = constructArc(center, radius1, start, stop);
                valid   = true;
                break;
            }
        }
        if(valid) break;
        [[fallthrough]];
    default:
        if((path_.size() && (path_.back() != arcStartPos)) || path_.empty())
            path_.emplace_back(arcStartPos);
        state_.setCurPos({x, y});
        path_.emplace_back(state_.curPos());
        qWarning() << u"Invalid arc in line %1."_s.arg(lineNum_) << gLine;
    }

    path_.append_range(std::move(arcPath));

    return true;
}

bool Parser::parseEndOfFile(const QString& gLine) {
    std::u16string_view data{gLine};
    static constexpr ctll::fixed_string ptrnEndOfFile1{R"(^M[0]?[0123]\*)"};
    static constexpr ctll::fixed_string ptrnEndOfFile2{R"(^D0?2M0?[02]\*)"};
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

    static const QVector<QChar> zeroOmissionModeList{u'L', u'T'};
    static const QVector<QChar> coordinateValuesNotationList{u'A', u'I'};
    static constexpr ctll::fixed_string ptrnFormat{R"(^%FS([LT]?)([AI]?)X(\d)(\d)Y(\d)(\d)\*%$)"};
    if(auto [whole, c1, c2, c3, c4, c5, c6]
        = ctre::match<ptrnFormat>(std::u16string_view{gLine});
        whole) {
        switch(zeroOmissionModeList.indexOf(c1.data()[0])) {
        case OmitLeadingZeros: file->format().zeroOmisMode = OmitLeadingZeros; break;
#ifdef DEPRECATED
        case OmitTrailingZeros: file->format().zeroOmisMode = OmitTrailingZeros; break;
#endif
        }
        switch(coordinateValuesNotationList.indexOf(c2.data()[0])) {
        case AbsoluteNotation: file->format().coordValueNotation = AbsoluteNotation; break;
#ifdef DEPRECATED
        case IncrementalNotation: file->format().coordValueNotation = IncrementalNotation; break;
#endif
        }
        file->format().xInteger = CtreCapTo(c3);
        file->format().xDecimal = CtreCapTo(c4);
        file->format().yInteger = CtreCapTo(c5);
        file->format().yDecimal = CtreCapTo(c6);

        int intVal = file->format().xInteger;
        if(intVal < 0 || intVal > 8)
            throw u"Modifiers '"_s + gLine + u"' XY is out of bounds 0в‰¤Nв‰¤7"_s;
        intVal = file->format().xDecimal;
        if(intVal < 0 || intVal > 8)
            throw u"Modifiers '"_s + gLine + u"' XY is out of bounds 0в‰¤Nв‰¤7"_s;
        intVal = file->format().yInteger;
        if(intVal < 0 || intVal > 8)
            throw u"Modifiers '"_s + gLine + u"' XY is out of bounds 0в‰¤Nв‰¤7"_s;
        intVal = file->format().yDecimal;
        if(intVal < 0 || intVal > 8)
            throw u"Modifiers '"_s + gLine + u"' XY is out of bounds 0в‰¤Nв‰¤7"_s;
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine) {
    std::u16string_view data{gLine};
    static constexpr ctll::fixed_string ptrnGCode{R"(^G([0]?[0-9]{2})\*$)"};
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
        case G04: state_.setGCode(G04); break;
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
        default: qWarning() << u"Erroror, unknown G-code "_s << gLine; //<< match.capturedTexts(); break;
        }
        return true;
    }
    static constexpr ctll::fixed_string ptrnGCodeComment{R"(^G0?4(.*)$)"};
    if(ctre::match<ptrnGCodeComment>(data)) {
        state_.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine) {
    static const std::vector<QString> slImagePolarity{u"POS"_s, u"NEG"_s};
    static constexpr ctll::fixed_string ptrnImagePolarity{R"(^%IP(POS|NEG)\*%$)"};
    if(auto [whole, c1]
        = ctre::match<ptrnImagePolarity>(std::u16string_view{gLine});
        whole) {
        switch(indexOf(slImagePolarity, CtreCapTo(c1))) {
        case Positive: state_.setImgPolarity(Positive); break;
        case Negative: state_.setImgPolarity(Negative); break;
        }
        return true;
    }
    return false;
}

bool Parser::parseLineInterpolation(const QString& gLine) {
    // G01 - Linear interpolation plus flashes
    // Operation code (D0x) missing is deprecated... oh well I will support it.
    // REGEX: ru"^(?:G0?(1))?(?:X(-?\d+))?(?:Y(-?\d+))?(?:D0([123]))?\*$"_s
    static constexpr ctll::fixed_string ptrnLineInterpolation{
        R"(^(?:G0?(1))?(?=.*X([\+\-]?\d+))?(?=.*Y([\+\-]?\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\*$)"};
    if(auto [whole, c1, c2, c3, c4]
        = ctre::match<ptrnLineInterpolation>(std::u16string_view{gLine});
        whole) {
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
    static constexpr ctll::fixed_string ptrnLoadName{R"(^%LN(.+)\*%$)"};
    if(ctre::match<ptrnLoadName>(std::u16string_view{gLine}))
        return true;
    return false;
}

bool Parser::parseDCode(const QString& gLine) {
    std::u16string_view data{gLine};
    static constexpr ctll::fixed_string ptrnDCode{R"(^D0?([123])\*$)"};
    if(auto [whole, c1] = ctre::match<ptrnDCode>(data); whole) {
        switch(int{CtreCapTo(c1)}) {
        case D01: state_.setDCode(D01); break;
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

    static constexpr ctll::fixed_string ptrnDCodeAperture{R"(^(?:G54)?D(\d+)\*$)"};
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
    static const QVector<QString> slUnitType{u"IN"_s, u"MM"_s};
    static constexpr ctll::fixed_string ptrnUnitMode{R"(^%MO(IN|MM)\*%$)"};
    if(auto [whole, c1]
        = ctre::match<ptrnUnitMode>(std::u16string_view{gLine});
        whole) {
        switch(slUnitType.indexOf(QString{CtreCapTo(c1)})) {
        case Inches     : file->format().unitMode = Inches; break;
        case Millimeters: file->format().unitMode = Millimeters; break;
        }
        return true;
    }
    return false;
}

} // namespace Gerber
