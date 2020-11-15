/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "myclipper.h"
#include <QAnimationGroup>
#include <QGraphicsItem>
#include <QPen>
#include <QPropertyAnimation>

using namespace ClipperLib;

enum class GiType {
    Gerber = QGraphicsItem::UserType,
    Bridge,
    Drill,
    Path,
    Home,
    PointZero,
    AperturePath,
    Pin,
    LayoutFrames,

    ThermalPr, // ThermalForm
    SlotPr, // DrillForm
    DrillPr, // DrillForm
    ApetrurePr, // DrillForm

    Error,

    ShapeC = QGraphicsItem::UserType + 100,
    ShapeR,
    ShapeL,
    ShapeA,
    ShapeT,
};

class AbstractFile;

namespace Gerber {
class File;
}
namespace Excellon {
class File;
}

class GraphicsItem : public QGraphicsObject /*QGraphicsItem*/ {
    friend class Gerber::File;
    friend class Excellon::File;
    friend class ItemGroup;
    friend class Project;
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
    explicit GraphicsItem(AbstractFile* file = nullptr);
    ~GraphicsItem() override = default;

    // clang-format off
    QColor color() const { return m_color; }
    void setColor(const QColor& brush) { m_bodyColor = m_color = brush; colorChanged(); }
    void setColorP(QColor* brushColor) { m_bodyColor = *(m_colorPtr = brushColor); colorChanged(); }

    QPen pen() const { return m_pen; }
    void setPen(const QPen& pen) { m_pen = pen; colorChanged(); }
    void setPenColor(QColor* penColor) { m_pnColorPrt = penColor; colorChanged(); }
    // clang-format on

    virtual Paths paths() const = 0;
    virtual Paths* rPaths() { return nullptr; }
    virtual void redraw() { }

    void setVisible(bool visible);

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(m_file); }

    int id() const;

protected:
    QPropertyAnimation animation;

    QRectF m_rect;

    const AbstractFile* m_file;
    QPainterPath m_shape;

    QPen m_pen;

    const QColor* m_pnColorPrt = nullptr;
    const QColor* m_colorPtr = nullptr;

    QColor m_color;
    QColor m_bodyColor;
    QColor m_pathColor;

    int m_id = -1;

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
    };

    int colorState = Default;

    virtual void changeColor() = 0;

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
