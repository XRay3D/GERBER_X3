/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include "gi.h"
#include <QTimer>
#include <cmath>

namespace Gi {

class DataPath : public Item {
    mutable QPainterPath selectionShape_;
    mutable double scale_ = std::numeric_limits<double>::max();
    void updateSelection() const;

    void redraw() override { update(); }
    void changeColor() override { }
    friend class ::MainWindow;

public:
    DataPath(const Path& path, AbstractFile* file);

protected:
    // QGraphicsItem interface
    QPainterPath shape() const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    int type() const override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};

} // namespace Gi
