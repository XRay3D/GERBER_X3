/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gbrcomp_onent.h"
#include "gi.h"

namespace Gerber::Comp {

class Item final : public Gi::Item {
    const Component& component_;
    QVector<QRectF> pins;
    mutable QPainterPath pathRefDes;
    mutable std::vector<QPair<QPainterPath, QPointF>> pathPins;
    mutable double scale_ = std::numeric_limits<double>::max();
    mutable QPointF pt;
    bool selected_{};

public:
    Item(const Component& component, AbstractFile* file);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // Gi::Item interface
    Geo::Polylines curves(int alternate = {}) const override;
    void updateColors() override { }

    void setSelected(bool selected) { selected_ = selected; }
};

} // namespace Gerber::Comp
