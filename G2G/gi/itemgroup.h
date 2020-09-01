#pragma once

#include "aperturepathitem.h"
#include "componentitem.h"
#include "drillitem.h"
#include "gerberitem.h"
#include "graphicsitem.h"
#include "pathitem.h"

class ItemGroup : public QList<GraphicsItem*> {
public:
    ~ItemGroup();
    void append(GraphicsItem* value);
    void setVisible(bool visible);
    void setSelected(const QVector<int>& ids);
    bool isVisible() { return m_visible; }
    void addToScene();
    QBrush brush() const { return m_brush; }
    QPen pen() const { return m_pen; }
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    void setBrushColor(QColor *col);
    void setPenColor(QColor *col);
    void setZValue(qreal z);

private:
    bool m_visible = true;
    QPen m_pen;
    QBrush m_brush;
};
