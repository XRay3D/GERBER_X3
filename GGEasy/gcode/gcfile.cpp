// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcfile.h"

#include "datapathitem.h"
#include "datasoliditem.h"
#include "point.h"
#include "project.h"
#include "settings.h"

#include "gcdrillitem.h"
#include "gcnode.h"
#include "gcpathitem.h"
#include "graphicsview.h"

#include <QDir>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QTextStream>

#include "leakdetector.h"

namespace GCode {

File::File()
    : GCUtils(m_gcp)
    , FileInterface() {
}

File::File(const Pathss& toolPathss, const GCodeParams& gcp, const Paths& pocketPaths)
    : GCUtils(m_gcp)
    , FileInterface()
    , m_pocketPaths(pocketPaths)
    , m_toolPathss(toolPathss)
    , m_gcp(gcp) {
    if(gcp.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

bool File::save(const QString& name) {
    if(name.isEmpty())
        return false;

    initSave();
    addInfo();
    statFile();
    genGcodeAndTile();
    endFile();

    setLastDir(name);
    m_name = name;
    QFile file(m_name);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for(QString& s : m_lines) {
            if(!s.isEmpty())
                str.push_back(s);
            if(!str.endsWith('\n'))
                str.push_back("\n");
        }
        out << str;
    } else
        return false;
    file.close();
    return true;
}

void File::saveDrill(const QPointF& offset) {
    QPolygonF path(normalizedPaths(offset, m_toolPathss.front()).front());

    const mvector<double> depths(getDepths());

    for(QPointF& point : path) {
        startPath(point);
        size_t i = 0;
        while(true) {
            m_lines.push_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            if(++i == depths.size())
                break;
            m_lines.push_back(formated({g0(), z(0.0)}));
        }
        endPath();
    }
}

void File::saveLaserPocket(const QPointF& offset) {
    saveLaserProfile(offset);
    //    m_lines.push_back(Settings::laserDynamOn());

    //    mvector<mvector<QPolygonF>> toolPathss(normalizedPathss(offset));

    //    for (mvector<QPolygonF>& paths : toolPathss) {
    //        startPath(paths.front().front());
    //        bool skip = true;
    //        for (auto& path : paths) {
    //            for (QPointF& point : path) {
    //                if (skip)
    //                    skip = false;
    //                else
    //                    m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
    //            }
    //        }
    //        endPath();
    //    }
}

void File::saveMillingPocket(const QPointF& offset) {
    m_lines.push_back(Settings::spindleOn());

    mvector<mvector<QPolygonF>> toolPathss(normalizedPathss(offset));

    const mvector<double> depths(getDepths());

    for(mvector<QPolygonF>& paths : toolPathss) {
        startPath(paths.front().front());
        for(size_t i = 0; i < depths.size(); ++i) {
            m_lines.push_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            bool skip = true;
            for(auto& path : paths) {
                for(QPointF& point : path) {
                    if(skip)
                        skip = false;
                    else
                        m_lines.push_back(formated({g1(), x(point.x()), y(point.y()), feed(feedRate())}));
                }
            }
            for(size_t j = paths.size() - 2; j != std::numeric_limits<size_t>::max() && i < depths.size() - 1; --j) {
                QPointF& point = paths[j].back();
                m_lines.push_back(formated({g0(), x(point.x()), y(point.y())}));
            }
            if(paths.size() > 1 && i < (depths.size() - 1))
                m_lines.push_back(formated({g0(), x(paths.front().front().x()), y(paths.front().front().y())}));
        }
        endPath();
    }
}

void File::saveMillingProfile(const QPointF& offset) {
    if(m_gcp.gcType == Raster) {
        saveMillingRaster(offset);
        return;
    }

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for(auto& paths : pathss) {
        for(size_t i = 0; i < depths.size(); ++i) {
            for(size_t j = 0; j < paths.size(); ++j) {
                QPolygonF& path = paths[j];
                if(path.front() == path.last()) { // make complete depth and remove from worck
                    startPath(path.front());
                    for(size_t k = 0; k < depths.size(); ++k) {
                        m_lines.push_back(formated({g1(), z(depths[k]), feed(plungeRate())}));
                        auto sp(savePath(path, spindleSpeed()));
                        m_lines.append(sp);
                        //                    bool skip = true;
                        //                    for (QPointF& point : path) {
                        //                        if (skip)
                        //                            skip = false;
                        //                        else
                        //                            m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                        //                    }
                    }
                    endPath();
                    paths.erase(paths.begin() + j--);
                } else {
                    startPath(path.front());
                    m_lines.push_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
                    auto sp(savePath(path, spindleSpeed()));
                    m_lines.append(sp);
                    //                    bool skip = true;
                    //                    for (QPointF& point : path) {
                    //                        if (skip)
                    //                            skip = false;
                    //                        else
                    //                            m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                    //                    }
                    endPath();
                }
            }
        }
    }
}

void File::saveLaserProfile(const QPointF& offset) {
    m_lines.push_back(Settings::laserDynamOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    for(auto& paths : pathss) {
        for(auto& path : paths) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            m_lines.append(sp);
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed()) }));
            //            }
            endPath();
        }
    }
}

void File::saveMillingRaster(const QPointF& offset) {
    m_lines.push_back(Settings::spindleOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for(auto& paths : pathss) {
        for(size_t i = 0; i < depths.size(); ++i) {
            for(auto& path : paths) {
                startPath(path.front());
                m_lines.push_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
                auto sp(savePath(path, spindleSpeed()));
                m_lines.append(sp);
                //                bool skip = true;
                //                for (QPointF& point : path) {
                //                    if (skip)
                //                        skip = false;
                //                    else
                //                        m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
                //                }
                endPath();
            }
        }
    }
}

void File::saveLaserHLDI(const QPointF& offset) {
    m_lines.push_back(Settings::laserConstOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    int i = 0;

    m_lines.push_back(formated({g0(), x(pathss.front().front().front().x()), y(pathss.front().front().front().y()), z(0.0)}));

    for(QPolygonF& path : pathss.front()) {
        if(i++ % 2) {
            auto sp(savePath(path, spindleSpeed()));
            m_lines.append(sp);
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed()) }));
            //            }
        } else {
            auto sp(savePath(path, 0));
            m_lines.append(sp);
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(static_cast<int>(0)) }));
            //            }
        }
    }
    if(pathss.size() > 1) {
        m_lines.push_back(Settings::laserDynamOn());
        for(QPolygonF& path : pathss.back()) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            m_lines.append(sp);
            //            bool skip = true;
            //            for (QPointF& point : path) {
            //                if (skip)
            //                    skip = false;
            //                else
            //                    m_lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()) }));
            //            }
            endPath();
        }
    }
}

const GCodeParams& File::gcp() const { return m_gcp; }

mvector<mvector<QPolygonF>> File::normalizedPathss(const QPointF& offset) {
    mvector<mvector<QPolygonF>> pathss;
    pathss.reserve(m_toolPathss.size());
    for(const Paths& paths : m_toolPathss) {
        pathss.push_back(normalizedPaths(offset, paths));
        //        pathss.push_back(toQPolygons(paths));
    }

    //    for (mvector<QPolygonF>& paths : pathss)
    //        for (QPolygonF& path : paths)
    //            path.translate(offset);

    //    if (m_side == Bottom) {
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
    //                point -= Marker::get(Marker::Zero)->pos();
    //            }
    //        }
    //    }

    return pathss;
}

mvector<QPolygonF> File::normalizedPaths(const QPointF& offset, const Paths& paths_) {
    mvector<QPolygonF> paths(paths_.empty() ? m_toolPathss.front() : paths_);

    for(QPolygonF& path : paths)
        path.translate(offset);

    if(m_side == Bottom) {
        const double k = Pin::minX() + Pin::maxX();
        for(auto& path : paths) {
            if(toolType() != Tool::Laser)
                std::reverse(path.begin(), path.end());
            for(QPointF& point : path) {
                point.rx() = -point.x() + k;
            }
        }
    }
    for(auto& path : paths) {
        for(QPointF& point : path) {
            point -= Marker::get(Marker::Zero)->pos();
        }
    }

    return paths;
}

void File::initSave() {
    m_lines.clear();

    for(bool& fl : formatFlags)
        fl = false;

    const QString format(m_gcp.getTool().type() == Tool::Laser ? Settings::formatLaser()
                                                               : Settings::formatMilling());
    for(size_t i = 0; i < cmdList.size(); ++i) {
        const int index = format.indexOf(cmdList[i], 0, Qt::CaseInsensitive);
        if(index != -1) {
            formatFlags[i + AlwaysG] = format[index + 1] == '+';
            if((index + 2) < format.size())
                formatFlags[i + SpaceG] = format[index + 2] == ' ';
        }
    }

    for(QString& str : lastValues)
        str.clear();

    setFeedRate(m_gcp.getTool().feedRate());
    setPlungeRate(m_gcp.getTool().plungeRate());
    setSpindleSpeed(m_gcp.getTool().spindleSpeed());
    setToolType(m_gcp.getTool().type());
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project()->worckRect();
    for(size_t x = 0; x < App::project()->stepsX(); ++x) {
        for(size_t y = 0; y < App::project()->stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project()->spaceX()) * x, (rect.height() + App::project()->spaceY()) * y);

            switch(m_gcp.gcType) {
            case Pocket:
                if(toolType() == Tool::Laser)
                    saveLaserPocket(offset);
                else
                    saveMillingPocket(offset);
                break;
            case Voronoi:
                if(toolType() == Tool::Laser) {
                    if(m_toolPathss.size() > 1)
                        saveLaserPocket(offset);
                    else
                        saveLaserProfile(offset);
                } else {
                    if(m_toolPathss.size() > 1)
                        saveMillingPocket(offset);
                    else
                        saveMillingProfile(offset);
                }
                break;
            case Profile:
            case Thermal:
            case Raster:
                if(toolType() == Tool::Laser)
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
            if(m_gcp.params.contains(GCodeParams::NotTile))
                return;
        }
    }
}

Tool File::getTool() const { return m_gcp.getTool(); }

void File::addInfo() {
    if(Settings::info()) {
        m_lines.push_back(QString(";\tName:\t%1").arg(shortName()));
        m_lines.push_back(QString(";\tTool:\t%1").arg(m_gcp.getTool().name()));
        m_lines.push_back(QString(";\tDepth:\t%1").arg(m_gcp.getDepth()));
        m_lines.push_back(QString(";\tSide:\t%1").arg(QStringList {"Top", "Bottom"}[side()]));
    }
}

GCodeType File::gtype() const { return m_gcp.gcType; }

void File::startPath(const QPointF& point) {
    if(toolType() == Tool::Laser) {
        m_lines.push_back(formated({g0(), x(point.x()), y(point.y()), speed(0)})); //start xy
        //        m_gCodeText.push_back(formated({ g1(), speed(spindleSpeed) }));
    } else {
        m_lines.push_back(formated({g0(), x(point.x()), y(point.y()), speed(spindleSpeed())})); //start xy
        m_lines.push_back(formated({g0(), z(App::project()->plunge())})); //start z
        //        lastValues[AlwaysF].clear();
    }
}

void File::endPath() {
    if(toolType() == Tool::Laser) {
        //
    } else {
        m_lines.push_back(formated({g0(), z(App::project()->clearence())}));
    }
}

void File::statFile() {
    if(toolType() == Tool::Laser) {
        QString str(Settings::laserStart()); //"G21 G17 G90"); //G17 XY plane
        m_lines.push_back(str);
        m_lines.push_back(formated({g0(), z(0)})); // Z0 for visible in Candle
    } else {
        QString str(Settings::start()); //"G21 G17 G90"); //G17 XY plane
        str.replace(QRegularExpression("S\\?"), formated({speed(spindleSpeed())}));
        m_lines.push_back(str);
        m_lines.push_back(formated({g0(), z(App::project()->safeZ())})); //HomeZ
    }
}

void File::endFile() {
    if(toolType() == Tool::Laser) {
        m_lines.push_back(Settings::spindleLaserOff());
        QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
        m_lines.push_back(formated({g0(), x(home.x()), y(home.y())})); //HomeXY
        m_lines.push_back(Settings::laserEnd());
    } else {
        m_lines.push_back(formated({g0(), z(App::project()->safeZ())})); //HomeZ
        QPointF home(Marker::get(Marker::Home)->pos() - Marker::get(Marker::Zero)->pos());
        m_lines.push_back(formated({g0(), x(home.x()), y(home.y())})); //HomeXY
        m_lines.push_back(Settings::end());
    }
    for(size_t i = 0; i < m_lines.size(); ++i) { // remove epty lines
        if(m_lines[i].isEmpty())
            m_lines.erase(m_lines.begin() + i--);
    }
}

mvector<QString> File::gCodeText() const { return m_lines; }

void File::createGiDrill() {
    GraphicsItem* item;
    for(const IntPoint& point : m_toolPathss.front().front()) {
        item = new DrillItem(m_gcp.getTool().diameter(), this);
        item->setPos(point);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    item = new GcPathItem(m_toolPathss.front().front());
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiPocket() {
    GraphicsItem* item;
    if(m_pocketPaths.size()) {
        //        {
        //            ClipperOffset offset(uScale);
        //            offset.AddPaths(m_pocketPaths, jtRound, etClosedPolygon);
        //            offset.Execute(m_pocketPaths, uScale * m_gcp.getToolDiameter() * 0.5);
        //        }
        item = new GiDataSolid(m_pocketPaths, nullptr);
        item->setPen(Qt::NoPen);
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    }
    m_g0path.reserve(m_toolPathss.size());
    size_t i = 0;
    for(const Paths& paths : m_toolPathss) {
        int k = static_cast<int>((m_toolPathss.size() > 1) ? (300.0 / (m_toolPathss.size() - 1)) * i : 0);
        debugColor.push_back(QSharedPointer<QColor>(new QColor(QColor::fromHsv(k, 255, 255, 255))));

        for(const Path& path : paths) {
            item = new GcPathItem(path, this);
#ifdef QT_DEBUG
            item->setPenColorPtr(debugColor.back().data());
#else
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
#endif
            itemGroup()->push_back(item);
        }

        {
            Paths g1path;
            for(size_t j = 0; j < paths.size() - 1; ++j)
                g1path.push_back({paths[j].back(), paths[j + 1].front()});
            item = new GcPathItem(g1path);
#ifdef QT_DEBUG
            debugColor.push_back(QSharedPointer<QColor>(new QColor(0, 0, 255)));
            item->setPenColorPtr(debugColor.back().data());
#else
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
#endif
            itemGroup()->push_back(item);
        }

        if(i < m_toolPathss.size() - 1) {
            m_g0path.push_back({m_toolPathss[i].back().back(), m_toolPathss[++i].front().front()});
        }
    }
    item = new GcPathItem(m_g0path);
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiProfile() {
    GraphicsItem* item;
    for(const Paths& paths : m_toolPathss) {
        item = new GcPathItem(paths, this);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    size_t i = 0;
    for(const Paths& paths : m_toolPathss) {
        item = new GcPathItem(m_toolPathss[i], this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            m_g0path.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < m_toolPathss.size() - 1) {
            m_g0path.push_back({m_toolPathss[i].back().back(), m_toolPathss[++i].front().front()});
        }
    }

    item = new GcPathItem(m_g0path);
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiRaster() {
    //        int k = static_cast<int>((m_toolPathss.size() > 1) ? (300.0 / (m_toolPathss.size() - 1)) * i : 0);
    //        QColor* c = new QColor;
    //        *c = QColor::fromHsv(k, 255, 255, 255);
    GraphicsItem* item;
    m_g0path.reserve(m_toolPathss.size());

    if(m_pocketPaths.size()) {
        item = new GiDataSolid(m_pocketPaths, nullptr);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    } else {
        for(const Paths& paths : m_toolPathss) {
            item = new GcPathItem(paths, this);
            item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
    }
    size_t i = 0;
    for(const Paths& paths : m_toolPathss) {
        item = new GcPathItem(paths, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            m_g0path.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < m_toolPathss.size() - 1) {
            m_g0path.push_back({m_toolPathss[i].back().back(), m_toolPathss[++i].front().front()});
        }
    }
    item = new GcPathItem(m_g0path);
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiLaser() {
    Paths paths;
    paths.reserve(m_toolPathss.front().size() / 2 + 1);
    m_g0path.reserve(paths.size());
    for(size_t i = 0; i < m_toolPathss.front().size(); ++i) {
        if(i % 2)
            paths.push_back(m_toolPathss.front()[i]);
        else
            m_g0path.push_back(m_toolPathss.front()[i]);
    }
    if(m_toolPathss.size() > 1) {
        paths.insert(paths.end(), m_toolPathss[1].begin(), m_toolPathss[1].end());
        m_g0path.push_back({m_toolPathss[0].back().back(), m_toolPathss[1].front().front()});
        for(size_t i = 0; i < m_toolPathss[1].size() - 1; ++i)
            m_g0path.push_back({m_toolPathss[1][i].back(), m_toolPathss[1][i + 1].front()});
    }

    auto item = new GcPathItem(paths, this);
    item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
    itemGroup()->push_back(item);

    if(Settings::simplifyHldi()) {
        auto item = new GcPathItem(m_g0path, this);
        item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        auto color = new QColor(App::settings().guiColor(GuiColors::G0));
        color->setAlpha(127);
        item->setPenColorPtr(color);
        itemGroup()->push_back(item);
        //        ClipperOffset offset;
        //        offset.AddPaths(m_g0path, jtRound, etOpenRound);
        //        offset.Execute(m_g0path,uScale*m_gcp.getToolDiameter());
        //        item = new GcPathItem(m_g0path, this);
        //        item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        //        itemGroup()->push_back(item);
    } else {
        item = new GcPathItem(paths, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);

        item = new GcPathItem(m_g0path, this);
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
        itemGroup()->push_back(item);
    }
}

void File::write(QDataStream& stream) const {
    stream << m_gcp;
    stream << m_pocketPaths;
    stream << m_toolPathss;
}

void File::read(QDataStream& stream) {
    auto& gcp = *const_cast<GCodeParams*>(&m_gcp);
    switch(App::project()->ver()) {
    case ProVer_5:
    case ProVer_4:
        stream >> gcp;
        stream >> m_pocketPaths;
        stream >> m_toolPathss;
        break;
    case ProVer_3: {
        stream >> m_pocketPaths;
        stream >> gcp.gcType;
        stream >> m_toolPathss;
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
    //    stream >> *static_cast<FileInterface*>(this);
    // _read(stream);
}

void File::createGi() {
    switch(m_gcp.gcType) {
    case GCode::Profile:
    case GCode::Thermal:
        createGiProfile();
        break;
    case GCode::Raster:
    case GCode::Hatching:
        createGiRaster();
        break;
    case GCode::Voronoi:
        if(m_toolPathss.size() > 1) {
            GraphicsItem* item;
            item = new GcPathItem(m_toolPathss.back().back(), this);
            item->setPen(QPen(Qt::black, m_gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
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

    switch(m_gcp.gcType) {
    case GCode::Profile:
        m_icon = QIcon::fromTheme("profile-path");
        break;
    case GCode::Pocket:
        m_icon = QIcon::fromTheme("pocket-path");
        break;
    case GCode::Voronoi:
        m_icon = QIcon::fromTheme("voronoi-path");
        break;
    case GCode::Thermal:
        m_icon = QIcon::fromTheme("thermal-path");
        break;
    case GCode::Drill:
        m_icon = QIcon::fromTheme("drill-path");
        break;
    case GCode::Raster:
    case GCode::LaserHLDI:
        m_icon = QIcon::fromTheme("raster-path");
        break;
    case GCode::Hatching:
        m_icon = QIcon::fromTheme("crosshatch-path");
    default:
        break;
    }

    itemGroup()->setVisible(true);
}

FileTree::Node* File::node() {
    return m_node ? m_node : m_node = new Node(this, &m_id);
}

}
