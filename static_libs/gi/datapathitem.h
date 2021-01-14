/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <graphicsitem.h>
#include <QTimer>
#include <cmath>

namespace Gerber {
class File;
}

class GiDataPath : public GraphicsItem {
    QRectF m_boundingRect;
    int timerId = 0;
#ifdef __GNUC__
    static QTimer timer;
#else
    static inline QTimer timer;
#endif
    static inline int d;
    void redraw() override { update(); }

public:
    GiDataPath(const Path& path, FileInterface* file);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;
    QPainterPath shape() const override;
    void changeColor() override { }

protected:
    QPolygonF m_polygon;
    const Path& m_path;
    mutable QPainterPath m_selectionShape;
    mutable double m_scale = std::numeric_limits<double>::max();
    void updateSelection() const;

    // QGraphicsItem interface
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
};
