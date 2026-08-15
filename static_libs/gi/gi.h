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
    ShCircle = ShapeBegin,
    ShRectangle,
    ShPolyLine,
    ShCirArc,
    ShText,
    ShScript,
    ShapeEnd
};
}; // namespace Type

class Group;

// Всё, что отрисовке нужно знать о кадре. Считается ОДИН раз в Item::paint и
// раздаётся наследникам: раньше каждый paint() сам лез за масштабом через
// scene()->views().front()->transform() и сам разбирал option->state.
struct RenderState {
    double sf;         // 1/масштаб вида, с учётом трансформации самого элемента
    bool hovered;      //
    bool selected;     //
    double dashOffset; // смещение «бегущих муравьёв»
    bool plain() const { return !hovered && !selected; }
};

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
    ~Item() override;

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

    // Точный габарит по ЗАЛИТОМУ контуру. Нужен ровно одному потребителю --
    // GraphicsView::getSelectedBoundingRect. Раньше это была ветка внутри
    // boundingRect(), то есть полный toFillPolygon в самой горячей виртуальной
    // функции сцены.
    QRectF fillBoundingRect() const { return shape_.toFillPolygon(transform()).boundingRect(); }

    // Общий каркас отрисовки. Наследники из static_libs/gi реализуют
    // paintGeometry()/paintHighlight() и не трогают paint().
    //
    // Элементы плагинов (Shapes::AbstractShape, Comp::Item, Gi::Bridge) пока
    // переопределяют paint() напрямую и каркас обходят -- их перевод отложен.
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    const AbstractFile* file() const;
    template <typename T>
    const T* typedFile() const { return dynamic_cast<const T* const>(file_); }

    int32_t id() const;
    void setId(int32_t id);

    // Пересчитать кэш цветов И ПОКАЗАТЬ результат. Невиртуальная обёртка
    // именно ради второй половины: цвет слоя или файла живёт снаружи, элементы
    // смотрят на него по указателю, и правка сама по себе экран не трогает.
    // Пока вид перерисовывал сцену 50 раз в секунду, это сходило с рук --
    // ближайший кадр всё показывал; теперь перерисовку надо просить, и просить
    // её здесь, а не в каждом из вызывающих (дерево слоёв, меню файла,
    // раскраска, наведение, выделение).
    void changeColor() {
        updateColors();
        update();
    }

    virtual Side side() const;

protected:
    // Собственно пересчёт кэша (brushColor_/penColor_) по colorState и
    // указателям на внешний цвет. Звать напрямую нельзя -- только changeColor().
    virtual void updateColors() = 0;

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
    const QColor* colorPtr_ = nullptr;

    QColor color_;
    QColor brushColor_;
    QColor penColor_;

    int32_t id_ = -1;
    double scaleFactor() const;
    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
    };

    int colorState = Default;
    double scar;
    std::optional<QPainterPath> updateArrows();

    // Пересчитать габарит после перестройки shape_ и сбросить зависящие от
    // геометрии кэши. Единственное место в проекте, где нужен
    // prepareGeometryChange(): без него индекс BSP остаётся со старым
    // прямоугольником.
    virtual void geometryChanged();

    // Собственно геометрия элемента. Обычное состояние -- никаких временных
    // QPen/QBrush/QColor: перо и цвета уже готовы в pen_/brushColor_, их
    // обновляет changeColor() по событию, а не покадрово.
    virtual void paintGeometry(QPainter* painter, const RenderState& st);
    // Подсветка наведения и выделения. Рисуется ДО геометрии.
    virtual void paintHighlight(QPainter* painter, const RenderState& st);

    // QGraphicsItem interface
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Gi
