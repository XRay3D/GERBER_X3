/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

// Заголовки Qt -- первыми: на gcc-16 наши заголовки, попав раньше ядра QtCore,
// сбивают разбор его внутренних (qmath.h, qfunctionaltools_impl.h).
#include <QAnimationGroup>
#include <QGraphicsItem>
#include <QPen>
#include <QPropertyAnimation>
#include <qmath.h>

#include "geo/polygon.h"
#include "geo/util.h"

#include "plugintypes.h"

class AbstractFile;
namespace Shapes {
class AbstractShape;
}

class Project;

namespace Gi {

namespace Type {
enum /*class*/ Type : int {
    DataPath = QGraphicsItem::UserType,
    DataSolid,
    Drill,

    MarkHome = QGraphicsItem::UserType + 100,
    MarkLayoutFrames,
    MarkPin,
    MarkZero,

    Path_ = QGraphicsItem::UserType + 200,
    Bridge,

    Preview = QGraphicsItem::UserType + 300, // Form
    Debug,
    // PrSlot,                                    // DrillForm
    // PrDrill,                                   // DrillForm
    // PrApetrure,                                // DrillForm

    Error = QGraphicsItem::UserType + 400, // Form

    ShapeBegin = QGraphicsItem::UserType + 500,
    ShCircle   = ShapeBegin,
    ShRectangle,
    ShPolyLine,
    ShCirArc,
    ShText,
    ShHandler,
    ShapeEnd
};
}; // namespace Type

class Group;

class Item : public /*QGraphicsObject*/ QGraphicsItem {

    friend class Group;
    friend class ::Project;

    // Q_OBJECT
    Q_GADGET
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible FINAL)
    QColor bodyColor();
    void setBodyColor(const QColor& c);

public:
    // signals:
    void colorChanged();

    explicit Item(AbstractFile* file = nullptr);
    ~Item() override = default;

    bool isEditable() const;
    void setEditable(bool fl);

    QColor color() const;
    void setColor(const QColor& brush);
    void setColorPtr(QColor* brushColor);

    QPen pen() const;
    void setPen(const QPen& pen);
    void setPenColorPtr(const QColor* penColor);

    // Контуры элемента -- НАБОР полилиний, а не тело с дырками: у трассы
    // (DataPath) они открытые, у траектории (GcPath) -- тем более. Тела с
    // дырками живут выше, в самом файле (AbstractFile::mergedCurves).
    virtual Geo::Polylines curves(int param = {}) const;
    virtual void setCurves(Geo::Polylines curves, int param = {});

    // Замкнутая геометрия элемента РЕГИОНОМ. Собрать её из curves() нельзя:
    // в плоском списке вложенность выражена одной лишь ориентацией, и тело,
    // лежащее внутри чужой дырки (репер в кольце, пятак в вырезе полигона),
    // из региона пропадает вместе с самой дыркой. Разомкнутые контуры сюда не
    // попадают -- площади у линии нет.
    virtual Geo::Polygons region() const;
    virtual void redraw();
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    Q_INVOKABLE void setVisible(bool visible);

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(file_); }

    int32_t id() const;
    void setId(int32_t id);
    virtual void changeColor() = 0;

    virtual Side side() const;

protected:
    // QPropertyAnimation animation;
    // QPropertyAnimation visibleAnim;

    mutable QRectF boundingRect_;

    const AbstractFile* file_;
    Group* itemGroup = nullptr;
    QPainterPath shape_;
    // Плоский список -- ровно то, что объявляет curves(). Регион здесь не
    // хранится даже у DataFill: тело с дырками нужно ему на один вызов, чтобы
    // построить shape_ по WindingFill, и живёт оно выше -- в самом файле
    // (AbstractFile::mergedCurves). Вариант из Polylines и Polygons стоил
    // дорого: на нём не собирались ни Drill, ни отладочная отрисовка, ни
    // shape-плагины -- все они правят curves_ как обычный вектор.
    Geo::Polylines curves_;

    QPen pen_;

    const QColor* pnColorPrt_ = nullptr;
    const QColor* colorPtr_   = nullptr;

    QColor color_;
    QColor brushColor_;
    QColor penColor_;

    int32_t id_ = -1;
    double scaleFactor() const;
    enum ColorState {
        Default,
        Hovered  = 1,
        Selected = 2,
    };

    int colorState = Default;
    double scar;
    std::optional<QPainterPath> updateArrows();

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Gi
