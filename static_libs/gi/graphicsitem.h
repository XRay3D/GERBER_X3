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

#include <myclipper.h>
using namespace ClipperLib;

#include <QAnimationGroup>
#include <QGraphicsItem>
#include <QPen>
#include <QPropertyAnimation>

enum class GiType {
    DataPath = QGraphicsItem::UserType,
    DataSolid,
    Drill,

    MarkHome = QGraphicsItem::UserType + 100,
    MarkLayoutFrames,
    MarkPin,
    MarkZero,

    Path = QGraphicsItem::UserType + 200,
    Bridge,

    PrThermal = QGraphicsItem::UserType + 300, // ThermalForm
    PrSlot, // DrillForm
    PrDrill, // DrillForm
    PrApetrure, // DrillForm

    Error = QGraphicsItem::UserType + 400, // ThermalForm

    ShCircle = QGraphicsItem::UserType + 500,
    ShRectangle,
    ShPolyLine,
    ShCirArc,
    ShText,
};

class FileInterface;
class ItemGroup;
class ShapeInterface;

//namespace Dxf {
//class LayerModel;
//}
//namespace Excellon {
//class File;
//}
//namespace Gerber {
//class File;
//}

class GraphicsItem : public QGraphicsObject /*QGraphicsItem*/ {
    //    friend class Excellon::File;
    //    friend class Gerber::File;
    friend class ItemGroup;
    friend class Project;

    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)

    // clang-format off
    inline QColor bodyColor() { return m_bodyColor; }
    inline void setBodyColor(const QColor& c) { m_bodyColor = c; colorChanged(); }

    // clang-format on

signals:
    void colorChanged();

public:
    explicit GraphicsItem(FileInterface* file = nullptr);
    ~GraphicsItem() override = default;

    QColor color() const { return m_color; }
    void setColor(const QColor& brush);
    void setColorPtr(QColor* brushColor);

    QPen pen() const { return m_pen; }
    void setPen(const QPen& pen);
    void setPenColorPtr(const QColor* penColor);

    virtual Paths paths(int alternate = {}) const = 0;
    virtual Paths* rPaths() { return nullptr; }
    virtual void redraw() { }

    void setVisible(bool visible);

    const FileInterface* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(m_file); }

    int id() const;
    void setId(int id);
    virtual void changeColor() = 0;

protected:
    QPropertyAnimation animation;
    QPropertyAnimation visibleA;

    QRectF m_rect;

    const FileInterface* m_file;
    ItemGroup* itemGroup = nullptr;
    QPainterPath m_shape;

    QPen m_pen;

    const QColor* m_pnColorPrt = nullptr;
    const QColor* m_colorPtr = nullptr;

    QColor m_color;
    QColor m_bodyColor;
    QColor m_pathColor;

    int m_giId = -1;
    double scaleFactor() const;
    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
    };

    int colorState = Default;

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
