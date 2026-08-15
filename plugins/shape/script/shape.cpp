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
#include "shape.h"
#include "geo/util.h"
#include "graphicsview.h"

#include <QFileInfo>
#include <QIcon>

using Shapes::Handle;

namespace ShScript {

Shape::Shape(Shapes::Plugin* plugin, QPointF center)
    : AbstractShape{plugin} {
    if(!std::isnan(center.x())) {
        handles = {
            Handle{center, Handle::Center}
        };
        curHandle = handles.data() + Center;
        redraw();
    }
    App::grView().addItem(this);
}

Plugin* Shape::plug() const { return static_cast<Plugin*>(plugin); }

Script* Shape::scriptDef() const { return plug()->registry().get(script_); }

void Shape::rebuild() {
    if(handles.size() < PtCount) return;
    curves_.clear();
    shape_ = {};
    lastError_.clear();

    const QPointF center = handles[Center];
    if(auto* script = scriptDef()) {
        curves_ = plug()->registry().run(*script, params_, &lastError_);
        Geo::translate(curves_, center);
    } else if(!script_.isEmpty()) {
        lastError_ = u"script not found: "_s + script_;
    }

    if(curves_.empty()) {
        // Крестик, чтобы пустую (сломанную) фигуру можно было выделить и удалить.
        constexpr double s = 1.0;
        shape_.moveTo(center - QPointF{s, 0});
        shape_.lineTo(center + QPointF{s, 0});
        shape_.moveTo(center - QPointF{0, s});
        shape_.lineTo(center + QPointF{0, s});
    } else
        shape_ = Geo::toPath(curves_);
    closed = std::ranges::any_of(curves_, &Geo::Polyline::closed);
}

QString Shape::name() const {
    // В дереве -- по имени файла скрипта (без .js), пока скрипт не выбран -- «Script».
    if(script_.isEmpty()) return QObject::tr("Script");
    return QFileInfo{script_}.completeBaseName();
}

QIcon Shape::icon() const { return QIcon::fromTheme(u"text-x-script"_s); }

void Shape::setPt(const QPointF& pt) {
    if(curHandle) *curHandle = pt;
    redraw();
}

void Shape::setScript(const QString& name) {
    // Тот же скрипт (перечитывание) -- свои значения сохраняются; другой --
    // берутся запомненные для него, затем дефолты.
    const bool same = script_ == name;
    script_ = name;
    auto& registry = plug()->registry();
    if(auto* script = registry.get(name))
        params_ = script->merge(same ? params_ : Params{}, registry.lastParams(name));
    curHandle = {};
    redraw();
    // Имя в дереве и подсказка зависят от скрипта.
    setToolTip(this->name() + QString::number(id()));
    if(row() >= 0 && App::fileModelPtr())
        emit App::fileModel().dataChanged(index(), index());
}

void Shape::setParam(const QString& name, double value) {
    auto it = params_.find(name);
    if(it == params_.end() || qFuzzyCompare(it->second, value)) return;
    it->second = value;
    plug()->registry().rememberParams(script_, params_);
    curHandle = {};
    redraw();
}

void Shape::postLoad() {
    if(auto* script = scriptDef())
        params_ = script->merge(params_, {}); // сохранённые в проекте важнее запомненных
    curHandle = {};
    AbstractShape::postLoad();
}

//////////////////////////////////////////
/// Plugin

Plugin::Plugin()
    : editor_{this, registry_} {
    registry_.ensureDefaultScripts();
    editor_.refreshScripts();
}

Shapes::AbstractShape* Plugin::createShape(const QPointF& point) {
    auto shape = new Shape{this, point};
    if(!std::isnan(point.x())) shape->setScript(editor_.currentScript());
    editor_.add(shape);
    return shape;
}

} // namespace ShScript

#include "moc_shape.cpp"
