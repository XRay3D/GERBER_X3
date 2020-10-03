#pragma once

#include "gbrtypes.h"
#include "gi/graphicsitem.h"

#include <QVector>

class Tool;

class ThermalNode;

class ThermalPreviewItem : public QObject, public QGraphicsItem {
    Q_OBJECT

    Q_PROPERTY(QColor bColor READ bColor WRITE setBColor NOTIFY colorChanged)
    Q_PROPERTY(QColor pColor READ pColor WRITE setPColor NOTIFY colorChanged)

    QColor bColor() { return mbColor; }
    void setBColor(const QColor& c)
    {
        mbColor = c;
        colorChanged();
    }
    QColor pColor() { return mpColor; }
    void setPColor(const QColor& c)
    {
        mpColor = c;
        colorChanged();
    }

    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
    friend class ThermalNode;

signals:
    void colorChanged();
    void selectionChanged(const QModelIndex& s, const QModelIndex& d);

public:
    explicit ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth);

    ~ThermalPreviewItem() override;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    //////////////////////////////////////////
    int type() const override;
    IntPoint pos() const;
    Paths paths() const;
    Paths bridge() const;
    void redraw();

    double angle() const;
    void setAngle(double angle);

    double tickness() const;
    void setTickness(double tickness);

    int count() const;
    void setCount(int count);

    //    ThermalNode* node() const;

    bool isValid() const;

private:
    Tool& tool;
    double& m_depth;
    bool m_isValid = false;

    const Gerber::GraphicObject* const grob = nullptr;

    const QPainterPath m_sourcePath;

    QPainterPath m_toolPath;

    QColor mbColor;
    QColor mpColor;
    bool hover = false;

    Paths m_bridge;

    double m_angle = 0.0;
    double m_tickness = 0.5;
    int m_count = 4;
    ThermalNode* m_node = nullptr;

    inline static QVector<ThermalPreviewItem*> hhh;

    // QGraphicsItem interface
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
