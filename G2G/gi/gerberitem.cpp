#include "gerberitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <gbrfile.h>
#include <graphicsview.h>
#include <scene.h>

GerberItem::GerberItem(Paths& paths, Gerber::File* file)
    : GraphicsItem(file)
    , m_paths(paths)
{
    for (Path path : m_paths) {
        if (path.size() && path.first() != path.last())
            path.append(path.first());
        m_shape.addPolygon(toQPolygon(path));
    }
    m_rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

GerberItem::~GerberItem()
{
    //    if (dynamic_cast<const G::File*>(m_file)) {
    //        int index = m_file->groupedPaths().indexOf(m_paths);
    //        //qDebug() << "~GerberItem() index" << index;
    //        if (index > -1)
    //            m_file->groupedPaths().remove(index);
    //    }
}

QRectF GerberItem::boundingRect() const { return m_rect; }

QPainterPath GerberItem::shape() const { return m_shape; }

void GerberItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(QPen(Qt::black, 0.0));
        painter->drawPath(m_shape);
        return;
    }
    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    if (Scene::drawPdf()) {
        painter->setBrush(m_brush.color());
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    QColor cb(m_brush.color());
    QBrush b(cb);
    QColor cp(cb);
    QPen pen(Qt::NoPen);

    if (option->state & QStyle::State_Selected) {
        cb.setAlpha(255);
        b.setColor(cb);
        cp.setAlpha(255);
        pen = QPen(cp, 0.0);
    }
    if (option->state & QStyle::State_MouseOver) {
        cb = cb.dark(110);
        b.setColor(cb);
        //        b.setStyle(Qt::Dense4Pattern);
        //        b.setMatrix(matrix().scale(2 * MyGraphicsView:: scaleFactor(), 2 * MyGraphicsView:: scaleFactor()));
        cp.setAlpha(255);
        pen = QPen(cp, 0.0);
    }

    painter->setBrush(b);
    painter->setPen(m_file ? pen : m_pen);
    painter->drawPath(m_shape);
}

int GerberItem::type() const { return GerberItemType; }

Paths GerberItem::paths() const { return m_paths; }
