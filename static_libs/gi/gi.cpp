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

#include "gi.h"
#include "abstract_file.h"
#include "app.h"
// #include "gi_group.h"
#include "graphicsview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <qbrush.h>
#include <qpen.h>

namespace Gi {

QColor Item::bodyColor() { return brushColor_; }

void Item::setBodyColor(const QColor& c) { brushColor_ = c, colorChanged(); }

void Item::colorChanged() { update(); }

Item::Item(AbstractFile* file)
    : file_{file}
    , pen_{Qt::white, 0.0}
    , colorPtr_{file ? &file->color() : nullptr}
    , color_{Qt::white}
    , brushColor_{colorPtr_ ? *colorPtr_ : color_}
    , penColor_{Qt::transparent} {
    // animation(this, u"bodyColor"_s)
    // , visibleAnim(this, u"opacity"_s)     ,// animation.setDuration(100);
    // animation.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    // connect(this, &Item::colorChanged, [this] { update(); });
    // visibleAnim.setDuration(100);
    // visibleAnim.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    // connect(&visibleAnim, &QAbstractAnimation::finished, [this] {
    // QGraphicsObject::setVisible(visibleAnim.currentValue().toDouble() >
    // 0.9); });
    QGraphicsItem::setVisible(false);
    // connect(this, &QGraphicsObject::rotationChanged, [] {
    // qDebug(u"rotationChanged"_s); });
}

Item::~Item() {
    // Реестр анимации держит сырые указатели -- выписываемся.
    if(colorState & Selected)
        if(auto* view = App::grViewPtr()) view->removeAnimated(this);
}

bool Item::isEditable() const { return QGraphicsItem::flags() & ItemIsMovable; }

void Item::setEditable(bool fl) {
    setZValue(id_ + (fl * std::numeric_limits<double>::max() * 0.5));
    if(fl) setSelected(true);
    setFlag(ItemIsMovable, fl);
}

QColor Item::color() const { return color_; }

void Item::setColor(const QColor& color) {
    brushColor_ = color_ = color;
    colorChanged();
}

void Item::setColorPtr(QColor* colorPtr) {
    if(colorPtr) brushColor_ = *(colorPtr_ = colorPtr);
    penColor_ = colorPtr_ ? *colorPtr_ : color_;
    colorChanged();
}

QPen Item::pen() const { return pen_; }

void Item::setPen(const QPen& pen) {
    pen_ = pen;
    colorChanged();
}

void Item::setPenColorPtr(const QColor* penColor) {
    if(penColor) pnColorPrt_ = penColor;
    colorChanged();
}

Geo::Polylines Item::curves(int param) const {
    Geo::Polylines curves{curves_};
    if(param) return curves; // сырые, в системе координат самого элемента
    return Geo::transform(curves, transform());
}

Geo::Polygons Item::region() const {
    // У элемента, который хранит только контуры, вложенность и правда выражена
    // их ориентацией -- других сведений о ней просто нет. Тем, у кого регион
    // есть (DataFill), метод переопределён и отдаёт его точно.
    return Geo::Polygons{curves()};
}

void Item::setCurves(Geo::Polylines curves, int /*param*/) {
    bool ok{};
    auto tr = transform().inverted(&ok);
    if(ok)
        // Хранятся контуры в своей системе координат, а приходят в сценовой --
        // отсюда обратное преобразование, парное тому, что делает curves().
        Geo::transform(curves, tr);
    else
        qCritical("transform().inverted(&ok); ok is false!!!");

    shape_ = Geo::toPath(curves);
    curves_ = std::move(curves);
    geometryChanged();

    redraw();
}

void Item::geometryChanged() {
    prepareGeometryChange();
    boundingRect_ = shape_.boundingRect();
}

// void Item::setPaths(Paths paths, int /*param*/) {
//     auto t{transform()};
//     auto a{qRadiansToDegrees(asin(t.m12()))};
//     t = t.rotateRadians(-t.m12());
//     auto x{t.dx()};
//     auto y{t.dy()};
//     shape_ = {};
//     t = {};
//     t.translate(-x, -y);
//     t.rotate(-a);
//     for(auto&& path: ~paths)
//         shape_.addPolygon(t.map(path));
//     redraw();
// }

void Item::redraw() { }

QRectF Item::boundingRect() const { return boundingRect_; }

QPainterPath Item::shape() const { return shape_; }

void Item::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) {
    // Цвета живут в файле и меняются извне (дерево слоёв), поэтому указатели
    // разыменовываем каждый кадр -- это два разыменования, а не аллокация.
    if(pnColorPrt_) pen_.setColor(*pnColorPrt_);
    if(colorPtr_) color_ = *colorPtr_;

    // levelOfDetailFromTransform -- статический хелпер, флаг
    // ItemUsesExtendedStyleOption для него не нужен. Мировая трансформация уже
    // включает трансформацию самого элемента, то есть множитель
    // file_->transform().scale, который scaleFactor() считает отдельно.
    const RenderState st{
        .sf = 1.0 / QStyleOptionGraphicsItem::levelOfDetailFromTransform(painter->worldTransform()),
        .hovered = bool(option->state & QStyle::State_MouseOver),
        .selected = bool(option->state & QStyle::State_Selected),
        .dashOffset = App::dashOffset(),
    };

    if(!st.plain()) [[unlikely]]
        paintHighlight(painter, st);
    paintGeometry(painter, st);
}

void Item::paintGeometry(QPainter* painter, const RenderState&) {
    painter->setBrush(Qt::NoBrush);
    painter->setPen(pen_);
    painter->drawPath(shape_);
}

void Item::paintHighlight(QPainter*, const RenderState&) { }

void Item::setVisible(bool visible) {
    // Без setOpacity: он гнал лишний itemChange и update() на КАЖДЫЙ элемент
    // (десятки тысяч на переключение слоя), а непрозрачность нигде не читается.
    QGraphicsItem /*QGraphicsObject*/ ::setVisible(visible);
}

const AbstractFile* Item::file() const { return file_; }

int Item::id() const { return id_; }

void Item::setId(int32_t id) { id_ = id; }

Side Item::side() const { return file_ ? file_->side() : Side::Top; }

double Item::scaleFactor() const {
    // Раньше здесь был обход scene() -> views() -> front() -> transform() на
    // КАЖДЫЙ вызов. Вид публикует масштаб сам, при смене трансформации.
    double scale = App::viewScaleFactor();
    if(file_) scale /= std::min(file_->transform().scale.x(), file_->transform().scale.y());
    return scale;
}

std::optional<QPainterPath> Item::updateArrows() {
    if(auto sf = scaleFactor(); scar == sf)
        return {};
    else
        scar = sf;

    QPainterPath arrows;

    using QPP = QPainterPath;
    using El = QPP::Element;

    const double length = std::clamp(30 * scar, 0.0, 0.5);

    for(auto&& elements:
        v::iota(0, shape_.elementCount())
            // Лямбда, а не std::bind: bind КОПИРОВАЛ shape_ в свой объект.
            | v::transform([this](int i) { return shape_.elementAt(i); })   // to Element
            | v::chunk_by([](const El&, const El& r) { return r.type; })) { // to Subpath Polygons

        constexpr auto splitCurve = +[](const El&, const El& r) {
            return r.type > QPP::CurveToElement;
        };

        for(auto&& [from, to]: v::pairwise(v::chunk_by(elements, splitCurve))) {
            QLineF line{to.back(), to.front().type == QPP::CurveToElement ? to[1] : from.back()};
            if(line.length() < length) continue;
            const double angle = line.angle();
            if(length > 0.) {
                line.setLength(length);
                line.setAngle(angle + 10);
                arrows.moveTo(line.p2());
                line.setAngle(angle - 10);
                arrows.lineTo(line.p1());
                arrows.lineTo(line.p2());
            }
        }
    }
    return arrows;
}

void Item::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

// double Item::penWidth(double w) const {
// if(scene() && scene()->views().size())
// w /= scene()->views().front()->transform().m11();
// return w;
// };

void Item::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant Item::itemChange(QGraphicsItem::GraphicsItemChange change,
    const QVariant& value) {
    switch(change) {
    case ItemSelectedChange: {
        const bool fl = value.toInt();
        fl ? colorState |= Selected : colorState &= ~Selected;
        changeColor();
        // Регистрация в реестре анимации вида: таймер «бегущих муравьёв»
        // крутится, только пока есть что анимировать.
        if(auto* view = App::grViewPtr())
            fl ? view->addAnimated(this) : view->removeAnimated(this);
    } break;
    case ItemSceneChange:
        // Уходим со сцены -- выписываемся, иначе реестр останется с висячим
        // указателем.
        if(!value.value<QGraphicsScene*>())
            if(auto* view = App::grViewPtr()) view->removeAnimated(this);
        break;
    default: break;
    }
    return QGraphicsItem::itemChange(change, value);
}

} // namespace Gi

#include "moc_gi.cpp"
