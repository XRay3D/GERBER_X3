#pragma once

#include "myclipper.h"
#include <QGraphicsItem>
#include <QPen>

using namespace ClipperLib;

enum GraphicsItemType {
    GiGerber = QGraphicsItem::UserType,
    GiBridge,
    GiDrill,
    GiPath,
    GiPointHome,
    GiPointZero,
    GiAperturePath,
    GiPin,
    GiLayoutFrames,

    GiThermalPr, // ThermalForm
    GiSlotPr, // DrillForm
    GiDrillPr, // DrillForm
    GiApetrurePr, // DrillForm

    GiError,

    GiShapeC = QGraphicsItem::UserType + 100,
    GiShapeR,
    GiShapeL,
    GiShapeA,
    GiShapeT,
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
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    // clang-format off
    inline QColor bodyColor() { return m_bodyColor; }
    inline void setBodyColor(const QColor& c) { m_bodyColor = c; colorChanged(); }
    inline QColor pathColor() { return m_pathColor; }
    inline void setPathColor(const QColor& c) { m_pathColor = c; colorChanged(); }
    // clang-format on

signals:
    void colorChanged();

public:
    explicit GraphicsItem(AbstractFile* file = nullptr);
    ~GraphicsItem() override = default;

    // clang-format off
    QColor color() const { return m_color; }
    void setColor(const QColor& brush) { m_bodyColor = m_color = brush; colorChanged(); }
    void setColorP(const QColor* brushColor) { m_bodyColor = *(m_colorPtr = brushColor); colorChanged(); }

    QPen pen() const { return m_pen; }
    void setPen(const QPen& pen) { m_pen = pen; colorChanged(); }
    void setPenColor(const QColor* penColor) { m_pnColorPrt = penColor; colorChanged(); }
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
    const AbstractFile* m_file;
    QPainterPath m_shape;

    QPen m_pen;
    QColor m_color;

    const QColor* m_pnColorPrt = nullptr;
    const QColor* m_colorPtr = nullptr;

    QColor m_bodyColor;
    QColor m_pathColor;

    QRectF m_rect;
    int m_id = -1;

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
    };

    int colorState = Default;

    virtual void changeColor() {};

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
