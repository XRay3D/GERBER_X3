#pragma once

#include "gc_drillform.h"
#include "gc_drillmodel.h"
#include "gi_preview.h"
#include "qpainterpath.h"

class GiDrillPreview final : public GiAbstractPreview {
    HV hv;
    Row& row;
    int toolId_ { -1 };

public:
    explicit GiDrillPreview(HV&& hv, double diameter, int toolId, Row& row);

    // GiAbstractPreview interface
    void updateTool() override;
    IntPoint pos() const override;
    Paths paths() const override;
    bool fit(double depth) override;

    // AbstractDrillPrGI interface
    int toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

private:
    static Paths offset(const Path& path, double offset);
};
