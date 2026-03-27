/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_file.h"
#include "gc_node.h"

#include "app.h"
#include "gi.h"
#include "gi_datasolid.h"
#include "gi_dbg.h"
#include "gi_drill.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "math.h"
#include "plugintypes.h"
#include "project.h"
#include "span.h"

#include <QFileInfo>
#include <QRegularExpression>

using namespace geo;

namespace GCode {

QString File::getLastDir() {
    if(App::gcSettings().sameFolder() && !redirected)
        lastDir = QFileInfo(App::project().name()).absolutePath();
    else if(lastDir.isEmpty()) {
        QSettings settings;
        lastDir = settings.value(u"LastGCodeDir"_s).toString();
        if(lastDir.isEmpty())
            lastDir = QFileInfo(App::project().name()).absolutePath();
        settings.setValue(u"LastGCodeDir"_s, lastDir);
    }
    return lastDir += u'/';
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
        settings.setValue(u"LastGCodeDir"_s, lastDir);
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
    QFile file{name_};
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out{&file};
        QString str;
        for(QString& s: lines_) {
            if(!s.isEmpty())
                str.push_back(s);
            if(!str.endsWith(u'\n'))
                str.push_back(u'\n');
        }
        out << str;
    } else
        return false;
    file.close();
    return true;
}

void File::initSave() {
    lines_.clear();

    for(bool& fl: formatFlags)
        fl = false;

    const QString format(gcp.tool().type() == Tool::Laser ? App::gcSettings().formatLaser() : App::gcSettings().formatMilling());
    for(size_t i{}; i < cmdList.size(); ++i) {
        const int index = format.indexOf(cmdList[i], 0, Qt::CaseInsensitive);
        if(index != -1) {
            formatFlags[i + AlwaysG] = format[index + 1] == u'+';
            if((index + 2) < format.size())
                formatFlags[i + SpaceG] = format[index + 2] == u' ';
        }
    }

    for(QString& str: lastValues)
        str.clear();

    // setFeedRate(gcp.getTool().feedRate);
    // setPlungeRate(gcp.getTool().plungeRate());
    // setSpindleSpeed(gcp.getTool().spindleSpeed);
    // setToolType(gcp.getTool().type());
}

void File::statFile() {
    if(toolType == Tool::Laser) {
        QString str(App::gcSettings().laserStart()); // u"G21 G17 G90"_s); //G17 XY plane
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(0)})); // Z0 for visible in Candle
    } else {
        QString str(App::gcSettings().start()); // u"G21 G17 G90"_s); //G17 XY plane
        str.replace(QRegularExpression(u"S\\?"_s), formated({strSpindle}));
        lines_.emplace_back(str);
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
    }
}

void File::addInfo() {
    const static auto side_{QObject::tr("Top|Bottom").split(u'|')};
    if(App::gcSettings().info()) {
        lines_.emplace_back(QObject::tr(";\t           Name: %1").arg(shortName()));
        lines_.emplace_back(QObject::tr(";\t           Tool: %1").arg(gcp.tool().name()));
        lines_.emplace_back(QObject::tr(";\t  Tool Stepover: %1").arg(gcp.tool().stepover()));
        lines_.emplace_back(QObject::tr(";\t Feed Rate mm/s: %1").arg(gcp.tool().feedRate_mmPerSec()));
        lines_.emplace_back(QObject::tr(";\tTool Pass Depth: %1").arg(gcp.tool().passDepth()));
        lines_.emplace_back(QObject::tr(";\t          Depth: %1").arg(gcp.getDepth()));
        lines_.emplace_back(QObject::tr(";\t           Side: %1").arg(side_[side()]));
    }
}

void File::endFile() {
    if(toolType == Tool::Laser) {
        lines_.emplace_back(App::gcSettings().spindleLaserOff());
        QPointF home(App::home().pos() - App::zero().pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
        lines_.emplace_back(App::gcSettings().laserEnd());
    } else {
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
        // QPointF home(App::home().pos() - App::zero().pos()); // FIXME
        // lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
        lines_.emplace_back(App::gcSettings().end());
    }

    std::erase_if(lines_, std::bind(&QString::isEmpty, _1)); // remove epty lines

    qApp->clipboard()->setText(lines_ | v::join_with(u'\n') | r::to<QString>());
}

FileTree::Node* File::node() { return node_ ? node_ : node_ = new Node{this}; }

void File::startPath(const QPointF& point) {
    if(toolType == Tool::Laser) {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), speed(0)})); // start xy
        // gCodeText_.push_back(formated({ g1(), strSpindle }));
    } else {
        lines_.emplace_back(formated({g0(), x(point.x()), y(point.y()), strSpindle})); // start xy
        lines_.emplace_back(formated({g0(), z(App::project().plunge())}));             // start plunge
        lines_.emplace_back(formated({g1(), z(z_ = 0), strPlungeFeed}));               // start z0 surface
        // lastValues[AlwaysF].clear();
    }
}

void File::endPath() {
    if(toolType == Tool::Laser) {
        //
    } else {
        lines_.emplace_back(formated({g0(), z(App::project().clearence())}));
    }
}

Curvess File::mirrorAndOffsetCurves(const QPointF& offset) {
    auto& toolPathss = gcp.toolPathss();

    return toolPathss;
    Curvess curvess;
    curvess.reserve(toolPathss.size());
    for(Curves curves: toolPathss)
        curvess.emplace_back(mirrorAndOffsetCurves(offset, curves));

    return curvess;
}

Curves File::mirrorAndOffsetCurves(const QPointF& offset, Curves curves) {
    auto& toolPathss = gcp.toolPathss();
    if(curves.empty()) curves = toolPathss.front(); // FIXME wtf

    r::for_each(curves, std::bind(TranslateCurve, _1, offset /*- App::zero().pos()*/));

    if(side_ == Bottom) {
        const double k = Gi::Pin::minX() + Gi::Pin::maxX();
        if(toolType != Tool::Laser) r::for_each(curves, &Curve::reverse);
        for(Vertex& v: v::join(curves)) {
            v.pt.rx() = -v.pt.x() + k;
            if(v) v.center.rx() = -v.center.x() + k;
        }
    }

    for(Vertex& v: v::join(curves)) { // FIXME ^^^^^^^^^
        v.pt -= App::zero().pos();
        if(v) v.center -= App::zero().pos();
    }

    return curves;
}

mvector<double> File::getDepths() {
    auto& tool = gcp.tool();
    const auto gDepth{gcp.getDepth()};
    if(gDepth < tool.passDepth() || qFuzzyCompare(gDepth, tool.passDepth()))
        return {-gDepth - tool.getDepth()};

    const int count = static_cast<int>(ceil(gDepth / tool.passDepth()));
    const double depth = gDepth / count;
    mvector<double> depths(count);
    for(int i{}; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.back() = -gDepth - tool.depth();
    return depths;
}

std::vector<QString> File::savePath(const Curve& curve, double depth) {
    std::vector<QString> lines;
    lines.reserve(curve.size());
    bool skip = true;

    if(depth) {
        double zk = depth - z_;
        double perimetr = curve.perimetr();
        qWarning() << "perimetr" << perimetr;
        for(Vertex prevPt; const Vertex& to: curve)
            if(skip) {
                prevPt = to;
                skip = false;
            } else {
                z_ += Span{prevPt.pt, to}.Length() / perimetr * zk;
                if(to.type) {
                    auto [I, J] = to.center - prevPt.pt;
                    lines.emplace_back(formated({g(to), x(to.x()), y(to.y()), z(z_), i(I), j(J), strFeed, strSpindle}));
                } else
                    lines.emplace_back(formated({g1(), x(to.x()), y(to.y()), z(z_), strFeed, strSpindle}));
                prevPt = to;
            }
    } else {
        qWarning() << "else";
        for(auto&& [fr, to]: curve | v::pairwise) {
            if(to.type) {
                auto [I, J] = to.center - fr.pt;
                lines.emplace_back(formated({g(to), x(to.x()), y(to.y()), z(z_), i(I), j(J), strFeed, strSpindle}));
            } else
                lines.emplace_back(formated({g1(), x(to.x()), y(to.y()), strFeed, strSpindle}));
        }
    }
    return lines;
}

QString File::formated(const std::vector<QString>& data) {
    QString ret;
    for(const QString& str: data) {
        const int index = cmdList.indexOf(str.front().toUpper());
        if(index != -1) {
            if(formatFlags[AlwaysG + index] || lastValues[index] != str) {
                lastValues[index] = str;
                ret += str + (formatFlags[SpaceG + index] ? u" " : u"");
            }
        }
    }
    return ret.trimmed();
}

QString File::g0() { return gCode_ = G00, u"G0"_s; }

QString File::g1() { return gCode_ = G01, u"G1"_s; }

QString File::g2() { return gCode_ = G02, u"G2"_s; }

QString File::g3() { return gCode_ = G03, u"G3"_s; }

QString File::g(const Vertex& v) {
    switch(v.type) {
    case Vertex::Line: return g1();
    case Vertex::Ccw : return g3();
    case Vertex::Cw  : return g2();
    }
}

QString File::format(double val) {
    //     QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
    //     if(str.contains(u'e'))
    //         return QString::number(val, 'f', 3);
    //     return str;
    return QString::fromStdString(std::format("{:1.3f}", val));
}

/////////////////////////////////////////////////////////////
void File::saveDrill(const QPointF& offset) {
    auto& toolPathss = gcp.toolPathss();

    Curve path = mirrorAndOffsetCurves(offset, toolPathss.front()).front();

    const std::vector<double> depths(getDepths());

    for(QPointF point: path) {
        startPath(point);
        size_t i{};
        while(true) {
            lines_.emplace_back(formated({g1(), z(depths[i]), strPlungeFeed}));
            if(++i == depths.size())
                break;
            lines_.emplace_back(formated({g0(), z(0.0)}));
        }
        endPath();
    }
}

void File::saveLaserHLDI(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().laserConstOn());

    Curvess pathss = mirrorAndOffsetCurves(offset);

    int i{};

    lines_.emplace_back(formated({g0(), x(pathss.front().front().front().x()), y(pathss.front().front().front().y()), z(0.0)}));

    for(Curve& path: pathss.front()) {
        if(i++ % 2) {
            lines_.append_range(savePath(path, spindleSpeed));
        } else {
            lines_.append_range(savePath(path, 0));
        }
    }
    if(pathss.size() > 1) {
        lines_.emplace_back(App::gcSettings().laserDynamOn());
        for(Curve& path: pathss.back()) {
            startPath(path.front());
            lines_.append_range(savePath(path, spindleSpeed));
            endPath();
        }
    }
}

void File::saveLaserPocket(const QPointF& offset) {
    saveLaserProfile(offset);
}

void File::saveLaserProfile(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().laserDynamOn());

    Curvess pathss = mirrorAndOffsetCurves(offset);

    for(Curves& paths: pathss) {
        for(Curve& path: paths) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed));
            lines_.append_range(sp);
            endPath();
        }
    }
}

void File::saveMillingPocket(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().spindleOn());
    Curvess toolPathss = mirrorAndOffsetCurves(offset);
    const mvector<double> depths(getDepths());
    qCritical("TODO G2 G3");
    for(Curves& paths: toolPathss) {
        startPath(paths.front().front());
        for(size_t i{}; i < depths.size(); ++i) {
            lines_.emplace_back(formated({g1(), z(depths[i]), strPlungeFeed}));
            bool skip = true;
            for(auto& path: paths) {
                for(const Vertex& point: path)
                    if(skip)
                        skip = false;
                    else
                        lines_.emplace_back(formated({g1(), x(point.x()), y(point.y()), strFeed}));
            }
            for(size_t j = paths.size() - 2; j != std::numeric_limits<size_t>::max() && i < depths.size() - 1; --j) {
                QPointF point = paths[j].back();
                lines_.emplace_back(formated({g0(), x(point.x()), y(point.y())}));
            }
            if(paths.size() > 1 && i < (depths.size() - 1))
                lines_.emplace_back(formated({g0(), x(paths.front().front().x()), y(paths.front().front().y())}));
        }
        endPath();
    }
}

void File::saveMillingProfile(const QPointF& offset) {
    const mvector<double> depths(getDepths());
    Curvess toolPathss = mirrorAndOffsetCurves(offset);
    for(auto&& paths: toolPathss) {
        for(size_t i{}; i < depths.size(); ++i) {
            for(size_t j{}; j < paths.size(); ++j) {
                Curve& curve = paths[j];
                // if(curve.front().pt == curve.back().pt) { // make complete depth and remove from worck
                startPath(curve.front().pt);
                for(auto&& depth: depths) lines_.append_range(savePath(curve, depth));
                lines_.append_range(savePath(curve)); // Проход без спирали.
                endPath();
                paths.erase(paths.begin() + j--);
                // } else {
                // startPath(curve.front().pt);
                // lines_.emplace_back(formated({g1(), z(depths[i]), strPlungeFeed}));
                // lines_.append_range(savePath(curve, spindleSpeed));
                // endPath();
                // }
            }
        }
    }
}

void File::saveMillingRaster(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().spindleOn());

    Curvess pathss = mirrorAndOffsetCurves(offset);
    const mvector<double> depths(getDepths());

    for(Curves& paths: pathss) {
        for(size_t i{}; i < depths.size(); ++i) {
            for(auto& path: paths) {
                startPath(path.front());
                lines_.emplace_back(formated({g1(), z(depths[i]), strPlungeFeed}));
                auto sp(savePath(path, spindleSpeed));
                lines_.append_range(sp);
                endPath();
            }
        }
    }
}

void File::createGiDrill() {
    auto& toolPathss = gcp.toolPathss();

    Gi::Item* item;
    for(QPointF point: toolPathss.front().front()) {
        item = new Gi::Drill{{point}, gcp.tool().diameter(), this, gcp.tool().id()};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    item = new Gi::GcPath{{toolPathss.front().front()}};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiLaser() {
    Curves paths;

    auto& toolPathss = gcp.toolPathss();

    paths.reserve(toolPathss.front().size() / 2 + 1);
    g0path_.reserve(paths.size());
    for(size_t i{}; i < toolPathss.front().size(); ++i)
        if(i % 2)
            paths.push_back(toolPathss.front()[i]);
        else
            g0path_.push_back(toolPathss.front()[i]);
    if(toolPathss.size() > 1) {
        paths.insert(paths.end(), toolPathss[1].begin(), toolPathss[1].end());
        g0path_.push_back({toolPathss[0].back().back(), toolPathss[1].front().front()});
        for(size_t i{}; i < toolPathss[1].size() - 1; ++i)
            g0path_.push_back({toolPathss[1][i].back(), toolPathss[1][i + 1].front()});
    }

    auto item = new Gi::GcPath{paths, this};
    item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
    itemGroup()->push_back(item);

    // if (App::gcSettings().simplifyHldi()) {
    // auto item = new Gi::GcPath{g0path_, this};
    // item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    // auto color = new QColor{App::settings().guiColor{GuiColors::G0});
    // color->setAlpha(127);
    // item->setPenColorPtr(color);
    // itemGroup()->push_back(item);
    // // ClipperOffset offset;
    // // offset.AddPaths(g0path_, JoinType::Round, EndType::Round);
    // // offset.Execute(g0path_,uScale*gcp.getToolDiameter());
    // // item = new GcPathItem{g0path_, this};
    // // item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    // itemGroup()->push_back(item);
    // } else {
    item = new Gi::GcPath{paths, this};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
    itemGroup()->push_back(item);

    item = new Gi::GcPath{g0path_, this};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
    // }
}

void File::createGiPocket() {
    auto& toolPathss = gcp.toolPathss();
    auto& pocketAreaCurves = gcp.pocketAreaCurves();

    Gi::Item* item;
    if(pocketAreaCurves.size()) {
        item = new Gi::DataFill{pocketAreaCurves, nullptr};
        item->setPen(Qt::NoPen);
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    }

    for(size_t i{}; const Curves& paths: toolPathss) {
        int k = static_cast<int>((toolPathss.size() > 1) ? (300.0 / (toolPathss.size() - 1)) * i : 0);
        debugColor.emplace_back(QSharedPointer<QColor>(new QColor{QColor::fromHsv(k, 255, 255, 255)}));

        for(const Curve& path: paths) {
            item = new Gi::GcPath{{path}, this};
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
            itemGroup()->push_back(item);
        }

        Curve g1path;
        g1path.reserve(paths.size());
        for(auto&& [fr, to]: v::pairwise(paths))
            g1path.push_back({{fr.back().pt}, {to.front().pt}});
        item = new Gi::GcPath{{g1path}};
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setPen({Qt::magenta, 0.0});
        itemGroup()->push_back(item);
    }

    g0path_.reserve(toolPathss.size());
    for(auto&& [fr, to]: v::pairwise(toolPathss))
        g0path_.push_back({{fr.back().back().pt}, {to.front().front().pt}});

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiProfile() {
    auto& toolPathss = gcp.toolPathss();

    Gi::Item* item;
    for(const Curves& paths: toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    size_t i{};

    g0path_.clear();

    for(const Curves& paths: toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        if(g0path_.size()) g0path_.back().emplace_back(paths.front().front());
        for(auto&& [fr, to]: v::pairwise(paths)) g0path_.push_back({fr.back(), to.front()});
        g0path_.push_back({paths.back().back()});
    }

    // item = new Gi::GcPath{g0path_};
    // // item->setPen(QPen(Qt::black, 0.0)); //, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin));
    // item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    // itemGroup()->push_back(item);
}

void File::createGiRaster() {
    // int k = static_cast<int>((toolPathss.size() > 1) ? (300.0 / (toolPathss.size() - 1)) * i : 0);
    // QColor* c = new QColor;
    // *c = QColor::fromHsv(k, 255, 255, 255);

    auto& toolPathss = gcp.toolPathss();
    auto& pocketAreaCurves = gcp.pocketAreaCurves();

    Gi::Item* item;
    g0path_.reserve(toolPathss.size());

    if(pocketAreaCurves.size()) {
        item = new Gi::DataFill{pocketAreaCurves, nullptr}; // FIXME const_cast
        item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    } else {
        for(const Curves& paths: toolPathss) {
            item = new Gi::GcPath{paths, this};
            item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
    }
    size_t i{};

    // for (int i {}; auto& path : v::join(toolPathss)) { }

    for(const Curves& paths: toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j{}; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < toolPathss.size() - 1)
            g0path_.push_back({toolPathss[i].back().back(), toolPathss[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

} // namespace GCode
