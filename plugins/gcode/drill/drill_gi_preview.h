#pragma once

#include "drill_form.h"
#include "drill_model.h"
#include "gi_preview.h"

namespace Drilling {

namespace Gi {

class Preview final : public ::Gi::AbstractPreview {
    // Осевая линия: одна вершина у отверстия, две и больше у паза.
    Geo::Polyline path_;
    // Контуры самого тела превью (отверстие как оно есть, паз -- раздутая
    // осевая). Хранятся рядом с sourcePath_ базового класса, потому что тот
    // существует только для рисования: обратного пути из QPainterPath в
    // bulge-вид нет, а paths()/offset() отдают наружу именно контуры.
    Geo::Polylines source_;
    Row& row;
    Tool::ID toolId_{};

public:
    explicit Preview(Geo::Polyline&& hv, double diameter, Tool::ID toolId, Row& row, const Geo::Polylines& draw_ = {});

    // AbstractPreview interface
    void updateTool() override;
    Geo::Polylines paths() const;
    bool fit(double depth) const override;

    // AbstractDrillPrGI interface
    Tool::ID toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

    Geo::Polylines offset() const;
    QPointF pos() const { return path_.front(); }; // NOTE shadow base class pos func
    Geo::Polyline hv() const { return path_; };    // NOTE shadow base class pos func

protected:
    // Двойной клик по превью переключает использование этого отверстия/паза
    // для построения G-кода (эквивалент чекбокса в шапке таблицы инструментов).
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

} // namespace Drilling::Gi
