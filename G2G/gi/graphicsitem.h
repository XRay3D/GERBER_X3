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

class GraphicsItem : public QGraphicsItem {
    friend class Gerber::File;
    friend class Excellon::File;
    friend class ItemGroup;
    friend class Project;

public:
    explicit GraphicsItem(AbstractFile* file = nullptr);
    ~GraphicsItem() override = default;

    QBrush brush() const;
    QPen pen() const;
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    virtual Paths paths() const = 0;
    virtual Paths* rPaths() { return nullptr; }
    virtual void redraw() { }

    void setPenColor(const QColor& penColor);
    void setBrushColor(const QColor& brushColor);

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(m_file); }

    int id() const;

protected:
    const AbstractFile* m_file;
    QPen m_pen;
    QBrush m_brush;
    const QColor* m_pnColorPrt = nullptr;
    const QColor* m_brColorPtr = nullptr;
    QPainterPath m_shape;
    QRectF m_rect;
    int m_id = -1;
};
