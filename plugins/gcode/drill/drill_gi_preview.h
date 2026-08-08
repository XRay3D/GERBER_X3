#pragma once

#include "drill_form.h"
#include "drill_model.h"
#include "gi_preview.h"

namespace Drilling {

namespace Gi {

class Preview final : public ::Gi::AbstractPreview {
    Path64 path_;
    Row& row;
    Tool::ID toolId_{};

public:
    explicit Preview(Path64&& hv, double diameter, Tool::ID toolId, Row& row, const Paths64& draw_ = {}); // FIXME to Geo::Polyline

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
    Path64 hv() const { return path_; };            // NOTE shadow base class pos func

protected:
    // Двойной клик по превью переключает использование этого отверстия/паза
    // для построения G-кода (эквивалент чекбокса в шапке таблицы инструментов).
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

} // namespace Drilling::Gi
