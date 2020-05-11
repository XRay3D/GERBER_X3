#include "itemgroup.h"

#include <scene.h>

ItemGroup::~ItemGroup()
{
    //qDebug("~ItemGroup()");
    if (App::scene()->items().size())
        qDeleteAll(*this);
}

void ItemGroup::append(GraphicsItem* value)
{
    value->m_id = QList::size() ? QList::last()->m_id + 1 : 0;
    value->setToolTip((value->toolTip().isEmpty() ? QString() : value->toolTip() + '\n')
        + QString("ID(%1): %2").arg(value->type()).arg(value->m_id));
    QList::append(value);
}

void ItemGroup::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (QGraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void ItemGroup::setSelected(const QVector<int>& ids)
{
    for (GraphicsItem* item : *this)
        item->setSelected(ids.contains(item->id()));
}

void ItemGroup::addToScene()
{
    for (QGraphicsItem* item : *this)
        App::scene()->addItem(item);
}

void ItemGroup::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        for (GraphicsItem* item : *this)
            item->setBrush(m_brush);
    }
}

void ItemGroup::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        for (GraphicsItem* item : *this)
            item->setPen(m_pen);
    }
}

void ItemGroup::setZValue(qreal z)
{
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}
