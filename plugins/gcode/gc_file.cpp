/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_file.h"
#include "gc_highlighter.h"
#include "gc_jsproxy.h"
#include "gc_node.h"
#include "gc_plugin.h"

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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QRegularExpression>
#include <algorithm>

#undef emit
#include <execution>
#define emit

namespace GCode {

void regenerateGCodeFiles() {
    // r::for_each(App::project().files<File>(), &File::regenerate);
    auto files = App::project().files<File>();
    std::for_each( // std::execution::par_unseq,
        files.begin(), files.end(),
        [](File* f) { f->regenerate(); });
}

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

static bool jsEvalFile(QJSEngine& engine, const QString& path) {
    QFile f{path};
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "GCode JS: cannot open" << path;
        return false;
    }
    auto result = engine.evaluate(QString::fromUtf8(f.readAll()), path);
    if(result.isError()) {
        qWarning() << "GCode JS error in" << path
                   << "line" << result.property(u"lineNumber"_s).toInt()
                   << ":" << result.toString();
        return false;
    }
    return true;
}

bool File::runJsScript(const QString& scriptPath) {
    QJSEngine engine;
    engine.installExtensions(QJSEngine::ConsoleExtension);
    GcFileProxy proxy{this, &engine};
    auto proxyVal = engine.newQObject(&proxy);

    // Load common_gcode.js from the same directory before the plugin script
    const QString commonPath = QFileInfo{scriptPath}.dir().filePath(u"common_gcode.js"_s);
    if(QFile::exists(commonPath) && !jsEvalFile(engine, commonPath))
        return false;

    if(!jsEvalFile(engine, scriptPath))
        return false;

    auto generateFn = engine.globalObject().property(u"generate"_s);
    if(!generateFn.isCallable()) {
        qWarning() << "GCode JS:" << scriptPath << "has no generate() function";
        return false;
    }

    auto callResult = generateFn.call({proxyVal});
    if(callResult.isError()) {
        qWarning() << "GCode JS generate() error in" << scriptPath
                   << "line" << callResult.property(u"lineNumber"_s).toInt()
                   << ":" << callResult.toString();
        return false;
    }

    return true;
}

void File::ensureDefaultScripts() {
    static bool done = false;
    if(done) return;
    done = true;

    const QString scriptsDirPath = QCoreApplication::applicationDirPath() + u"/scripts"_s;
    QDir{}.mkpath(scriptsDirPath);

    // Extract all embedded script resources to disk if they are missing
    const QDir resDir{u":/gcode/scripts"_s};
    for(const QString& fileName: resDir.entryList(QDir::Files)) {
        const QString destPath = scriptsDirPath + u'/' + fileName;
        if(!QFile::exists(destPath)) {
            QFile src{u":/gcode/scripts/"_s + fileName};
            QFile dst{destPath};
            if(src.open(QIODevice::ReadOnly) && dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
                dst.write(src.readAll());
        }
    }

    // Set default script path for plugins that have no configured script
    for(auto& [type, ptr]: App::gCodePlugins()) {
        const QString gcName = ptr->gcName();
        if(gcName.isEmpty()) continue;
        if(App::gcSettings().scriptPaths_.value(gcName).isEmpty()) {
            const QString defaultPath = scriptsDirPath + u'/' + gcName.toLower() + u".js"_s;
            if(QFile::exists(defaultPath))
                App::gcSettings().scriptPaths_[gcName] = defaultPath;
        }
    }
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

void File::regenerate() {
    initSave();
    addInfo();
    statFile();

    bool jsRan = false;
    if(auto* plugin = App::gCodePlugin(type())) {
        const QString scriptPath = App::gcSettings().scriptPath(plugin->gcName());
        if(!scriptPath.isEmpty() && QFile::exists(scriptPath))
            jsRan = runJsScript(scriptPath);
    }
    // assert(jsRan);
    if(!jsRan)
        try {
            genGcodeAndTile();
        } catch(...) {
        }

    endFile();
}

bool File::save(const QString& name) {
    if(name.isEmpty()) return false;

    regenerate();

    setLastDir(name);
    name_ = name;
    QFile file{name_};
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out{&file};
        out.setEncoding(QStringConverter::Utf8);
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
    for(size_t i{}; i < CMD_LIST.size(); ++i) {
        const int index = format.indexOf(CMD_LIST[i], 0, Qt::CaseInsensitive);
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

// Из чего посчитана программа: по строке на исходный файл с числом взятых из
// него элементов. Ключ UsedItems -- пара {id файла, тип элементов}, поэтому один
// и тот же файл может дать несколько строк, если брали из разных его слоёв.
void File::addSourceInfo() {
    auto it = gcp.params.find(Params::GrItems);
    if(it == gcp.params.end()) return;

    for(auto&& [key, ids]: it->second.value<UsedItems>()) {
        if(key.empty()) continue;
        // Файл мог быть удалён уже после расчёта: addInfo() зовётся при каждом
        // regenerate(), в том числе после перезагрузки проекта.
        auto* source = App::project().file(key.front());
        lines_.emplace_back(QObject::tr(";\t         Source: %1 [%2]")
                .arg(source ? source->shortName() : QObject::tr("#%1 (deleted)").arg(key.front()))
                .arg(ids.size()));
    }
}

void File::addInfo() {
    const static auto side_{QObject::tr("Top|Bottom").split(u'|')};
    if(App::gcSettings().info()) {
        lines_.emplace_back(QObject::tr(";\t           Name: %1").arg(shortName()));
        if(!programName_.isEmpty())
            lines_.emplace_back(QObject::tr(";\t        Program: %1").arg(programName_));
        addSourceInfo();
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
    qApp->clipboard()->setText(lines_
        | v::filter([](QString& str) { return !str.startsWith(u';'); })
        | v::join_with(u'\n')
        | r::to<QString>());
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

std::vector<Geo::Polylines> File::mirrorAndOffsetCurves(const QPointF& offset) {
    std::vector<Geo::Polylines> curvess;
    curvess.reserve(gcp.toolPathss.size());
    for(const Geo::Polylines& curves: gcp.toolPathss)
        curvess.emplace_back(mirrorAndOffsetCurves(offset, curves));
    return curvess;
}

Geo::Polylines File::mirrorAndOffsetCurves(const QPointF& offset, Geo::Polylines curves) {
    if(curves.empty()) curves = gcp.toolPathss.front(); // FIXME wtf

    Geo::translate(curves, offset /*- App::zero().pos()*/);

    if(side_ == Bottom) {
        const double k = Gi::Pin::minX() + Gi::Pin::maxX();
        if(toolType != Tool::Laser) r::for_each(curves, &Geo::Polyline::reverse);
        for(Geo::Vertex& v: v::join(curves)) {
            v.rx()  = -v.x() + k;
            v.bulge = -v.bulge; // зеркало разворачивает обход дуги
        }
    }

    Geo::translate(curves, -App::zero().pos());

    return curves;
}

std::vector<double> File::getDepths() {
    auto& tool = gcp.tool();
    const auto gDepth{gcp.getDepth()};
    if(gDepth < tool.passDepth() || qFuzzyCompare(gDepth, tool.passDepth()))
        return {-gDepth - tool.getDepth()};

    const int count    = static_cast<int>(ceil(gDepth / tool.passDepth()));
    const double depth = gDepth / count;
    std::vector<double> depths(count);
    for(int i{}; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.back() = -gDepth - tool.depth();
    return depths;
}

std::vector<QString> File::savePath(const Geo::Polyline& curve, double perimeter, double depth) {
    std::vector<QString> lines;
    lines.reserve(curve.size());

    // Дуга -- свойство пары вершин: центр (а с ним I/J и выбор G2/G3) считается
    // по прогибу НАЧАЛЬНОЙ вершины сегмента, отдельно взятая вершина о дуге
    // ничего не знает.
    auto getLine = [this](const Geo::Vertex& fr, const Geo::Vertex& to) -> QString {
        if(auto arc = Geo::arcOf(fr, to, fr.bulge)) {
            auto [I, J] = arc->center - static_cast<const QPointF&>(fr);
            return formated({g(fr), x(to.x()), y(to.y()), z(z_), i(I), j(J), strFeed, strSpindle});
        } else
            return formated({g1(), x(to.x()), y(to.y()), z(z_), strFeed, strSpindle});
    };
    // Ход, не выражающийся в выводе, ПРОПУСКАЕТСЯ, а не пишется как есть.
    // format даёт три значащие цифры, и у сегмента короче этого разрешения
    // (точный домен, переведённый в прогибы, оставляет обрывки в десятки
    // нанометров) X и Y округляются до прежних, formated их подавляет как
    // неизменившиеся -- и от дуги остаются одни I/J. Строка `G2 I.. J..` без
    // координат означает для станка ПОЛНЫЙ КРУГ: это и есть завитки на стыках.
    //
    // Сравнение идёт с последней НАПИСАННОЙ точкой, а не с началом сегмента:
    // подряд идущие обрывки иначе накопились бы в заметный сдвиг.
    QPointF last = curve.front();
    auto representable = [&last](const Geo::Vertex& to) {
        return format(to.x()) != format(last.x()) || format(to.y()) != format(last.y());
    };

    // Z по спирали набирается на ВСЕХ сегментах, включая пропущенные, иначе
    // проход не дойдёт до нужной глубины.
    const double zk = depth && perimeter ? depth - z_ : 0.0;
    const double len = zk ? curve.perimeter() : 0.0;

    for(auto&& [fr, to]: Geo::segments(curve)) {
        if(zk) z_ += Geo::segmentLength(fr, to) / len * zk;
        if(!representable(to)) continue;
        lines.emplace_back(getLine(fr, to));
        last = to;
    }
    return lines;
}

QString File::formated(const std::vector<QString>& data) {
    QString ret;
    for(const QString& str: data | v::filter(&QString::size)) {
        // if(str.isEmpty())continue
        ssize_t index = CMD_LIST.indexOf(str.front(), 0, Qt::CaseInsensitive);
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

// Направление обхода задаёт знак прогиба сегмента, начинающегося в вершине.
QString File::g(const Geo::Vertex& v) {
    switch(v.dir()) {
    case Geo::Vertex::Line: return g1();
    case Geo::Vertex::Ccw : return g3();
    case Geo::Vertex::Cw  : return g2();
    }
    return {};
}

QString File::format(double val) {
    QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
    if(str.contains(u'e'))
        return QString::number(val, 'f', 3);
    return str;
    // return QString::fromStdString(std::format("{:1.3f}", val));
}

/////////////////////////////////////////////////////////////
void File::saveDrill(const QPointF& offset) {
    if(gcp.toolPathss.empty()) return;
    Geo::Polyline path = mirrorAndOffsetCurves(offset, gcp.toolPathss.front()).front();
    const std::vector<double> depths(getDepths());
    for(auto&& point: path) {
        startPath(point);
        size_t i{};
        while(true) {
            lines_.emplace_back(formated({g1(), z(depths[i]), strPlungeFeed}));
            if(++i == depths.size()) break;
            if(gcp.tool().lenght() > depths[i])
                lines_.emplace_back(formated({g0(), z(depths[i] - gcp.tool().oneTurnCut() * 4)}));
            else
                lines_.emplace_back(formated({g0(), z(0)}));
        }
        endPath();
    }
}

void File::saveLaserHLDI(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().laserConstOn());

    std::vector<Geo::Polylines> pathss = mirrorAndOffsetCurves(offset);

    int i{};

    lines_.emplace_back(formated({g0(), x(pathss.front().front().front().x()), y(pathss.front().front().front().y()), z(0.0)}));

    for(Geo::Polyline& path: pathss.front()) {
        if(i++ % 2) {
            lines_.append_range(savePath(path, spindleSpeed));
        } else {
            lines_.append_range(savePath(path, 0));
        }
    }
    if(pathss.size() > 1) {
        lines_.emplace_back(App::gcSettings().laserDynamOn());
        for(Geo::Polyline& path: pathss.back()) {
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

    std::vector<Geo::Polylines> pathss = mirrorAndOffsetCurves(offset);

    for(Geo::Polylines& paths: pathss) {
        for(Geo::Polyline& path: paths) {
            startPath(path.front());
            auto sp(savePath(path, spindleSpeed));
            lines_.append_range(sp);
            endPath();
        }
    }
}

void File::saveMillingPocket(const QPointF& offset) {
    // lines_.emplace_back(App::gcSettings().spindleOn());

    const std::vector<double> depths = getDepths();
    double diameter                  = tool().diameter();

    std::vector<Geo::Polylines> pathss = mirrorAndOffsetCurves(offset);

    QPointF point = pathss.front().front().front();

    startPath(point);
    for(const Geo::Polylines& paths: pathss) {
        for(double zd: depths) {
            for(const Geo::Polyline& path: paths) {
                // Прежний критерий: |расстояние - diameter| <= 2*diameter, то есть
                // переезд длиннее трёх диаметров -- разрыв, нужен подъём.
                bool first = Geo::distance(std::exchange(point, path.front()), path.front()) > diameter * 3.0;
                if(first) {
                    endPath();
                    startPath(point);
                }
                // if(first || (paths.front().front() == path.front())) {
                //     lines_.emplace_back(formated({g1(), x(point.x()), y(point.y())})); // start xy
                //     lines_.append_range(savePath(path, path.perimeter(), zd));
                //     // lines_.append_range(savePath(path));
                // } else {
                lines_.emplace_back(formated({g1(), x(point.x()), y(point.y())})); // start xy
                lines_.emplace_back(formated({g1(), z(z_ = zd), strPlungeFeed}));  // start z0 surface
                lines_.append_range(savePath(path));
                // }
            }
        }
    }
    endPath();
}

void File::saveMillingProfile(const QPointF& offset) {
    const std::vector<double> depths(getDepths());
    std::vector<Geo::Polylines> pathss = mirrorAndOffsetCurves(offset);

    // Спиральное врезание можно выключить: тогда на каждый уровень идёт
    // вертикальный плунж, а финальной подчистки дна не нужно вовсе -- уступа от
    // спирали не остаётся.
    const bool spiral = gcp.spiralRamp();

    for(const Geo::Polylines& paths: pathss) {
        if(paths.size() == 1) {
            const Geo::Polyline& path = paths.front();
            double perimeter          = path.perimeter();
            if(paths.front().isClosed()) { // Spiral
                startPath(path.front());
                for(double depth: depths)
                    if(spiral)
                        lines_.append_range(savePath(path, perimeter, depth));
                    else {
                        lines_.emplace_back(formated({g1(), z(z_ = depth), strPlungeFeed}));
                        lines_.append_range(savePath(path));
                    }
                if(spiral)
                    lines_.append_range(savePath(path)); // Проход без спирали.
                endPath();
            } else { // Zigzag
                startPath(path.front());
                const Geo::Polyline reversed = paths.front().reversed();
                uint i{};
                for(double depth: depths)
                    if(spiral)
                        lines_.append_range(savePath(i++ & 1u ? reversed : path, perimeter, depth));
                    else {
                        // Направление всё равно чередуем: врезаться дешевле там,
                        // где инструмент уже стоит.
                        lines_.emplace_back(formated({g1(), z(z_ = depth), strPlungeFeed}));
                        lines_.append_range(savePath(i++ & 1u ? reversed : path));
                    }
                if(spiral)
                    lines_.append_range(savePath(i & 1u ? reversed : path)); // Проход без спирали.
                endPath();
            }
        } else {
            // tool().diameter();
            // double perimeter = r::fold_left(
            //     v::transform(paths, std::bind(&Geo::Polyline::perimeter, _1)),
            //     0.0, std::plus<double>{});
            startPath(paths.front().front());
            for(double zd: depths) {
                for(const Geo::Polyline& path: paths) {
                    QPointF point = path.front();
                    lines_.emplace_back(formated({g0(), x(point.x()), y(point.y())})); // start xy
                    lines_.emplace_back(formated({g1(), z(z_ = zd), strPlungeFeed}));  // start z0 surface
                    lines_.append_range(savePath(path));
                    lines_.emplace_back(formated({g0(), z(0)}));
                }
            }
            endPath();
        }
    }
}

void File::saveMillingRaster(const QPointF& offset) {
    lines_.emplace_back(App::gcSettings().spindleOn());

    std::vector<Geo::Polylines> pathss = mirrorAndOffsetCurves(offset);
    const std::vector<double> depths(getDepths());

    for(Geo::Polylines& paths: pathss) {
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
    if(!gcp.toolPathss.size()) return;
    Gi::Item* item;
    for(QPointF point: gcp.toolPathss.front().front()) {
        item = new Gi::Drill{{point}, gcp.tool().diameter(), this, gcp.tool().id()};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }
    item = new Gi::GcPath{{gcp.toolPathss.front().front()}};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiLaser() {
    Geo::Polylines paths;

    paths.reserve(gcp.toolPathss.front().size() / 2 + 1);
    g0path_.reserve(paths.size());
    for(size_t i{}; i < gcp.toolPathss.front().size(); ++i)
        if(i % 2)
            paths.push_back(gcp.toolPathss.front()[i]);
        else
            g0path_.push_back(gcp.toolPathss.front()[i]);
    if(gcp.toolPathss.size() > 1) {
        paths.insert(paths.end(), gcp.toolPathss[1].begin(), gcp.toolPathss[1].end());
        g0path_.push_back({gcp.toolPathss[0].back().back(), gcp.toolPathss[1].front().front()});
        for(size_t i{}; i < gcp.toolPathss[1].size() - 1; ++i)
            g0path_.push_back({gcp.toolPathss[1][i].back(), gcp.toolPathss[1][i + 1].front()});
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
    // // offset.AddPaths(g0path_, cl::JoinType::Round, cl::EndType::Round);
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

    for(size_t i{}; const Geo::Polylines& paths: gcp.toolPathss) {
        int k = static_cast<int>((gcp.toolPathss.size() > 1) ? (300.0 / (gcp.toolPathss.size() - 1)) * i : 0);
        debugColor.emplace_back(QSharedPointer<QColor>(new QColor{QColor::fromHsv(k, 255, 255, 255)}));

        for(const Geo::Polyline& path: paths) {
            item = new Gi::GcPath{{path}, this};
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
            itemGroup()->push_back(item);
        }

        // перебежки между соседними путями -- каждая отдельным отрезком
        Geo::Polylines g1path;
        g1path.reserve(paths.size());
        for(auto&& [fr, to]: v::pairwise(paths))
            g1path.push_back(Geo::Polyline{{fr.back()}, {to.front()}});
        item = new Gi::GcPath{g1path};
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setPen({Qt::magenta, 0.0});
        itemGroup()->push_back(item);
    }

    g0path_.reserve(gcp.toolPathss.size());
    for(auto&& [fr, to]: v::pairwise(gcp.toolPathss))
        g0path_.push_back({{fr.back().back()}, {to.front().front()}});

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

void File::createGiProfile() {

    Gi::Item* item;
    for(const Geo::Polylines& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    g0path_.clear();

    for(const Geo::Polylines& paths: gcp.toolPathss) {
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
    // int k = static_cast<int>((gcp.toolPathss.size() > 1) ? (300.0 / (gcp.toolPathss.size() - 1)) * i : 0);
    // QColor* c = new QColor;
    // *c = QColor::fromHsv(k, 255, 255, 255);

    auto& pocketAreaCurves = gcp.pocketAreaCurves();

    Gi::Item* item;
    g0path_.reserve(gcp.toolPathss.size());

    if(pocketAreaCurves.size()) {
        item = new Gi::DataFill{pocketAreaCurves, nullptr}; // FIXME const_cast
        item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        item->setAcceptHoverEvents(false);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        itemGroup()->push_back(item);
    } else {
        for(const Geo::Polylines& paths: gcp.toolPathss) {
            item = new Gi::GcPath{paths, this};
            item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
            itemGroup()->push_back(item);
        }
    }
    size_t i{};

    // for (int i {}; auto& path : v::join(gcp.toolPathss)) { }

    for(const Geo::Polylines& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
        for(size_t j{}; j < paths.size() - 1; ++j)
            g0path_.push_back({paths[j].back(), paths[j + 1].front()});
        if(i < gcp.toolPathss.size() - 1)
            g0path_.push_back({gcp.toolPathss[i].back().back(), gcp.toolPathss[++i].front().front()});
    }

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

} // namespace GCode
