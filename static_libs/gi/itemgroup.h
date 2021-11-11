/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
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

class ItemGroup : public mvector<GraphicsItem*> {
public:
    ~ItemGroup();
    void push_back(GraphicsItem* item);
    void setVisible(bool visible);
    void setSelected(const mvector<int>& ids);
    bool isVisible() { return m_visible; }
    void addToScene(QGraphicsScene* scene = nullptr);
    QColor brushColor() const { return m_brushColor; }
    QPen pen() const { return m_pen; }
    void setBrushColor(const QColor& color);
    void setPen(const QPen& pen);
    void setBrushColorP(QColor* col);
    void setPenColor(QColor* col);
    void setZValue(qreal z);

private:
    bool m_visible = false;
    QPen m_pen;
    QColor m_brushColor;
};
