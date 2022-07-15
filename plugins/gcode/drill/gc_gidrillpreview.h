#pragma once

#include "drill_form.h"
#include "drill_model.h"
#include "gi_preview.h"
#include "qpainterpath.h"

class GiDrillPreview final : public GiAbstractPreview {
    PosPath hv;
    Row& row;
    int toolId_ { -1 };

public:
    explicit GiDrillPreview(PosPath&& hv, double diameter, int toolId, Row& row, const Paths& draw_ = {});

    // GiAbstractPreview interface
    void updateTool() override;
    Paths paths() const override;
    bool fit(double depth) const override;

    // AbstractDrillPrGI interface
    int toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

    Paths offset() const;

private:
    static Paths offset(const Path& path, double offset);
};
