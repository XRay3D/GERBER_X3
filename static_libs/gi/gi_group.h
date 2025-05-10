/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"

namespace Gi {

class Group : public mvector<Item*> {
public:
    ~Group();
    void push_back(Item* item);
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

} // namespace Gi
