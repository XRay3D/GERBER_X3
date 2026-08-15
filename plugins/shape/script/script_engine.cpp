/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "script_engine.h"
#include "geo/util.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJSValueIterator>
#include <QSettings>
#include <QTimer>
#include <numbers>

using namespace Qt::Literals;

namespace ShScript {

//////////////////////////////////////////
/// PathBuilder

PathBuilder::PathBuilder(ShapeApi* api, size_t index)
    : QObject{api}
    , api_{api}
    , index_{index} { }

Geo::Polyline* PathBuilder::poly() const {
    if(!api_ || index_ >= api_->polylines_.size()) return nullptr;
    return &api_->polylines_[index_];
}

QJSValue PathBuilder::lineTo(double x, double y) {
    if(auto* p = poly()) p->emplace_back(QPointF{x, y});
    return api_ ? api_->engine_->newQObject(this) : QJSValue{};
}

QJSValue PathBuilder::arcTo(double x, double y, double bulge) {
    if(auto* p = poly(); p && !p->empty()) {
        p->back().bulge = bulge;
        p->emplace_back(QPointF{x, y});
    }
    return api_ ? api_->engine_->newQObject(this) : QJSValue{};
}

QJSValue PathBuilder::arcToC(double x, double y, double cx, double cy, bool ccw) {
    if(auto* p = poly(); p && !p->empty()) {
        const QPointF a{p->back()}, b{x, y};
        p->back().bulge = Geo::bulgeOf(a, b, {cx, cy}, ShapeApi::dirOf(ccw));
        p->emplace_back(b);
    }
    return api_ ? api_->engine_->newQObject(this) : QJSValue{};
}

void PathBuilder::close() {
    if(auto* p = poly()) {
        // Замыкание -- прямым отрезком: если конец совпал с началом, дубль не нужен.
        if(p->size() > 1 && Geo::distance(p->back(), p->front()) < 1e-9) {
            const double b = p->back().bulge;
            p->pop_back();
            if(!qFuzzyIsNull(b)) p->back().bulge = b; // дуга, ведущая в первую точку
        }
        p->closed = true;
    }
}

void PathBuilder::end() { /* путь и так уже в результате */ }

//////////////////////////////////////////
/// ShapeApi

ShapeApi::ShapeApi(QJSEngine* engine)
    : engine_{engine} { }

void ShapeApi::line(double x1, double y1, double x2, double y2) {
    polylines_.push_back(Geo::Polyline{
        Geo::Vertex{x1, y1},
        Geo::Vertex{x2, y2}
    });
}

void ShapeApi::circle(double cx, double cy, double diameter) {
    if(diameter <= 0) return;
    polylines_.push_back(Geo::circle(diameter, {cx, cy}));
}

void ShapeApi::rectangle(double cx, double cy, double width, double height, double angleDeg) {
    if(width <= 0 || height <= 0) return;
    auto rect = Geo::rectangle(width, height, {cx, cy});
    if(!qFuzzyIsNull(angleDeg)) Geo::rotate(rect, angleDeg, {cx, cy});
    polylines_.push_back(std::move(rect));
}

void ShapeApi::arc(double cx, double cy, double radius, double startDeg, double sweepDeg) {
    if(radius <= 0 || qFuzzyIsNull(sweepDeg)) return;
    polylines_.push_back(Geo::arc({cx, cy}, radius, deg2rad(startDeg), deg2rad(sweepDeg)));
}

void ShapeApi::arcBulge(double x1, double y1, double x2, double y2, double bulge) {
    polylines_.push_back(Geo::Polyline{
        Geo::Vertex{x1, y1, bulge},
        Geo::Vertex{x2, y2}
    });
}

void ShapeApi::arcC(double x1, double y1, double x2, double y2, double cx, double cy, bool ccw) {
    polylines_.push_back(Geo::Polyline{
        Geo::Vertex{QPointF{x1, y1}, this->bulge(x1, y1, x2, y2, cx, cy, ccw)},
        Geo::Vertex{x2, y2}
    });
}

void ShapeApi::polyline(const QJSValue& pts, bool closed) {
    if(!pts.isArray()) return;
    Geo::Polyline poly;
    const int n = pts.property(u"length"_s).toInt();
    poly.reserve(size_t(n));
    for(int i{}; i < n; ++i) {
        const QJSValue v = pts.property(quint32(i));
        double x{}, y{}, b{};
        if(v.isArray()) {
            x = v.property(0).toNumber();
            y = v.property(1).toNumber();
            b = v.property(2).isNumber() ? v.property(2).toNumber() : 0.0;
        } else if(v.isObject()) {
            x             = v.property(u"x"_s).toNumber();
            y             = v.property(u"y"_s).toNumber();
            const auto bv = v.property(u"bulge"_s);
            b             = bv.isNumber() ? bv.toNumber() : 0.0;
        } else
            continue;
        poly.emplace_back(QPointF{x, y}, b);
    }
    if(poly.empty()) return;
    poly.closed = closed;
    polylines_.push_back(std::move(poly));
}

QJSValue ShapeApi::begin(double x, double y) {
    polylines_.emplace_back().emplace_back(QPointF{x, y});
    return engine_->newQObject(new PathBuilder{this, polylines_.size() - 1});
}

double ShapeApi::bulge(double x1, double y1, double x2, double y2, double cx, double cy, bool ccw) const {
    return Geo::bulgeOf({x1, y1}, {x2, y2}, {cx, cy}, dirOf(ccw));
}

double ShapeApi::deg2rad(double deg) const { return deg * std::numbers::pi / 180.0; }
double ShapeApi::rad2deg(double rad) const { return rad * 180.0 / std::numbers::pi; }

QJSValue ShapeApi::polar(double radius, double deg) const {
    const double a = deg2rad(deg);
    auto obj       = engine_->newObject();
    obj.setProperty(u"x"_s, radius * std::cos(a));
    obj.setProperty(u"y"_s, radius * std::sin(a));
    return obj;
}

//////////////////////////////////////////
/// Script

const ParamDef* Script::param(const QString& name) const {
    auto it = std::ranges::find(params, name, &ParamDef::name);
    return it == params.end() ? nullptr : &*it;
}

Params Script::merge(const Params& own, const Params& remembered) const {
    Params out;
    for(const auto& def: params) {
        if(auto it = own.find(def.name); it != own.end()) out.emplace(def.name, it->second);
        else if(auto it = remembered.find(def.name); it != remembered.end()) out.emplace(def.name, it->second);
        else out.emplace(def.name, def.value);
    }
    return out;
}

//////////////////////////////////////////
/// ScriptRegistry

ScriptRegistry::ScriptRegistry(QObject* parent)
    : QObject{parent}
    , dirPath_{dirPath()} {
    QDir{}.mkpath(dirPath_);
    watcher_.addPath(dirPath_);

    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, [this] {
        // Редакторы часто пишут через rename: файл выпадает из watcher'а --
        // возвращаем его и перечитываем.
        for(auto& [name, script]: scripts_)
            if(QFile::exists(script->path) && !watcher_.files().contains(script->path)) {
                watchFile(script->path);
                reload(name);
            }
        emit scriptsChanged();
    });
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        // Небольшая задержка: запись файла редко атомарна.
        QTimer::singleShot(150, this, [this, path] {
            for(auto& [name, script]: scripts_)
                if(script->path == path) {
                    if(QFile::exists(path)) watchFile(path);
                    reload(name);
                    break;
                }
        });
    });
}

QString ScriptRegistry::dirPath() {
    return QCoreApplication::applicationDirPath() + u"/scripts/shapes"_s;
}

void ScriptRegistry::ensureDefaultScripts() {
    QDir{}.mkpath(dirPath_);
    const QDir resDir{u":/shape/scripts"_s};
    for(const QString& fileName: resDir.entryList(QDir::Files)) {
        const QString destPath = dirPath_ + u'/' + fileName;
        if(QFile::exists(destPath)) continue;
        QFile src{u":/shape/scripts/"_s + fileName};
        QFile dst{destPath};
        if(src.open(QIODevice::ReadOnly) && dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
            dst.write(src.readAll());
    }
}

QStringList ScriptRegistry::scripts() const {
    return QDir{dirPath_}.entryList({u"*.js"_s}, QDir::Files | QDir::Readable, QDir::Name);
}

void ScriptRegistry::watchFile(const QString& path) {
    if(!watcher_.files().contains(path)) watcher_.addPath(path);
}

void ScriptRegistry::load(Script& script) {
    script.engine = std::make_unique<QJSEngine>();
    script.engine->installExtensions(QJSEngine::ConsoleExtension);
    script.buildFn = {};
    script.params.clear();
    script.error.clear();

    QFile f{script.path};
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        script.error = u"cannot open "_s + script.path;
        return;
    }
    auto result = script.engine->evaluate(QString::fromUtf8(f.readAll()), script.path);
    if(result.isError()) {
        script.error = u"line "_s + QString::number(result.property(u"lineNumber"_s).toInt()) + u": "_s + result.toString();
        return;
    }
    auto global    = script.engine->globalObject();
    script.buildFn = global.property(u"build"_s);
    if(!script.buildFn.isCallable()) {
        script.error = u"no build(p, sh) function"_s;
        return;
    }
    auto paramsObj = global.property(u"params"_s);
    if(paramsObj.isObject()) {
        QJSValueIterator it{paramsObj};
        while(it.hasNext()) {
            it.next();
            ParamDef def{.name = it.name()};
            const QJSValue v = it.value();
            if(v.isNumber() || v.isBool()) {
                def.value = v.toNumber();
            } else if(v.isObject()) {
                auto num = [&](const char* key, double fallback) {
                    const auto p = v.property(QLatin1StringView{key});
                    return p.isNumber() || p.isBool() ? p.toNumber() : fallback;
                };
                def.value    = num("value", 0.0);
                def.min      = num("min", def.min);
                def.max      = num("max", def.max);
                def.step     = num("step", def.step);
                def.decimals = int(num("decimals", def.decimals));
                if(const auto d = v.property(u"description"_s); d.isString()) def.description = d.toString();
            } else
                continue;
            script.params.push_back(std::move(def));
        }
    }
}

Script* ScriptRegistry::get(const QString& name) {
    if(name.isEmpty()) return nullptr;
    auto it = scripts_.find(name);
    if(it == scripts_.end()) {
        auto script  = std::make_unique<Script>();
        script->name = name;
        script->path = dirPath_ + u'/' + name;
        load(*script);
        watchFile(script->path);
        it = scripts_.emplace(name, std::move(script)).first;
    }
    return it->second.get();
}

void ScriptRegistry::reload(const QString& name) {
    auto it = scripts_.find(name);
    if(it == scripts_.end()) return;
    load(*it->second);
    emit scriptReloaded(name);
}

Params ScriptRegistry::lastParams(const QString& scriptName) {
    if(auto it = remembered_.find(scriptName); it != remembered_.end()) return it->second;
    Params params;
    QSettings settings;
    settings.beginGroup(u"Shapes/Script/"_s + scriptName);
    for(const auto& key: settings.childKeys()) {
        bool ok{};
        const double v = settings.value(key).toDouble(&ok);
        if(ok) params.emplace(key, v);
    }
    settings.endGroup();
    return remembered_.emplace(scriptName, std::move(params)).first->second;
}

void ScriptRegistry::rememberParams(const QString& scriptName, const Params& params) {
    if(scriptName.isEmpty()) return;
    auto& stored = remembered_[scriptName];
    QSettings settings;
    settings.beginGroup(u"Shapes/Script/"_s + scriptName);
    for(const auto& [name, value]: params) {
        stored[name] = value;
        settings.setValue(name, value);
    }
    settings.endGroup();
}

Geo::Polylines ScriptRegistry::run(Script& script, const Params& params, QString* error) {
    if(!script.ok()) {
        if(error) *error = script.error;
        return {};
    }
    auto& engine = *script.engine;
    ShapeApi api{&engine};
    QJSEngine::setObjectOwnership(&api, QJSEngine::CppOwnership);
    auto apiVal = engine.newQObject(&api);
    auto p      = engine.newObject();
    for(const auto& [name, value]: params) p.setProperty(name, value);

    auto result = script.buildFn.call({p, apiVal});
    if(result.isError()) {
        if(error) *error = u"line "_s + QString::number(result.property(u"lineNumber"_s).toInt()) + u": "_s + result.toString();
        return {};
    }
    if(error) error->clear();
    auto polylines = api.take();
    // Пустые/одноточечные пути сцене не нужны.
    std::erase_if(polylines, [](const Geo::Polyline& p) { return p.size() < 2; });
    return polylines;
}

} // namespace ShScript

#include "moc_script_engine.cpp"
