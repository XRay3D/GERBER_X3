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
#pragma once

#include "geo/polyline.h"

#include <QFileSystemWatcher>
#include <QJSEngine>
#include <QJSValue>
#include <QObject>
#include <QPointer>
#include <map>
#include <memory>

namespace ShScript {

class ShapeApi;

/// Построитель одной полилинии: sh.begin(x, y) -> lineTo/arcTo/arcToC -> close()/end().
/// Пишет прямо в полилинию внутри ShapeApi, так что незавершённый путь всё
/// равно попадёт в результат (просто останется открытым).
class PathBuilder final : public QObject {
    Q_OBJECT
    QPointer<ShapeApi> api_;
    size_t index_;
    Geo::Polyline* poly() const;

public:
    PathBuilder(ShapeApi* api, size_t index);

    Q_INVOKABLE QJSValue lineTo(double x, double y);
    /// Дуга к точке через прогиб (bulge = tan(θ/4); знак -- сторона: > 0 против часовой).
    Q_INVOKABLE QJSValue arcTo(double x, double y, double bulge);
    /// Дуга к точке по центру и направлению обхода.
    Q_INVOKABLE QJSValue arcToC(double x, double y, double cx, double cy, bool ccw = true);
    Q_INVOKABLE void close();
    Q_INVOKABLE void end();
};

/// Объект `sh`, который получает build(p, sh). Все координаты локальные,
/// центр фигуры -- (0, 0); плагин потом сдвигает результат на ручку центра.
class ShapeApi final : public QObject {
    Q_OBJECT
    friend class PathBuilder;
    QJSEngine* engine_;
    Geo::Polylines polylines_;

    static Geo::Vertex::Dir dirOf(bool ccw) { return ccw ? Geo::Vertex::Ccw : Geo::Vertex::Cw; }

public:
    explicit ShapeApi(QJSEngine* engine);

    Geo::Polylines take() { return std::move(polylines_); }

    Q_INVOKABLE void line(double x1, double y1, double x2, double y2);
    Q_INVOKABLE void circle(double cx, double cy, double diameter);
    /// Прямоугольник по центру и размерам; angleDeg -- поворот вокруг центра.
    Q_INVOKABLE void rectangle(double cx, double cy, double width, double height, double angleDeg = 0);
    /// Дуга по центру, радиусу, начальному углу и размаху (градусы, размах > 0 -- против часовой).
    Q_INVOKABLE void arc(double cx, double cy, double radius, double startDeg, double sweepDeg);
    Q_INVOKABLE void arcBulge(double x1, double y1, double x2, double y2, double bulge);
    Q_INVOKABLE void arcC(double x1, double y1, double x2, double y2, double cx, double cy, bool ccw = true);
    /// pts: массив [x, y] / [x, y, bulge] или {x, y, bulge}.
    Q_INVOKABLE void polyline(const QJSValue& pts, bool closed = false);
    Q_INVOKABLE QJSValue begin(double x, double y);

    // утилиты
    Q_INVOKABLE double bulge(double x1, double y1, double x2, double y2, double cx, double cy, bool ccw = true) const;
    Q_INVOKABLE double deg2rad(double deg) const;
    Q_INVOKABLE double rad2deg(double rad) const;
    Q_INVOKABLE QJSValue polar(double radius, double deg) const;
};

struct ParamDef {
    QString name;
    QString description; // подпись в таблице; пусто -- показывается name
    double value{};
    double min{-1e6};
    double max{+1e6};
    double step{1.0};
    int decimals{3};

    const QString& label() const { return description.isEmpty() ? name : description; }
};

using Params = std::map<QString, double>;

struct Script {
    QString name;
    QString path;
    std::unique_ptr<QJSEngine> engine; // свой движок на скрипт: глобалы не текут между ними
    QJSValue buildFn;
    std::vector<ParamDef> params;
    QString error; // пусто -- скрипт годен

    bool ok() const { return error.isEmpty() && buildFn.isCallable(); }
    const ParamDef* param(const QString& name) const;
    /// Стартовые значения: свои старые -> запомненные -> дефолт скрипта.
    Params merge(const Params& own, const Params& remembered) const;
};

/// Папка scripts/shapes рядом с приложением: список, ленивая загрузка, кэш,
/// слежение за файлами и память последних введённых параметров по скрипту.
class ScriptRegistry final : public QObject {
    Q_OBJECT
    QFileSystemWatcher watcher_;
    std::map<QString, std::unique_ptr<Script>> scripts_;
    std::map<QString, Params> remembered_;
    QString dirPath_;

    void load(Script& script);
    void watchFile(const QString& path);

public:
    explicit ScriptRegistry(QObject* parent = nullptr);

    static QString dirPath();
    /// Распаковывает встроенные примеры, если их ещё нет на диске.
    void ensureDefaultScripts();

    QStringList scripts() const; // имена *.js (без пути)
    Script* get(const QString& name);
    void reload(const QString& name);

    Params lastParams(const QString& scriptName);
    void rememberParams(const QString& scriptName, const Params& params);

    /// Запуск build(); при ошибке возвращает пустой набор и пишет текст в *error.
    Geo::Polylines run(Script& script, const Params& params, QString* error = nullptr);

signals:
    void scriptsChanged();
    void scriptReloaded(const QString& name);
};

} // namespace ShScript
