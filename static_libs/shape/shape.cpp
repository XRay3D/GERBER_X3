// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "shape.h"
#include "graphicsview.h"
#include "scene.h"
#include "shhandler.h"
#include "shnode.h"
#include "treeview.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

#include "leakdetector.h"

namespace Shapes {

Shape::Shape()
    : GraphicsItem(nullptr)
    , NodeInterface(m_giId, 1)
{
    m_paths.resize(1);
    changeColor();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setVisible(true);
    //setZValue(std::numeric_limits<double>::max());
}

Shape::~Shape() { }

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*)
{
    m_pathColor = m_bodyColor;
    m_pathColor.setAlpha(255);
    m_pen.setColor(m_pathColor);

    painter->setPen(m_pen);
    painter->setBrush(m_bodyColor);
    painter->drawPath(m_shape);
}

QRectF Shape::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath Shape::shape() const { return m_shape; }

Paths Shape::paths() const { return m_paths; }

void Shape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mouseMoveEvent(event);
    const auto dp(App::settings().getSnappedPos(event->pos(), event->modifiers()) - initPos);
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
    const auto p(App::settings().getSnappedPos(event->pos(), event->modifiers()) - event->pos());
    initPos = event->pos() + p;
    for (auto item : scene()->selectedItems()) {
        if (static_cast<GiType>(item->type()) >= GiType::ShCircle) {
            auto* shape = static_cast<Shape*>(item);
            hInitPos[shape].reserve(shape->handlers.size());
            for (auto& h : shape->handlers) {
                hInitPos[shape].append(h->pos());
            }
        }
    }
}

void Shape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu_;
    menu(menu_, App::fileTreeView());
    menu_.exec(event->screenPos());
}

QVariant Shape::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        const bool selected = value.toInt();
        for (auto& item : handlers)
            item->setVisible(selected);
        if (index().isValid()) {
            App::fileTreeView()->selectionModel()->select(index(), (selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows);
        }
    }
    return GraphicsItem::itemChange(change, value);
}

void Shape::write(QDataStream& /*stream*/) const { }

void Shape::read(QDataStream& /*stream*/) { }

void Shape::updateOtherHandlers(Handler*) { }

// write to project
QDataStream& operator<<(QDataStream& stream, const Shape& shape)
{
    qDebug() << __FUNCTION__ << "Shape F";
    stream << shape.type();
    stream << shape.m_giId;
    stream << shape.isVisible();
    {
        stream << shape.handlers.size();
        for (auto& item : shape.handlers) {
            stream << item->QGraphicsItem::pos();
            stream << item->m_hType;
        }
    }
    shape.write(stream);
    return stream;
}
// read from project
QDataStream& operator>>(QDataStream& stream, Shape& shape)
{
    qDebug() << __FUNCTION__ << "Shape F";
    //    App::scene()->addItem(&shape);
    bool visible;
    stream >> shape.m_giId;
    stream >> visible;
    shape.QGraphicsItem::setVisible(visible);
    shape.setToolTip(QString::number(shape.m_giId));
    {
        int size;
        stream >> size;
        shape.handlers.reserve(size);
        while (size--) {
            QPointF pos;
            int type;
            stream >> pos;
            stream >> type;
            shape.handlers.emplace_back(std::make_unique<Handler>(&shape, static_cast<Handler::HType>(type)));
            shape.handlers.back()->QGraphicsItem::setPos(pos);
            shape.handlers.back()->setVisible(false);
        }
    }
    shape.read(stream);
    shape.redraw();
    return stream;
}

void Shape::changeColor()
{
    animation.setStartValue(m_bodyColor);

    switch (colorState) {
    case Default:
        m_bodyColor = QColor(255, 255, 255, 50);
        break;
    case Hovered:
        m_bodyColor = QColor(255, 255, 255, 100);
        break;
    case Selected:
        m_bodyColor = QColor(255, 0x0, 0x0, 100);
        break;
    case Hovered | Selected:
        m_bodyColor = QColor(255, 0x0, 0x0, 150);
        break;
    }

    animation.setEndValue(m_bodyColor);
    animation.start();
}

bool Shape::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (Column(index.column()) == Column::NameColorVisible && role == Qt::CheckStateRole) {
        setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    }
    return false;
}

QVariant Shape::data(const QModelIndex& index, int role) const
{
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return QString("%1 (%2)")
                .arg(name())
                .arg(m_giId);
        case Qt::CheckStateRole:
            return isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return icon();
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

Qt::ItemFlags Shape::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Column::SideType:
        return itemFlag;
    default:
        return itemFlag;
    }
}

void Shape::menu(QMenu& menu, FileTreeView* /*tv*/) const
{
    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(name()), [this] {
        App::fileModel()->removeRow(row(), index().parent());
    });
}

}
