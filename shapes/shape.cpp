// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shape.h"
#include "graphicsview.h"
#include "scene.h"
#include "shhandler.h"
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

namespace Shapes {
Shape::Shape()
    : GraphicsItem(nullptr)
{
}

Shape::~Shape() { qDeleteAll(handlers); }

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_brColorPtr)
        m_brush.setColor(*m_brColorPtr);

    //QColor color(m_pen.color());
    QPen pen(m_pen);
    pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());

    if (option->state & QStyle::State_Selected)
        pen.setColor(Qt::green);
    if (option->state & QStyle::State_MouseOver)
        pen.setColor(Qt::red);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_shape);
}

QRectF Shape::boundingRect() const
{
    return m_shape.boundingRect();
}

QPainterPath Shape::shape() const
{
    if (!qFuzzyCompare(m_scale, App::graphicsView()->scaleFactor())) {
        m_scale = App::graphicsView()->scaleFactor();
        m_selectionShape = QPainterPath();
        ClipperOffset offset;
        Paths tmpPpath;
        offset.AddPaths(m_paths, jtSquare, etOpenSquare);
        offset.Execute(tmpPpath, 5 * uScale * m_scale);
        for (const Path& path : tmpPpath)
            m_selectionShape.addPolygon(toQPolygon(path));
        //App::scene()->addPath(m_selectionShape, Qt::NoPen, QColor(0, 255, 255, 100));
    }
    return m_selectionShape;
}

Paths Shape::paths() const { return m_paths; }

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        for (Handler* item : handlers)
            item->setVisible(value.toInt());
    }
    return QGraphicsItem::itemChange(change, value);
}

void Shape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mouseMoveEvent(event);
    const auto dp(GlobalSettings::getSnappedPos(event->pos(), event->modifiers()) - initPos);
    for (auto& [shape, hPos] : hInitPos) {
        for (int i = 0, e = hPos.size(); i < e; ++i)
            shape->handlers[i]->QGraphicsItem::setPos(hPos[i] + dp);
        shape->redraw();
    }
}

void Shape::mousePressEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mousePressEvent(event);
    hInitPos.clear();
    const auto p(GlobalSettings::getSnappedPos(event->pos(), event->modifiers()) - event->pos());
    initPos = event->pos() + p;
    for (auto item : scene()->selectedItems()) {
        if (item->type() >= GiShapeC) {
            auto* shape = static_cast<Shape*>(item);
            hInitPos[shape].reserve(shape->handlers.size());
            for (auto h : shape->handlers) {
                hInitPos[shape].append(h->pos());
            }
        }
    }
}
// write to project
QDataStream& operator<<(QDataStream& stream, const Shape& shape)
{
    stream << shape.type();
    stream << shape.m_id;
    stream << shape.handlers.size();
    for (Handler* item : shape.handlers) {
        stream << item->pos();
        stream << item->m_hType;
    }
    shape.write(stream);
    return stream;
}
// read from project
QDataStream& operator>>(QDataStream& stream, Shape& shape)
{
    shape.m_paths.resize(1);
    App::scene()->addItem(&shape);
    stream >> shape.m_id;
    shape.setToolTip(QString::number(shape.m_id));
    int size;
    stream >> size;
    shape.handlers.reserve(size);
    while (size--) {
        QPointF pos;
        int type;
        stream >> pos;
        stream >> type;
        Handler* item = new Handler(&shape, static_cast<Handler::HType>(type));
        item->QGraphicsItem::setPos(pos);
        item->setVisible(false);
        shape.handlers.append(item);
        App::scene()->addItem(item);
    }
    shape.read(stream);

    shape.setFlags(GraphicsItem::ItemIsSelectable | GraphicsItem::ItemIsFocusable);
    shape.setAcceptHoverEvents(true);
    shape.setZValue(std::numeric_limits<double>::max());
    return stream;
}
}
