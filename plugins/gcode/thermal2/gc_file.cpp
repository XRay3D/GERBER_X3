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
 ********************************************************************************/
#include "gc_file.h"

#include "gc_node.h"
#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "gi_drill.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "graphicsview.h"
#include "project.h"
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QTextStream>

void calcArcs(Path path) {
    return;
    //    if (!qApp->applicationDirPath().contains("GERBER_X3/bin"))
    //        return;
    //    auto addPoint = [](const QPointF& pos, const QColor& color = QColor(255, 255, 255)) {
    //        QGraphicsLineItem* item;
    //        item = App::graphicsView()->scene()->addLine(.0, +.1, .0, -.1, QPen(color, 0.0));
    //        item->setPos(pos);
    //        item = App::graphicsView()->scene()->addLine(+.1, .0, -.1, .0, QPen(color, 0.0));
    //        item->setPos(pos);
    //    };

    //    QPolygonF polyOfCenters;
    //    std::vector<QLineF> normals;
    //    QPointF center;
    //    QPointF beg;
    //    QPointF end;
    //    int ctr {};

    //    constexpr double centerError = 0.2;
    //    constexpr int minSegCtr = 3;

    //    struct Center {
    //        QPointF pt;
    //        int i {};
    //    };

    //    std::vector<Center> centers;

    //    CleanPolygon(path, uScale * 0.001);
    //    QPolygonF poly = path;
    //    for (int i {}, size { static_cast<int>(poly.size()) }; i < size; ++i) {
    //        QLineF line(QLineF(poly[i], end = poly[(i + 1) % size]).center(), poly[i]);
    //        line = line.normalVector();
    //        if (beg.isNull())
    //            beg = poly[i];
    //        //                App::graphicsView()->scene()->addLine(line, QPen(QColor(0, 255, 0), 0.0));
    //        if (normals.size()) {
    //            QPointF intersectionPoint;
    // #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    //            normals.back().intersects(line, &intersectionPoint);
    // #else
    //            normals.back().intersect(line, &intersectionPoint);
    // #endif
    //            if (polyOfCenters.size() && QLineF(polyOfCenters.back(), intersectionPoint).length() < centerError) {
    //                center += intersectionPoint;
    //                addPoint(intersectionPoint, Qt::darkGray);
    //                ++ctr;
    //                centers.emplace_back(intersectionPoint, i);
    //            } else if (ctr > minSegCtr) {
    //                center /= ctr;
    //                addPoint(center, Qt::red);
    //                double r = QLineF(center, beg).length();
    //                QRectF rect(-r, -r, +r * 2, +r * 2);

    //                App::graphicsView()->scene()->addEllipse(rect, QPen(Qt::red, 0.0), Qt::NoBrush)->setPos(center);
    //                ctr = {};
    //                center = {};
    //                beg = {};
    //                end = {};
    //            } else {
    //                ctr = {};
    //                center = {};
    //                beg = {};
    //                end = {};
    //            }
    //            polyOfCenters.push_back(intersectionPoint);
    //        }
    //        normals.emplace_back(line);
    //    }
}

namespace GCode {

File::File()
    : GCFile() { }

File::File(const Pathss& toolPathss, GCodeParams&& gcp, const Paths& pocketPaths)
    : GCFile(std::move(gcp))
    , pocketPaths_(pocketPaths)
    , toolPathss_(toolPathss) {
    if (gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

bool File::save(const QString& name) {
    if (name.isEmpty())
        return false;

    initSave();
    addInfo();
    statFile();
    genGcodeAndTile();
    endFile();

    setLastDir(name);
    name_ = name;
    QFile file(name_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for (QString& s : lines_) {
            if (!s.isEmpty())
                str.push_back(s);
            if (!str.endsWith('\n'))
                str.push_back("\n");
        }
        out << str;
    } else
        return false;
    file.close();
    return true;
}

void File::saveDrill(const QPointF& offset) {
    QPolygonF path(normalizedPaths(offset, toolPathss_.front()).front());

    const mvector<double> depths(getDepths());

    for (QPointF& point : path) {
        startPath(point);
        size_t i = 0;
        while (true) {
            lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            if (++i == depths.size())
                break;
            lines_.emplace_back(formated({g0(), z(0.0)}));
        }
        endPath();
    }
}

void File::saveLaserPocket(const QPointF& offset) {
    saveLaserProfile(offset);
}

void File::saveMillingPocket(const QPointF& offset) {
    lines_.emplace_back(Settings::spindleOn());

    mvector<mvector<QPolygonF>> toolPathss(normalizedPathss(offset));

    const mvector<double> depths(getDepths());

    for (mvector<QPolygonF>& paths : toolPathss) {
        startPath(paths.front().front());
        for (size_t i = 0; i < depths.size(); ++i) {
            lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            bool skip = true;
            for (auto& path : paths) {
                for (QPointF& point : path) {
                    if (skip)
                        skip = false;
                    else
                        lines_.emplace_back(formated({g1(), x(point.x()), y(point.y()), feed(feedRate())}));
                }
            }
            for (size_t j = paths.size() - 2; j != std::numeric_limits<size_t>::max() && i < depths.size() - 1; --j) {
                QPointF& point = paths[j].back();
                lines_.emplace_back(formated({g0(), x(point.x()), y(point.y())}));
            }
            if (paths.size() > 1 && i < (depths.size() - 1))
                lines_.emplace_back(formated({g0(), x(paths.front().front().x()), y(paths.front().front().y())}));
        }
        endPath();
    }
}

void File::saveMillingProfile(const QPointF& offset) {
    if (gcp_.gcType == Raster) {
        saveMillingRaster(offset);
        return;
    }

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for (auto& paths : pathss) {
        for (size_t i = 0; i < depths.size(); ++i) {
            for (size_t j = 0; j < paths.size(); ++j) {
                QPolygonF& path = paths[j];
                if (path.front() == path.last()) { // make complete depth and remove from worck
                    startPath(path.front());
                    for (size_t k = 0; k < depths.size(); ++k) {
                        lines_.emplace_back(formated({g1(), z(depths[k]), feed(plungeRate())}));
                        auto sp(savePath(path, spindleSpeed()));
                        lines_.append(sp);
                    }
                    endPath();
                    paths.erase(paths.begin() + j--);
                } else {
                    startPath(path.front());
                    lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
                    auto sp(savePath(path, spindleSpeed()));
                    lines_.append(sp);
                    endPath();
                }
            }
        }
    }
}

void File::saveLaserProfile(const QPointF& offset) {
    lines_.emplace_back(Settings::laserDynamOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    for (auto& paths : pathss) {
        for (auto& path : paths) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
            endPath();
        }
    }
}

void File::saveMillingRaster(const QPointF& offset) {
    lines_.emplace_back(Settings::spindleOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for (auto& paths : pathss) {
        for (size_t i = 0; i < depths.size(); ++i) {
            for (auto& path : paths) {
                startPath(path.front());
                lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
                auto sp(savePath(path, spindleSpeed()));
                lines_.append(sp);
                endPath();
            }
        }
    }
}

void File::saveLaserHLDI(const QPointF& offset) {
    lines_.emplace_back(Settings::laserConstOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    int i = 0;

    lines_.emplace_back(formated({g0(), x(pathss.front().front().front().x()), y(pathss.front().front().front().y()), z(0.0)}));

    for (QPolygonF& path : pathss.front()) {
        if (i++ % 2) {
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
        } else {
            auto sp(savePath(path, 0));
            lines_.append(sp);
        }
    }
    if (pathss.size() > 1) {
        lines_.emplace_back(Settings::laserDynamOn());
        for (QPolygonF& path : pathss.back()) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
            endPath();
        }
    }
}

const GCodeParams& File::gcp() const { return gcp_; }

mvector<mvector<QPolygonF>> File::normalizedPathss(const QPointF& offset) {
    mvector<mvector<QPolygonF>> pathss;
    pathss.reserve(toolPathss_.size());
    for (const Paths& paths : toolPathss_) {
        pathss.emplace_back(normalizedPaths(offset, paths));
        //        pathss.push_back(toQPolygons(paths));
    }

    //    for (mvector<QPolygonF>& paths : pathss)
    //        for (QPolygonF& path : paths)
    //            path.translate(offset);

    //    if (side_ == Bottom) {
    //        const double k = Pin::minX() + Pin::maxX();
    //        for (auto& paths : pathss) {
    //            for (auto& path : paths) {
    //                if (toolType() != Tool::Laser)
    //                    std::reverse(path.begin(), path.end());
    //                for (QPointF& point : path) {
    //                    point.rx() = -point.x() + k;
    //                }
    //            }
    //        }
    //    }

    //    for (auto& paths : pathss) {
    //        for (auto& path : paths) {
    //            for (QPointF& point : path) {
    //                point -= App::zero()->pos();
    //            }
    //        }
    //    }

    return pathss;
}

mvector<QPolygonF> File::normalizedPaths(const QPointF& offset, const Paths& paths_) {
    mvector<QPolygonF> paths(paths_.empty() ? toolPathss_.front() : paths_);

    for (QPolygonF& path : paths)
        path.translate(offset);

    if (side_ == Bottom) {
        const double k = GiPin::minX() + GiPin::maxX();
        for (auto& path : paths) {
            if (toolType() != Tool::Laser)
                std::reverse(path.begin(), path.end());
            for (QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }
    for (auto& path : paths) {
        for (QPointF& point : path) {
            point -= App::zero()->pos();
        }
    }

    return paths;
}

void File::initSave() {
    lines_.clear();

    for (bool& fl : formatFlags)
        fl = false;

    const QString format(gcp_.getTool().type() == Tool::Laser ? Settings::formatLaser() : Settings::formatMilling());
    for (size_t i = 0; i < cmdList.size(); ++i) {
        const int index = format.indexOf(cmdList[i], 0, Qt::CaseInsensitive);
        if (index != -1) {
            formatFlags[i + AlwaysG] = format[index + 1] == '+';
            if ((index + 2) < format.size())
                formatFlags[i + SpaceG] = format[index + 2] == ' ';
        }
    }

    for (QString& str : lastValues)
        str.clear();

    setFeedRate(gcp_.getTool().feedRate());
    setPlungeRate(gcp_.getTool().plungeRate());
    setSpindleSpeed(gcp_.getTool().spindleSpeed());
    setToolType(gcp_.getTool().type());
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project()->worckRect();
    for (size_t x = 0; x < App::project()->stepsX(); ++x) {
        for (size_t y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);

            switch (gcp_.gcType) {
            case Pocket:
                if (toolType() == Tool::Laser)
                    saveLaserPocket(offset);
                else
                    saveMillingPocket(offset);
                break;
            case Voronoi:
                if (toolType() == Tool::Laser) {
                    if (toolPathss_.size() > 1)
                        saveLaserPocket(offset);
                    else
                        saveLaserProfile(offset);
                } else {
                    if (toolPathss_.size() > 1)
                        saveMillingPocket(offset);
                    else
                        saveMillingProfile(offset);
                }
                break;
            case Profile:
            case Thermal:
            case Raster:
            case Hatching:
                if (toolType() == Tool::Laser)
                    saveLaserProfile(offset);
                else
                    saveMillingProfile(offset);
                break;
            case Drill:
                saveDrill(offset);
                break;
            case LaserHLDI:
                saveLaserHLDI(offset);
                break;
            default:
                break;
            }
            if (gcp_.params.contains(GCodeParams::NotTile))
                return;
        }
    }
}

Tool File::getTool() const { return gcp_.getTool(); }

void File::addInfo() {
    const static auto side_ {GCObj::tr("Top|Bottom").split('|')};
    if (Settings::info()) {
        lines_.emplace_back(GCObj::tr(";\t           Name: %1").arg(shortName()));
        lines_.emplace_back(GCObj::tr(";\t           Tool: %1").arg(gcp_.getTool().name()));
        lines_.emplace_back(GCObj::tr(";\t  Tool Stepover: %1").arg(gcp_.getTool().stepover()));
        lines_.emplace_back(GCObj::tr(";\t Feed Rate mm/s: %1").arg(gcp_.getTool().feedRate_mm_s()));
        lines_.emplace_back(GCObj::tr(";\tTool Pass Depth: %1").arg(gcp_.getTool().passDepth()));
        lines_.emplace_back(GCObj::tr(";\t          Depth: %1").arg(gcp_.getDepth()));
        lines_.emplace_back(GCObj::tr(";\t           Side: %1").arg(side_[side()]));
    }
}

GCodeType File::gtype() const { return gcp_.gcType; }

void File::startPath(const QPointF& point) {
    if (toolType() == Tool::Laser) {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), speed(0)})); // start xy
        //        gCodeText_.push_back(formated({ g1(), speed(spindleSpeed) }));
    } else {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), speed(spindleSpeed())})); // start xy
        lines_.emplace_back(formated({g0(), z(App::project()->plunge())}));                       // start z
        //        lastValues[AlwaysF].clear();
    }
}

void File::endPath() {
    if (toolType() == Tool::Laser) {
        //
    } else {
        lines_.emplace_back(formated({g0(), z(App::project()->clearence())}));
    }
}

void File::statFile() {
    if (toolType() == Tool::Laser) {
        QString str(Settings::laserStart()); //"G21 G17 G90"); //G17 XY plane
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(0)})); // Z0 for visible in Candle
    } else {
        QString str(Settings::start()); //"G21 G17 G90"); //G17 XY plane
        str.replace(QRegularExpression("S\\?"), formated({speed(spindleSpeed())}));
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(App::project()->safeZ())})); // HomeZ
    }
}

void File::endFile() {
    if (toolType() == Tool::Laser) {
        lines_.emplace_back(Settings::spindleLaserOff());
        QPointF home(App::home()->pos() - App::zero()->pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
        lines_.emplace_back(Settings::laserEnd());
    } else {
        lines_.emplace_back(formated({g0(), z(App::project()->safeZ())})); // HomeZ
        QPointF home(App::home()->pos() - App::zero()->pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
        lines_.emplace_back(Settings::end());
    }
    for (size_t i = 0; i < lines_.size(); ++i) { // remove epty lines
        if (lines_[i].isEmpty())
            lines_.erase(lines_.begin() + i--);
    }
}

mvector<QString> File::gCodeText() const { return lines_; }

void File::createGiDrill() {
    GraphicsItem* item;
    for (const IntPoint& point : toolPathss_.front().front()) {
        item = new GiDrill({point}, gcp_.getTool().diameter(), this, gcp_.getTool().id());
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    item = new GiGcPath(toolPathss_.front().front());
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiPocket() {
    GraphicsItem* item;
    if (pocketPaths_.size()) {
        //        {
        //            ClipperOffset offset(uScale);
        //            offset.AddPaths(pocketPaths_, jtRound, etClosedPolygon);
        //            offset.Execute(pocketPaths_, uScale * gcp_.getToolDiameter() * 0.5);
        //        }
        item = new GiDataSolid(pocketPaths_, nullptr);
        item->setPen(Qt::NoPen);
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    }
    g0path_.reserve(toolPathss_.size());
    size_t i = 0;
    for (const Paths& paths : toolPathss_) {
        int k = static_cast<int>((toolPathss_.size() > 1) ? (300.0 / (toolPathss_.size() - 1)) * i : 0);
        debugColor.emplace_back(QSharedPointer<QColor>(new QColor(QColor::fromHsv(k, 255, 255, 255))));

        for (const Path& path : paths) {
            item = new GiGcPath(path, this);
#ifdef QT_DEBUG
            item->setPenColorPtr(debugColor.back().data());
#else
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
#endif
            itemGroup()->push_back(item);
        }

        {
            Paths g1path;
            for (size_t j = 0; j < paths.size() - 1; ++j)
                g1path.push_back({paths[j].back(), paths[j + 1].front()});
            item = new GiGcPath(g1path);
#ifdef QT_DEBUG
            debugColor.push_back(QSharedPointer<QColor>(new QColor(0, 0, 255)));
            item->setPenColorPtr(debugColor.back().data());
#else
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
#endif
            itemGroup()->push_back(item);
        }

        if (i < toolPathss_.size() - 1) {
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
        }
    }
    item = new GiGcPath(g0path_);
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiProfile() {
    GraphicsItem* item;
    for (const Paths& paths : toolPathss_) {
        item = new GiGcPath(paths, this);
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    size_t i = 0;
    for (const Paths& paths : toolPathss_) {
        item = new GiGcPath(toolPathss_[i], this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for (size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if (i < toolPathss_.size() - 1) {
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
        }
    }

    item = new GiGcPath(g0path_);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiRaster() {
    //        int k = static_cast<int>((toolPathss_.size() > 1) ? (300.0 / (toolPathss_.size() - 1)) * i : 0);
    //        QColor* c = new QColor;
    //        *c = QColor::fromHsv(k, 255, 255, 255);
    GraphicsItem* item;
    g0path_.reserve(toolPathss_.size());

    if (pocketPaths_.size()) {
        item = new GiDataSolid(pocketPaths_, nullptr);
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    } else {
        for (const Paths& paths : toolPathss_) {
            item = new GiGcPath(paths, this);
            item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
    }
    size_t i = 0;
    for (const Paths& paths : toolPathss_) {
        item = new GiGcPath(paths, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for (size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if (i < toolPathss_.size() - 1) {
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
        }
    }
    item = new GiGcPath(g0path_);
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiLaser() {
    Paths paths;
    paths.reserve(toolPathss_.front().size() / 2 + 1);
    g0path_.reserve(paths.size());
    for (size_t i = 0; i < toolPathss_.front().size(); ++i) {
        if (i % 2)
            paths.push_back(toolPathss_.front()[i]);
        else
            g0path_.push_back(toolPathss_.front()[i]);
    }
    if (toolPathss_.size() > 1) {
        paths.insert(paths.end(), toolPathss_[1].begin(), toolPathss_[1].end());
        g0path_.push_back({toolPathss_[0].back().back(), toolPathss_[1].front().front()});
        for (size_t i = 0; i < toolPathss_[1].size() - 1; ++i)
            g0path_.push_back({toolPathss_[1][i].back(), toolPathss_[1][i + 1].front()});
    }

    auto item = new GiGcPath(paths, this);
    item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
    itemGroup()->push_back(item);

    if (Settings::simplifyHldi()) {
        auto item = new GiGcPath(g0path_, this);
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        auto color = new QColor(App::settings().guiColor(GuiColors::G0));
        color->setAlpha(127);
        item->setPenColorPtr(color);
        itemGroup()->push_back(item);
        //        ClipperOffset offset;
        //        offset.AddPaths(g0path_, jtRound, etOpenRound);
        //        offset.Execute(g0path_,uScale*gcp_.getToolDiameter());
        //        item = new GcPathItem(g0path_, this);
        //        item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        //        itemGroup()->push_back(item);
    } else {
        item = new GiGcPath(paths, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);

        item = new GiGcPath(g0path_, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        itemGroup()->push_back(item);
    }
}

void File::write(QDataStream& stream) const {
    stream << gcp_;
    stream << pocketPaths_;
    stream << toolPathss_;
}

void File::read(QDataStream& stream) {
    auto& gcp = *const_cast<GCodeParams*>(&gcp_);
    switch (App::project()->ver()) {
    case ProVer_6:
    case ProVer_5:
    case ProVer_4:
        stream >> gcp;
        stream >> pocketPaths_;
        stream >> toolPathss_;
        break;
    case ProVer_3: {
        stream >> pocketPaths_;
        stream >> gcp.gcType;
        stream >> toolPathss_;
        gcp.tools.resize(1);
        stream >> gcp.tools.front();
        double depth;
        stream >> depth;
        gcp.params[GCodeParams::Depth] = depth;
    }
        [[fallthrough]];
    case ProVer_2:
        [[fallthrough]];
    case ProVer_1:;
    }
    //    stream >> *static_cast<AbstractFile*>(this);
    // _read(stream);
}

void File::createGi() {
    switch (gcp_.gcType) {
    case md5::hash32("Profile"):
    case GCode::Thermal:
        createGiProfile();
        break;
    case GCode::Raster:
    case GCode::Hatching:
        createGiRaster();
        break;
    case GCode::Voronoi:
        if (toolPathss_.size() > 1) {
            GraphicsItem* item;
            item = new GiGcPath(toolPathss_.back().back(), this);
            item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
            createGiPocket();
        } else
            createGiProfile();
        break;
    case GCode::Pocket:
        createGiPocket();
        break;
    case GCode::Drill:
        createGiDrill();
        break;
    case GCode::LaserHLDI:
        createGiLaser();
        break;
    default:
        break;
    }

    for (auto&& paths : toolPathss_)
        for (auto&& path : paths)
            calcArcs(path);

    switch (gcp_.gcType) {
    case md5::hash32("Profile"):
        icon_ = QIcon::fromTheme("profile-path");
        break;
    case GCode::Pocket:
        icon_ = QIcon::fromTheme("pocket-path");
        break;
    case GCode::Voronoi:
        icon_ = QIcon::fromTheme("voronoi-path");
        break;
    case GCode::Thermal:
        icon_ = QIcon::fromTheme("thermal-path");
        break;
    case GCode::Drill:
        icon_ = QIcon::fromTheme("drill-path");
        break;
    case GCode::Raster:
    case GCode::LaserHLDI:
        icon_ = QIcon::fromTheme("raster-path");
        break;
    case GCode::Hatching:
        icon_ = QIcon::fromTheme("crosshatch-path");
    default:
        break;
    }

    itemGroup()->setVisible(true);
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node(this);
}

} // namespace GCode
