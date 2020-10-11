#pragma once

#include "graphicsitem.h"

class ItemGroup : public QList<GraphicsItem*> {
public:
    ~ItemGroup();
    void append(GraphicsItem* item);
    void setVisible(bool visible);
    void setSelected(const QVector<int>& ids);
    bool isVisible() { return m_visible; }
    void addToScene();
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
