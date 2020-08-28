// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shape.h"
#include "shandler.h"
#include "shtext.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>
#include <scene.h>

namespace Shapes {
Shape::Shape()
    : GraphicsItem(nullptr)
{
}

Shape::~Shape() { qDeleteAll(sh); }

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

QRectF Shape::boundingRect() const { return m_shape.boundingRect(); }

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
    if (change == GraphicsItemChange::ItemSelectedChange) {
        for (Handler* item : sh)
            item->setVisible(value.toInt());
    }
    return QGraphicsItem::itemChange(change, value);
}

QDataStream& operator<<(QDataStream& stream, const Shape& sh)
{
    stream << sh.type();
    stream << sh.m_id;
    stream << sh.sh.size();
    for (Handler* item : sh.sh) {
        stream << item->pos();
        stream << item->center;
    }
    sh.write(stream);
    return stream;
}
QDataStream& operator>>(QDataStream& stream, Shape& sh)
{
    sh.m_paths.resize(1);
    App::scene()->addItem(&sh);
    stream >> sh.m_id;
    sh.setToolTip(QString::number(sh.m_id));
    int size;
    stream >> size;
    sh.sh.reserve(size);
    while (size--) {
        QPointF pos;
        bool center;
        stream >> pos;
        stream >> center;
        Handler* item = new Handler(&sh, center);
        item->QGraphicsItem::setPos(pos);
        item->setVisible(false);
        sh.sh.append(item);
        App::scene()->addItem(item);
    }
    sh.read(stream);

    sh.setFlags(GraphicsItem::ItemIsSelectable | GraphicsItem::ItemIsFocusable);
    sh.setAcceptHoverEvents(true);
    sh.setZValue(std::numeric_limits<double>::max());
    return stream;
}
}
