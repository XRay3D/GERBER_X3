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

#include "abstract_shape.h"
#include "editor.h"
#include "gi.h"
#include "script_engine.h"
#include "shapepluginin.h"

namespace ShScript {

class Plugin;

/// Фигура, геометрию которой строит JS-скрипт из scripts/shapes/*.js.
/// Одна ручка -- центр; скрипт строит в локальных координатах вокруг (0, 0).
class Shape final : public Shapes::AbstractShape {
    friend class Model;
    friend class Editor;

public:
    explicit Shape(Shapes::Plugin* plugin, QPointF center = {});
    ~Shape() override = default;

    // QGraphicsItem interface
    int type() const override { return Gi::Type::ShScript; }

    // AbstractShape interface
    QIcon icon() const override;
    QString name() const override;
    void setPt(const QPointF& pt) override;
    bool addPt(const QPointF&) override { return curHandle = {}, false; } // один клик -- фигура поставлена

    enum PointEnum : int {
        Center,
        PtCount,
    };

    [[nodiscard]] std::any getVal(int id) const override {
        switch(PointEnum{id}) {
        case Center: return static_cast<QPointF>(handles[Center]);
        default    : return {};
        };
    };

    const QString& script() const { return script_; }
    /// Смена скрипта: параметры = запомненные для него -> дефолты скрипта;
    /// перечитывание того же скрипта сохраняет свои значения.
    void setScript(const QString& name);
    const Params& params() const { return params_; }
    void setParam(const QString& name, double value);
    const QString& lastError() const { return lastError_; }
    Script* scriptDef() const;

public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    void deserialize(std::string_view json) override { Serial::loadInto(json, *this); }
    void postLoad(); // параметры/скрипт прочитаны -- слить с актуальным скриптом и построить

protected:
    void rebuild() override;

private:
    Plugin* plug() const;

    [[= Serial::rename("script")]] QString script_;
    [[= Serial::rename("params")]] Params params_;
    [[= Serial::skip]] QString lastError_;
};

class Plugin final : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "description.json")
    Q_INTERFACES(Shapes::Plugin)

    ScriptRegistry registry_; // до editor_: редактор подписывается на реестр
    Editor editor_;

public:
    Plugin();

    // Shapes::Plugin interface
    uint32_t type() const override { return Gi::Type::ShScript; }
    std::string_view typeName() const override { return "Script"; }
    QIcon icon() const override { return QIcon::fromTheme(u"text-x-script"_s); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) override;
    Editor* editor() override { return &editor_; }

    ScriptRegistry& registry() { return registry_; }
};

} // namespace ShScript
