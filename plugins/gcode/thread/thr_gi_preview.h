#pragma once

#include "thr_form.h"
#include "thr_model.h"
#include "gi_preview.h"

namespace Threading {

namespace Gi {

class Preview final : public ::Gi::AbstractPreview {
    Path path_;
    Row& row;
    Tool::ID toolId_{};

public:
    explicit Preview(Path&& hv, double diameter, Tool::ID  toolId, Row& row, const Paths& draw_ = {}); // FIXME to Curve

    // AbstractPreview interface
    void updateTool() override;
    Paths paths() const override;
    bool fit(double depth) const override;

    // AbstractDrillPrGI interface
    Tool::ID toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

    Paths offset() const;
    QPointF pos() const { return ~path_.front(); }; // NOTE shadow base class pos func
    Path hv() const { return path_; };              // NOTE shadow base class pos func

protected:
    // Двойной клик по превью переключает использование этого отверстия/паза
    // для построения G-кода (эквивалент чекбокса в шапке таблицы инструментов).
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}

} // namespace Threading::Gi
