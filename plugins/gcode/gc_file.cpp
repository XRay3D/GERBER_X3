// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_file.h"
#include "gc_node.h"

#include "app.h"
#include "gi.h"
#include "gi_datasolid.h"
#include "gi_drill.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "math.h"
#include "plugintypes.h"
#include "project.h"

#include <QFileInfo>
#include <QRegularExpression>

namespace GCode {

void calcArcs(Path path);

File::File(Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : gcp_(std::move(gcp))
    , pocketPaths_(std::move(pocketPaths))
    , toolPathss_(std::move(toolPathss)) {
    for(auto&& paths: toolPathss_)
        for(auto&& path: paths)
            calcArcs(path);

    feedRate_ = gcp_.getTool().feedRate();
    plungeRate_ = gcp_.getTool().plungeRate();
    spindleSpeed_ = gcp_.getTool().spindleSpeed();
    toolType_ = gcp_.getTool().type();
}

File::File() { }

// GCodeType File::gtype() const { return gcp_.gcType; }

mvector<QString> File::gCodeText() const { return lines_; }

const Tool& File::getTool() const { return gcp_.getTool(); }

const Params& File::gcp() const { return gcp_; }

double File::feedRate() { return feedRate_; }

double File::plungeRate() { return plungeRate_; }

int File::spindleSpeed() { return spindleSpeed_; }

int File::toolType() { return toolType_; }

void File::setFeedRate(double val) { feedRate_ = val; }

void File::setPlungeRate(double val) { plungeRate_ = val; }

void File::setSpindleSpeed(int val) { spindleSpeed_ = val; }

void File::setToolType(int val) { toolType_ = val; }

QString File::getLastDir() {
    if(App::gcSettings().sameFolder() && !redirected)
        lastDir = QFileInfo(App::project().name()).absolutePath();
    else if(lastDir.isEmpty()) {
        QSettings settings;
        lastDir = settings.value("LastGCodeDir").toString();
        if(lastDir.isEmpty())
            lastDir = QFileInfo(App::project().name()).absolutePath();
        settings.setValue("LastGCodeDir", lastDir);
    }
    return lastDir += '/';
}

void File::setLastDir(QString dirPath) {
    dirPath = QFileInfo(dirPath).absolutePath();
    if(App::gcSettings().sameFolder() && !redirected) {
        redirected = QFileInfo(App::project().name()).absolutePath() != dirPath;
        if(!redirected)
            return;
    }
    if(lastDir != dirPath) {
        lastDir = dirPath;
        QSettings settings;
        settings.setValue("LastGCodeDir", lastDir);
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
    name_ = name;
    QFile file(name_);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString str;
        for(QString& s: lines_) {
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

void File::statFile() {
    if(toolType() == Tool::Laser) {
        QString str(App::gcSettings().laserStart()); //"G21 G17 G90"); //G17 XY plane
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(0)})); // Z0 for visible in Candle
    } else {
        QString str(App::gcSettings().start());      //"G21 G17 G90"); //G17 XY plane
        str.replace(QRegularExpression("S\\?"), formated({speed(spindleSpeed())}));
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
    }
}

void File::endFile() {
    if(toolType() == Tool::Laser) {
        lines_.emplace_back(App::gcSettings().spindleLaserOff());
        QPointF home(App::home().pos() - App::zero().pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
        lines_.emplace_back(App::gcSettings().laserEnd());
    } else {
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
        QPointF home(App::home().pos() - App::zero().pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())}));  // HomeXY
        lines_.emplace_back(App::gcSettings().end());
    }
    for(size_t i = 0; i < lines_.size(); ++i) // remove epty lines
        if(lines_[i].isEmpty())
            lines_.erase(lines_.begin() + i--);
}

void File::addInfo() {
    const static auto side_{QObject::tr("Top|Bottom").split('|')};
    if(App::gcSettings().info()) {
        lines_.emplace_back(QObject::tr(";\t           Name: %1").arg(shortName()));
        lines_.emplace_back(QObject::tr(";\t           Tool: %1").arg(gcp_.getTool().name()));
        lines_.emplace_back(QObject::tr(";\t  Tool Stepover: %1").arg(gcp_.getTool().stepover()));
        lines_.emplace_back(QObject::tr(";\t Feed Rate mm/s: %1").arg(gcp_.getTool().feedRate_mmPerSec()));
        lines_.emplace_back(QObject::tr(";\tTool Pass Depth: %1").arg(gcp_.getTool().passDepth()));
        lines_.emplace_back(QObject::tr(";\t          Depth: %1").arg(gcp_.getDepth()));
        lines_.emplace_back(QObject::tr(";\t           Side: %1").arg(side_[side()]));
    }
}

void File::initSave() {
    lines_.clear();

    for(bool& fl: formatFlags)
        fl = false;

    const QString format(gcp_.getTool().type() == Tool::Laser ? App::gcSettings().formatLaser() : App::gcSettings().formatMilling());
    for(size_t i = 0; i < cmdList.size(); ++i) {
        const int index = format.indexOf(cmdList[i], 0, Qt::CaseInsensitive);
        if(index != -1) {
            formatFlags[i + AlwaysG] = format[index + 1] == '+';
            if((index + 2) < format.size())
                formatFlags[i + SpaceG] = format[index + 2] == ' ';
        }
    }

    for(QString& str: lastValues)
        str.clear();

    setFeedRate(gcp_.getTool().feedRate());
    setPlungeRate(gcp_.getTool().plungeRate());
    setSpindleSpeed(gcp_.getTool().spindleSpeed());
    setToolType(gcp_.getTool().type());
}

void File::startPath(const QPointF& point) {
    if(toolType() == Tool::Laser) {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), speed(0)})); // start xy
        //        gCodeText_.push_back(formated({ g1(), speed(spindleSpeed) }));
    } else {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), speed(spindleSpeed())})); // start xy
        lines_.emplace_back(formated({g0(), z(App::project().plunge())}));                        // start z
        //        lastValues[AlwaysF].clear();
    }
}

void File::endPath() {
    if(toolType() == Tool::Laser) {
        //
    } else {
        lines_.emplace_back(formated({g0(), z(App::project().clearence())}));
    }
}

mvector<mvector<QPolygonF>> File::normalizedPathss(const QPointF& offset) {
    mvector<mvector<QPolygonF>> pathss;
    pathss.reserve(toolPathss_.size());
    for(const Paths& paths: toolPathss_) {
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
    //                point -= App::zero().pos();
    //            }
    //        }
    //    }

    return pathss;
}

mvector<QPolygonF> File::normalizedPaths(const QPointF& offset, const Paths& paths_) {
    mvector<QPolygonF> paths(paths_.empty() ? toolPathss_.front() : paths_);

    for(QPolygonF& path: paths)
        path.translate(offset);

    if(side_ == Bottom) {
        const double k = Gi::Pin::minX() + Gi::Pin::maxX();
        if(toolType() != Tool::Laser)
            std::ranges::for_each(paths, [](auto& path) { std::reverse(path.begin(), path.end()); });
        for(QPointF& point: std::views::join(paths))
            point.rx() = -point.x() + k;
    }

    for(QPointF& point: std::views::join(paths))
        point -= App::zero().pos();

    return paths;
}

mvector<double> File::getDepths() {
    const auto gDepth{gcp_.getDepth()};
    if(gDepth < gcp_.getTool().passDepth() || qFuzzyCompare(gDepth, gcp_.getTool().passDepth()))
        return {-gDepth - gcp_.getTool().getDepth()};

    const int count = static_cast<int>(ceil(gDepth / gcp_.getTool().passDepth()));
    const double depth = gDepth / count;
    mvector<double> depths(count);
    for(int i = 0; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.back() = -gDepth - gcp_.getTool().depth();
    return depths;
}

mvector<QString> File::savePath(const QPolygonF& path, double spindleSpeed) {
    mvector<QString> lines;
    lines.reserve(path.size());
    bool skip = true;
    for(const QPointF& point: path)
        if(skip)
            skip = false;
        else
            lines.emplace_back(formated({g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed)}));
    return lines;
}

QString File::formated(const mvector<QString>& data) {
    QString ret;
    for(const QString& str: data) {
        const int index = cmdList.indexOf(str.front().toUpper());
        if(index != -1) {
            if(formatFlags[AlwaysG + index] || lastValues[index] != str) {
                lastValues[index] = str;
                ret += str + (formatFlags[SpaceG + index] ? " " : "");
            }
        }
    }
    return ret.trimmed();
}

QString File::g0() { return gCode_ = G00, "G0"; }

QString File::g1() { return gCode_ = G01, "G1"; }

QString File::x(double val) { return 'X' + format(val); }

QString File::y(double val) { return 'Y' + format(val); }

QString File::z(double val) { return 'Z' + format(val); }

QString File::feed(double val) { return 'F' + format(val); }

QString File::speed(int val) { return 'S' + QString::number(val); }

QString File::format(double val) {
    QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
    if(str.contains('e'))
        return QString::number(val, 'f', 3);
    return str;
}

void File::write(QDataStream& stream) const {
    stream << gcp_;
    stream << pocketPaths_;
    stream << toolPathss_;
}

void File::read(QDataStream& stream) {
    auto& gcp = *const_cast<Params*>(&gcp_);
    switch(App::project().ver()) {
    case ProVer_7:
    case ProVer_6:
    case ProVer_5:
    case ProVer_4:
        stream >> gcp;
        stream >> pocketPaths_;
        stream >> toolPathss_;
        break;
    case ProVer_3: {
        //        stream >> pocketPaths_;
        //        stream >> gcp.gcType;
        //        stream >> toolPathss_;
        //        gcp.tools.resize(1);
        //        stream >> gcp.tools.front();
        //        double depth;
        //        stream >> depth;
        //        gcp.params[Params::Depth] = depth;
    }
        [[fallthrough]];
    case ProVer_2:
        [[fallthrough]];
    case ProVer_1:;
    }
    //    stream >> *static_cast<AbstractFile*>(this);
    // _read(stream);
}

FileTree::Node* File::node() { return node_ ? node_ : node_ = new Node{this}; }

/////////////////////////////////////////////////////////////
void File::saveDrill(const QPointF& offset) {
    QPolygonF path(normalizedPaths(offset, toolPathss_.front()).front());

    const mvector<double> depths(getDepths());

    for(QPointF& point: path) {
        startPath(point);
        size_t i = 0;
        while(true) {
            lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            if(++i == depths.size())
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
    lines_.emplace_back(App::gcSettings().spindleOn());

    mvector<mvector<QPolygonF>> toolPathss(normalizedPathss(offset));

    const mvector<double> depths(getDepths());

    for(mvector<QPolygonF>& paths: toolPathss) {
        startPath(paths.front().front());
        for(size_t i = 0; i < depths.size(); ++i) {
            lines_.emplace_back(formated({g1(), z(depths[i]), feed(plungeRate())}));
            bool skip = true;
            for(auto& path: paths) {
                for(QPointF& point: path)
                    if(skip)
                        skip = false;
                    else
                        lines_.emplace_back(formated({g1(), x(point.x()), y(point.y()), feed(feedRate())}));
            }
            for(size_t j = paths.size() - 2; j != std::numeric_limits<size_t>::max() && i < depths.size() - 1; --j) {
                QPointF& point = paths[j].back();
                lines_.emplace_back(formated({g0(), x(point.x()), y(point.y())}));
            }
            if(paths.size() > 1 && i < (depths.size() - 1))
                lines_.emplace_back(formated({g0(), x(paths.front().front().x()), y(paths.front().front().y())}));
        }
        endPath();
    }
}

void File::saveMillingProfile(const QPointF& offset) {
    //    if (gcp_.gcType == Raster) {
    //        saveMillingRaster(offset);
    //        return;
    //    }

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for(auto& paths: pathss) {
        for(size_t i = 0; i < depths.size(); ++i) {
            for(size_t j = 0; j < paths.size(); ++j) {
                QPolygonF& path = paths[j];
                if(path.front() == path.last()) { // make complete depth and remove from worck
                    startPath(path.front());
                    for(size_t k = 0; k < depths.size(); ++k) {
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
    lines_.emplace_back(App::gcSettings().laserDynamOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    for(auto& paths: pathss) {
        for(auto& path: paths) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
            endPath();
        }
    }
}

void File::saveMillingRaster(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().spindleOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));
    const mvector<double> depths(getDepths());

    for(auto& paths: pathss) {
        for(size_t i = 0; i < depths.size(); ++i) {
            for(auto& path: paths) {
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
    lines_.emplace_back(App::gcSettings().laserConstOn());

    mvector<mvector<QPolygonF>> pathss(normalizedPathss(offset));

    int i = 0;

    lines_.emplace_back(formated({g0(), x(pathss.front().front().front().x()), y(pathss.front().front().front().y()), z(0.0)}));

    for(QPolygonF& path: pathss.front()) {
        if(i++ % 2) {
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
        } else {
            auto sp(savePath(path, 0));
            lines_.append(sp);
        }
    }
    if(pathss.size() > 1) {
        lines_.emplace_back(App::gcSettings().laserDynamOn());
        for(QPolygonF& path: pathss.back()) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed()));
            lines_.append(sp);
            endPath();
        }
    }
}

void File::createGiDrill() {
    Gi::Item* item;
    for(const Point& point: toolPathss_.front().front()) {
        item = new Gi::Drill{{point}, gcp_.getTool().diameter(), this, gcp_.getTool().id()};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    item = new Gi::GcPath{toolPathss_.front().front()};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiPocket() {
    Gi::Item* item;
    if(pocketPaths_.size()) {
        //        {
        //            ClipperOffset offset(uScale);
        //            offset.AddPaths(pocketPaths_, JoinType::Round, EndType::Polygon);
        //            offset.Execute(pocketPaths_, uScale * gcp_.getToolDiameter() * 0.5);
        //        }
            item = new Gi::DataFill{pocketPaths_, nullptr};
        item->setPen(Qt::NoPen);
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    }
    g0path_.reserve(toolPathss_.size());
    size_t i = 0;
    for(const Paths& paths: toolPathss_) {
        int k = static_cast<int>((toolPathss_.size() > 1) ? (300.0 / (toolPathss_.size() - 1)) * i : 0);
        debugColor.emplace_back(QSharedPointer<QColor>(new QColor{QColor::fromHsv(k, 255, 255, 255)}));

        for(const Path& path: paths) {
            item = new Gi::GcPath{path, this};
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
            item = new Gi::GcPath{g1path};
#ifdef QT_DEBUG
            debugColor.push_back(QSharedPointer<QColor>(new QColor{0, 0, 255}));
            item->setPenColorPtr(debugColor.back().data());
#else
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
#endif
            itemGroup()->push_back(item);
        }

        if(i < toolPathss_.size() - 1)
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
    }
    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiProfile() {
    Gi::Item* item;
    for(const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    size_t i = 0;
    for(const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{toolPathss_[i], this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < toolPathss_.size() - 1)
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    //    item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiRaster() {
    //        int k = static_cast<int>((toolPathss_.size() > 1) ? (300.0 / (toolPathss_.size() - 1)) * i : 0);
    //        QColor* c = new QColor;
    //        *c = QColor::fromHsv(k, 255, 255, 255);
    Gi::Item* item;
    g0path_.reserve(toolPathss_.size());

    if(pocketPaths_.size()) {
            item = new Gi::DataFill{pocketPaths_, nullptr};
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    } else {
        for(const Paths& paths: toolPathss_) {
            item = new Gi::GcPath{paths, this};
            item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
    }
    size_t i = 0;

    //    for (int i {}; auto& path : std::views::join(toolPathss_)) { }

    for(const Paths& paths: toolPathss_) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j = 0; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < toolPathss_.size() - 1)
            g0path_.push_back({toolPathss_[i].back().back(), toolPathss_[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiLaser() {
    Paths paths;
    paths.reserve(toolPathss_.front().size() / 2 + 1);
    g0path_.reserve(paths.size());
    for(size_t i = 0; i < toolPathss_.front().size(); ++i)
        if(i % 2)
            paths.push_back(toolPathss_.front()[i]);
        else
            g0path_.push_back(toolPathss_.front()[i]);
    if(toolPathss_.size() > 1) {
        paths.insert(paths.end(), toolPathss_[1].begin(), toolPathss_[1].end());
        g0path_.push_back({toolPathss_[0].back().back(), toolPathss_[1].front().front()});
        for(size_t i = 0; i < toolPathss_[1].size() - 1; ++i)
            g0path_.push_back({toolPathss_[1][i].back(), toolPathss_[1][i + 1].front()});
    }

    auto item = new Gi::GcPath{paths, this};
    item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
    itemGroup()->push_back(item);

    //    if (App::gcSettings().simplifyHldi()) {
    //        auto item = new Gi::GcPath{g0path_, this};
    //        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        auto color = new QColor{App::settings().guiColor{GuiColors::G0});
    //        color->setAlpha(127);
    //        item->setPenColorPtr(color);
    //        itemGroup()->push_back(item);
    //        //        ClipperOffset offset;
    //        //        offset.AddPaths(g0path_, JoinType::Round, EndType::Round);
    //        //        offset.Execute(g0path_,uScale*gcp_.getToolDiameter());
    //        //        item = new GcPathItem{g0path_, this};
    //        //        item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    //        itemGroup()->push_back(item);
    //    } else {
    item = new Gi::GcPath{paths, this};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
    itemGroup()->push_back(item);

    item = new Gi::GcPath{g0path_, this};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
    //    }
}

/////////////////////////////////////////////////////////////

void calcArcs(Path path) {
    return;
    //    if (!App::isDebug())
    //        return;
    //    auto addPoint = [](const QPointF& pos, const QColor& color = QColor(255, 255, 255)) {
    //        QGraphicsLineItem* item;
    //        item = App::grView().scene()->addLine(.0, +.1, .0, -.1, QPen(color, 0.0));
    //        item->setPos(pos);
    //        item = App::grView().scene()->addLine(+.1, .0, -.1, .0, QPen(color, 0.0));
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
    //        //                App::grView().scene()->addLine(line, QPen(QColor(0, 255, 0), 0.0));
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

    //                App::grView().scene()->addEllipse(rect, QPen(Qt::red, 0.0), Qt::NoBrush)->setPos(center);
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

} // namespace GCode
