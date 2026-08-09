#pragma once

#include "gi_preview.h"
#include "thr_form.h"
#include "thr_model.h"

namespace Threading {

namespace Gi {

class Preview final : public ::Gi::AbstractPreview {
    Geo::Polyline path_;
    Row& row;
    Tool::ID toolId_{};

public:
    explicit Preview(Geo::Polyline&& hv, double diameter, Tool::ID toolId, Row& row, const Paths64& draw_ = {}); // FIXME to Curve

    // AbstractPreview interface
    void updateTool() override;
    Paths64 paths() const;
    bool fit(double depth) const override;

    // AbstractDrillPrGI interface
    Tool::ID toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

    Paths64 offset() const;
    QPointF pos() const { return ~path_.front(); }; // NOTE shadow base class pos func
    Geo::Polyline hv() const { return path_; };            // NOTE shadow base class pos func

protected:
    // Двойной клик по превью переключает использование этого отверстия/паза
    // для построения G-кода (эквивалент чекбокса в шапке таблицы инструментов).
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

} // namespace Threading::Gi
