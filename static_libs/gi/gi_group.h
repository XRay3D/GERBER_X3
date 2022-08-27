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

#include "gi.h"

class GiGroup : public mvector<GraphicsItem*> {
public:
    ~GiGroup();
    void push_back(GraphicsItem* item);
    void setVisible(bool visible);
    void setSelected(const mvector<int>& ids);
    bool isVisible() { return visible_; }
    void addToScene(QGraphicsScene* scene = nullptr);
    QColor brushColor() const { return brushColor_; }
    QPen pen() const { return pen_; }
    void setBrushColor(const QColor& color);
    void setPen(const QPen& pen);
    void setBrushColorP(QColor* col);
    void setPenColor(QColor* col);
    void setZValue(double z);
    void setPos(QPointF offset);

private:
    bool visible_ = false;
    QPen pen_;
    QColor brushColor_;
};
