#pragma once

#include "drill_form.h"
#include "drill_model.h"
#include "gi_preview.h"

namespace Drilling {

namespace Gi {

class Preview final : public ::Gi::AbstractPreview {
    Path path_;
    Row& row;
    int toolId_{-1};

public:
    explicit Preview(Path&& hv, double diameter, int toolId, Row& row, const Paths& draw_ = {});

    // AbstractPreview interface
    void updateTool() override;
    Paths paths() const override;
    bool fit(double depth) const override;

    // AbstractDrillPrGI interface
    int toolId() const override;

    // QGraphicsItem interface
    int type() const override;
    bool isSlot() const;

    Paths offset() const;
    QPointF pos() const { return ~path_.front(); }; // NOTE shadow base class pos func
    Path hv() const { return path_; };              // NOTE shadow base class pos func

    // private:
    //     static Paths offset(const Path& path, double offset);
};
}

} // namespace Drilling::Gi
