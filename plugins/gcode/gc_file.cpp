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
#include "gc_jsproxy.h"
#include "gc_node.h"
#include "gc_plugin.h"
#include "gc_programdialog.h"

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
#include <QJsonObject>
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

// Текст ошибки: и в журнал, и в саму УП (regenerate пишет его комментарием).
static QString jsErrorText(const QJSValue& err, const QString& path) {
    return QFileInfo{path}.fileName() + u':' + QString::number(err.property(u"lineNumber"_s).toInt())
        + u": "_s + err.toString();
}

static bool jsEvalFile(QJSEngine& engine, const QString& path, QString& error) {
    QFile f{path};
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = u"cannot open "_s + path;
        qWarning() << "GCode JS:" << error;
        return false;
    }
    auto result = engine.evaluate(QString::fromUtf8(f.readAll()), path);
    if(result.isError()) {
        error = jsErrorText(result, path);
        qWarning() << "GCode JS error:" << error;
        return false;
    }
    return true;
}

bool File::runJsScript(const QString& scriptPath) {
    QJSEngine engine;
    engine.installExtensions(QJSEngine::ConsoleExtension);
    GcFileProxy proxy{this, &engine};
    // Объекты живут на стеке C++: сборщику движка их не отдавать.
    QJSEngine::setObjectOwnership(&proxy, QJSEngine::CppOwnership);
    auto proxyVal = engine.newQObject(&proxy);

    // Load common_gcode.js from the same directory before the plugin script
    const QString commonPath = QFileInfo{scriptPath}.dir().filePath(u"common_gcode.js"_s);
    if(QFile::exists(commonPath) && !jsEvalFile(engine, commonPath, jsError_))
        return false;

    if(!jsEvalFile(engine, scriptPath, jsError_))
        return false;

    auto generateFn = engine.globalObject().property(u"generate"_s);
    if(!generateFn.isCallable()) {
        jsError_ = QFileInfo{scriptPath}.fileName() + u": has no generate() function"_s;
        qWarning() << "GCode JS:" << jsError_;
        return false;
    }

    std::unique_ptr<QObject> ext{createJsExtension(proxy, engine)};
    if(ext) {
        QJSEngine::setObjectOwnership(ext.get(), QJSEngine::CppOwnership);
        proxyVal.setProperty(u"ext"_s, engine.newQObject(ext.get()));
    }

    auto callResult = generateFn.call({proxyVal});
    if(callResult.isError()) {
        jsError_ = jsErrorText(callResult, scriptPath);
        qWarning() << "GCode JS generate() error:" << jsError_;
        return false;
    }

    jsError_.clear();
    return true;
}

void File::ensureDefaultScripts() {
    static bool done = false;
    if(done) return;
    done = true;

    const QString scriptsDirPath = QCoreApplication::applicationDirPath() + u"/scripts"_s;
    QDir{}.mkpath(scriptsDirPath);

    // Extract all embedded script resources to disk if they are missing
    // (скрипты плагинов -- в scripts/, посты -- в scripts/posts/).
    auto extract = [](const QString& resPrefix, const QString& dirPath) {
        QDir{}.mkpath(dirPath);
        const QDir resDir{resPrefix};
        for(const QString& fileName: resDir.entryList(QDir::Files)) {
            const QString destPath = dirPath + u'/' + fileName;
            if(!QFile::exists(destPath)) {
                QFile src{resPrefix + u'/' + fileName};
                QFile dst{destPath};
                if(src.open(QIODevice::ReadOnly) && dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
                    dst.write(src.readAll());
            }
        }
    };
    extract(u":/gcode/scripts"_s, scriptsDirPath);
    extract(u":/gcode/scripts/posts"_s, Post::postsDir());

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

void File::setSide(Side side) {
    AbstractFile::setSide(side);
    gcp.params[Params::FileSide].setValue(side);
}

void File::regenerate(unsigned sections) {
    initSave();
    if(sections & Header) writeHeader();
    addInfo();

    if(sections & Body) {
        auto* plugin = App::gCodePlugin(type());
        const QString scriptPath = plugin ? App::gcSettings().scriptPath(plugin->gcName()) : QString{};
        // Генерация -- только скриптом. Фолбэка нет: ошибка видна в самой УП
        // и в журнале, а не маскируется другим генератором.
        if(!plugin) {
            // файл без плагина (отладочный) -- текста не имеет
        } else if(scriptPath.isEmpty() || !QFile::exists(scriptPath)) {
            qCritical() << "GCode: no script for" << plugin->gcName() << scriptPath;
            lines_.emplace_back(comment(u"ERROR: no G-code script for "_s + plugin->gcName()));
        } else if(!runJsScript(scriptPath)) {
            qCritical() << "GCode: script failed" << scriptPath << jsError_;
            lines_.emplace_back(comment(u"ERROR: G-code script failed: "_s + jsError_));
        }
    }

    if(sections & Footer) writeFooter();

    std::erase_if(lines_, std::bind(&QString::isEmpty, _1)); // remove epty lines
    if(sections == All)
        qApp->clipboard()->setText(lines_
            | v::filter([post = &App::gcSettings().post()](QString& str) { return !post->isComment(str); })
            | v::join_with(u'\n')
            | r::to<QString>());
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
        out << renderText(lines_);
    } else
        return false;
    file.close();
    return true;
}

QString File::comment(const QString& text) { return App::gcSettings().post().comment(text); }

QString File::renderText(const std::vector<QString>& lines) { return App::gcSettings().post().renderText(lines); }

Post::Context File::postContext() {
    Post& post = App::gcSettings().post();
    Post::Context ctx;
    ctx.laser = toolType == Tool::Laser;
    ctx.name = shortName();
    ctx.programName = programName_;
    ctx.spindleSpeed = spindleSpeed;
    ctx.feedRate = feedRate;
    ctx.plungeRate = plungeRate;
    QJsonObject json;
    gcp.tool().write(json);
    ctx.tool = post.engine().toScriptValue(json.toVariantMap());
    ctx.properties = GcFileProxy::propertiesValue(post.engine());
    return ctx;
}

void File::parseFormat(const QString& format, bool (&flags)[Size]) {
    for(size_t i{}; i < CMD_LIST.size(); ++i) {
        const int index = format.indexOf(CMD_LIST[i], 0, Qt::CaseInsensitive);
        if(index != -1) {
            flags[i + AlwaysG] = format[index + 1] == u'+';
            if((index + 2) < format.size())
                flags[i + SpaceG] = format[index + 2] == u' ';
        }
    }
}

void File::initSave() {
    lines_.clear();

    for(auto& kind: formatFlags)
        for(bool& fl: kind)
            fl = false;

    Post& post = App::gcSettings().post();
    const bool laser = gcp.tool().type() == Tool::Laser;
    parseFormat(post.formatLinear(laser), formatFlags[Linear]);
    parseFormat(post.formatArc(laser), formatFlags[Arc]);

    for(QString& str: lastValues)
        str.clear();
    for(bool& written: lastWritten)
        written = false;
    gCode_ = GNull;
}

void File::writeHeader() {
    Post& post = App::gcSettings().post();
    lines_.append_range(post.header(postContext()));
    if(toolType == Tool::Laser)
        lines_.emplace_back(formated({g0(), z(0)})); // Z0 for visible in Candle
    else
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
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
        lines_.emplace_back(comment(QObject::tr("\t         Source: %1 [%2]")
                .arg(source ? source->shortName() : QObject::tr("#%1 (deleted)").arg(key.front()))
                .arg(ids.size())));
    }
}

void File::addInfo() {
    const static auto side_{QObject::tr("Top|Bottom").split(u'|')};
    if(App::gcSettings().info()) {
        lines_.emplace_back(comment(QObject::tr("\t           Name: %1").arg(shortName())));
        if(!programName_.isEmpty())
            lines_.emplace_back(comment(QObject::tr("\t        Program: %1").arg(programName_)));
        addSourceInfo();
        lines_.emplace_back(comment(QObject::tr("\t           Tool: %1").arg(gcp.tool().name())));
        lines_.emplace_back(comment(QObject::tr("\t  Tool Stepover: %1").arg(gcp.tool().stepover())));
        lines_.emplace_back(comment(QObject::tr("\t Feed Rate mm/s: %1").arg(gcp.tool().feedRate_mmPerSec())));
        lines_.emplace_back(comment(QObject::tr("\tTool Pass Depth: %1").arg(gcp.tool().passDepth())));
        lines_.emplace_back(comment(QObject::tr("\t          Depth: %1").arg(gcp.getDepth())));
        lines_.emplace_back(comment(QObject::tr("\t           Side: %1").arg(side_[side()])));
    }
}

void File::writeFooter() {
    Post& post = App::gcSettings().post();
    const Post::Context ctx = postContext();
    if(toolType == Tool::Laser) {
        lines_.emplace_back(post.laserOff(ctx));
        QPointF home(App::home().pos() - App::zero().pos());
        lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
    } else {
        lines_.emplace_back(formated({g0(), z(App::project().safeZ())})); // HomeZ
        // QPointF home(App::home().pos() - App::zero().pos()); // FIXME
        // lines_.emplace_back(formated({g0(), x(home.x()), y(home.y())})); // HomeXY
    }
    lines_.append_range(post.footer(ctx));
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
            v.rx() = -v.x() + k;
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

    const int count = static_cast<int>(ceil(gDepth / tool.passDepth()));
    const double depth = gDepth / count;
    std::vector<double> depths(count);
    for(int i{}; i < count; ++i)
        depths[i] = (i + 1) * -depth;
    depths.back() = -gDepth - tool.depth();
    return depths;
}

std::vector<QString> File::savePath(const Geo::Polyline& srcCurve, double perimeter, double depth) {
    std::vector<QString> lines;
    lines.reserve(srcCurve.size());

    // Дуга, разрезанная на куски, -- по-прежнему одна дуга, и станку она
    // говорится одной строкой. Geo сшивает их у себя (Geo::stitchArcs в
    // toPolyline), но путь до вывода идёт не только оттуда: его перебирают и
    // складывают плагины, зеркалят по стороне платы, сдвигают по тайлам. Здесь
    // последняя точка перед станком -- тут и проверяем.
    Geo::Polyline curve = srcCurve;
    Geo::stitchArcs(curve);

    // Дуга -- свойство пары вершин: центр (а с ним I/J и выбор G2/G3) считается
    // по прогибу НАЧАЛЬНОЙ вершины сегмента, отдельно взятая вершина о дуге
    // ничего не знает.
    auto emitSegment = [this, &lines](const Geo::Vertex& fr, const Geo::Vertex& to) {
        if(auto arc = Geo::arcOf(fr, to, fr.bulge))
            arcLines(lines, fr, to, *arc);
        else
            lines.emplace_back(formated({g1(), x(to.x()), y(to.y()), z(z_), strFeed, strSpindle}));
    };
    // Ход, не выражающийся в выводе, ПРОПУСКАЕТСЯ, а не пишется как есть.
    // Сегмент короче единицы вывода (точный домен, переведённый в прогибы,
    // оставляет обрывки в десятки нанометров) даёт те же X и Y, formated
    // подавляет их как неизменившиеся -- и от дуги остаются одни I/J. Строка
    // `G2 I.. J..` без координат означает для станка ПОЛНЫЙ КРУГ: это и есть
    // завитки на стыках.
    //
    // Сравнение идёт с последней НАПИСАННОЙ точкой, а не с началом сегмента:
    // подряд идущие обрывки иначе накопились бы в заметный сдвиг.
    QPointF last = curve.front();
    auto representable = [&last](const Geo::Vertex& to) {
        return toUnits(to.x()) != toUnits(last.x()) || toUnits(to.y()) != toUnits(last.y());
    };

    // Z по спирали набирается на ВСЕХ сегментах, включая пропущенные, иначе
    // проход не дойдёт до нужной глубины.
    const double zk = depth && perimeter ? depth - z_ : 0.0;
    const double len = zk ? curve.perimeter() : 0.0;

    // ОКРУЖНОСТЬ -- одной командой. Прогибом полный оборот не выражается, и в
    // геометрии она живёт двумя половинами (две вершины с прогибом +-1);
    // станку же её говорят строкой «G2/G3 I.. J..» без координат конца -- конец
    // и есть начало. Координаты не пишутся сами: они не изменились, и formated
    // подавит их как неизменившиеся.
    //
    // На спуске по спирали (zk) круг остаётся двумя половинами: одной командой
    // Z по дуге не набрать.
    //
    // Одной командой окружность выражается только словами I/J: у R полный
    // оборот неоднозначен, а хордами он и так идёт по точкам, -- в этих
    // диалектах она остаётся двумя половинами.
    if(!zk && curve.closed && curve.size() == 2
        && App::gcSettings().post().arcs() == Post::Arcs::IJ
        && curve.front().bulge == curve.back().bulge
        && std::abs(std::abs(curve.front().bulge) - 1.0) < 1e-9) {
        const Geo::Vertex& start = curve.front();
        if(auto arc = Geo::arcOf(start, curve.back(), start.bulge)) {
            auto [I, J] = arc->center - static_cast<const QPointF&>(start);
            lines.emplace_back(formated({g(start), x(start.x()), y(start.y()), z(z_), i(I), j(J), strFeed, strSpindle}));
            return lines;
        }
    }

    for(auto&& [fr, to]: Geo::segments(curve)) {
        if(zk) z_ += Geo::segmentLength(fr, to) / len * zk;
        if(!representable(to)) continue;
        emitSegment(fr, to);
        last = to;
    }
    return lines;
}

// Дуга fr -> to в диалекте поста. Z на входе уже набран для конца сегмента
// (спираль), так что и хорды получают конечный z_: точнее по каждой хорде
// набирать нечего -- разница в тысячные на длине одной хорды.
void File::arcLines(std::vector<QString>& lines, const Geo::Vertex& fr, const Geo::Vertex& to, const Geo::Arc& arc) {
    switch(App::gcSettings().post().arcs()) {
    case Post::Arcs::IJ: {
        auto [I, J] = arc.center - static_cast<const QPointF&>(fr);
        lines.emplace_back(formated({g(fr), x(to.x()), y(to.y()), z(z_), i(I), j(J), strFeed, strSpindle}));
        break;
    }
    case Post::Arcs::R: {
        // Знак R: отрицательный -- дуга больше полуоборота. Ровно полуоборот
        // и больше -- пополам: у станков R на 180° неустойчив по стороне.
        const double sweep = std::abs(arc.theta);
        if(sweep >= std::numbers::pi - 1e-9) {
            const Geo::Vertex mid{arc.pointAt(0.5), Geo::bulgeOf(arc.theta * 0.5)};
            Geo::Vertex head{fr};
            head.bulge = mid.bulge;
            if(auto a = Geo::arcOf(head, mid, head.bulge)) arcLines(lines, head, mid, *a);
            if(auto a = Geo::arcOf(mid, to, mid.bulge)) arcLines(lines, mid, to, *a);
            break;
        }
        lines.emplace_back(formated({g(fr), x(to.x()), y(to.y()), z(z_), rad(arc.radius), strFeed, strSpindle}));
        break;
    }
    case Post::Arcs::Linear: {
        // Шаг по стрелке прогиба: хорда с отклонением от дуги не больше допуска.
        const double tol = std::min(App::gcSettings().post().arcTolerance(), arc.radius);
        const double step = 2.0 * std::acos(std::max(0.0, 1.0 - tol / arc.radius));
        const int n = std::max(1, static_cast<int>(std::ceil(std::abs(arc.theta) / std::max(step, 1e-6))));
        for(int k = 1; k <= n; ++k) {
            const QPointF pt = k == n ? static_cast<const QPointF&>(to) : arc.pointAt(double(k) / n);
            lines.emplace_back(formated({g1(), x(pt.x()), y(pt.y()), z(z_), strFeed, strSpindle}));
        }
        break;
    }
    }
}

QString File::formatedText(const std::vector<QString>& data) {
    std::vector<Word> words;
    words.reserve(data.size());
    for(const QString& text: data) {
        if(text.isEmpty()) continue;
        const QChar letter = text.front();
        bool isNumber{};
        const double value = QStringView{text}.sliced(1).toDouble(&isNumber);
        // Координата от скрипта -- такое же число на сетке вывода, как и своя:
        // и сравнивается по нему, и печатается из него. Прочие слова (G, F, S,
        // да и всё, что скрипт придумает сам) идут как написаны.
        if(isNumber && COORD_LIST.contains(letter.toUpper()))
            words.emplace_back(letter, toUnits(value));
        else
            words.emplace_back(text);
    }
    return formated(words);
}

QString File::formated(const std::vector<Word>& data) {
    // Диалект строки -- по её слову G: дуги пишутся своим набором флагов.
    // Строка без G наследует последний записанный код.
    FormatKind kind = (gCode_ == G02 || gCode_ == G03) ? Arc : Linear;
    for(const Word& word: data)
        if(!word.text.isEmpty() && (word.text.front() == u'G' || word.text.front() == u'g')) {
            const QStringView num = QStringView{word.text}.sliced(1);
            kind = (num == u"2" || num == u"02" || num == u"3" || num == u"03") ? Arc : Linear;
            break;
        }
    const bool (&flags)[Size] = formatFlags[kind];

    QString ret;
    for(const Word& word: data) {
        if(word.text.isEmpty()) continue;
        const ssize_t index = CMD_LIST.indexOf(word.text.front(), 0, Qt::CaseInsensitive);
        if(index == -1) continue;
        // Слово пишется, если формат требует его всегда либо если оно
        // ИЗМЕНИЛОСЬ. У координатных изменение -- это другое ЧИСЛО: сравнивать
        // их записи значило бы сравнивать округления, а на сетке вывода число
        // и есть окончательное значение.
        const bool same = word.numeric
            ? lastWritten[index] && lastUnits[index] == word.units
            : lastValues[index] == word.text;
        if(flags[AlwaysG + index] || !same) {
            lastUnits[index] = word.units;
            lastWritten[index] = true;
            lastValues[index] = word.text;
            ret += word.text + (flags[SpaceG + index] ? u" " : u"");
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

QString File::format(Units units) {
    const bool negative = units < 0;
    const Units magnitude = negative ? -units : units;

    QString str = QString::number(magnitude / unitsPerMm);
    if(const Units fraction = magnitude % unitsPerMm) {
        QString digits = QString::number(fraction).rightJustified(unitDecimals, u'0');
        while(digits.endsWith(u'0')) digits.chop(1);
        str += u'.' + digits;
    }
    return negative ? u'-' + str : str;
}

QString File::formatValue(double val) {
    QString str = QString::number(val, 'f', 3);
    if(str.contains(u'.')) {
        while(str.endsWith(u'0')) str.chop(1);
        if(str.endsWith(u'.')) str.chop(1);
    }
    return str;
}

namespace {

// Переезд -- отрезок между двумя точками, и только. Вершины путей берутся БЕЗ
// прогиба: у конца прохода он описывает дугу САМОГО прохода (Geo::Vertex::bulge
// -- кривизна сегмента, начинающегося в вершине), и, скопированный в переезд,
// рисует холостой ход дугой. Отсюда QPointF: до полилинии он доезжает уже
// голой точкой.
Geo::Polyline travel(QPointF from, QPointF to) { return Geo::Polyline{
    QPolygonF{from, to}
 }; }

} // namespace

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
        g0path_.push_back(travel(gcp.toolPathss[0].back().back(), gcp.toolPathss[1].front().front()));
        for(size_t i{}; i < gcp.toolPathss[1].size() - 1; ++i)
            g0path_.push_back(travel(gcp.toolPathss[1][i].back(), gcp.toolPathss[1][i + 1].front()));
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
            g1path.push_back(travel(fr.back(), to.front()));
        item = new Gi::GcPath{g1path};
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        item->setPen({Qt::magenta, 0.0});
        itemGroup()->push_back(item);
    }

    g0path_.reserve(gcp.toolPathss.size());
    for(auto&& [fr, to]: v::pairwise(gcp.toolPathss))
        g0path_.push_back(travel(fr.back().back(), to.front().front()));

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
        if(g0path_.size()) g0path_.back().emplace_back(QPointF{paths.front().front()});
        for(auto&& [fr, to]: v::pairwise(paths)) g0path_.push_back(travel(fr.back(), to.front()));
        g0path_.push_back({Geo::Vertex{QPointF{paths.back().back()}}});
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
            g0path_.push_back(travel(paths[j].back(), paths[j + 1].front()));
        if(i < gcp.toolPathss.size() - 1)
            g0path_.push_back(travel(gcp.toolPathss[i].back().back(), gcp.toolPathss[++i].front().front()));
    }

    item = new Gi::GcPath{g0path_};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);
}

} // namespace GCode
