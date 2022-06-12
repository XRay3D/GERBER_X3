/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gbrcomp_onent.h"
#include "gi.h"

namespace Gerber {

class ComponentItem final : public GraphicsItem {
    const Gerber::Component& m_component;
    QVector<QRectF> pins;
    mutable QPainterPath pathRefDes;
    mutable mvector<QPair<QPainterPath, QPointF>> pathPins;
    mutable double m_scale = std::numeric_limits<double>::max();
    mutable QPointF pt;
    bool m_selected {};

public:
    ComponentItem(const Gerber::Component& component, FileInterface* file);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override { }

    void setSelected(bool selected) { m_selected = selected; }
};

} // namespace Gerber
