// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "shape.h"
#include "ft_view.h"
#include "graphicsview.h"
#include "scene.h"
#include "shhandler.h"
#include "shnode.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

#include "leakdetector.h"

namespace Shapes {

Shape::Shape()
    : m_node(new Node(this, &m_giId))
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
        for (size_t i = 0, e = hPos.size(); i < e; ++i)
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
                hInitPos[shape].push_back(h->pos());
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
        if (m_node->index().isValid()) {
            App::fileTreeView()->selectionModel()->select(m_node->index(),
                (selected ? QItemSelectionModel::Select
                          : QItemSelectionModel::Deselect)
                    | QItemSelectionModel::Rows);
        }
    } else if (change == ItemVisibleChange) {
        emit App::fileModel()->dataChanged(m_node->index(), m_node->index(), { Qt::CheckStateRole });
    }
    return GraphicsItem::itemChange(change, value);
}

void Shape::updateOtherHandlers(Handler*) { }

void Shape::changeColor()
{
    animation.setStartValue(m_bodyColor);

    switch (colorState) {
    case Default:
        m_bodyColor = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        m_bodyColor.setAlpha(50);
        break;
    case Hovered:
        m_bodyColor = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        m_bodyColor.setAlpha(100);
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

Node* Shape::node() const { return m_node; }

bool Shape::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        //case Qt::DisplayRole:
        //    return QString("%1 (%2)").arg(name()).arg(m_giId);
        case Qt::CheckStateRole:
            setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        //case Qt::DecorationRole:
        //    return icon();
        case FileTree::Id:
            m_giId = value.toInt();
            return true;
        case FileTree::Select:
            setSelected(value.toBool());
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

QVariant Shape::data(const QModelIndex& index, int role) const
{
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return QString("%1 (%2)").arg(name()).arg(m_giId);
        case Qt::CheckStateRole:
            return isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return icon();
        case FileTree::Id:
            return m_giId;
        case FileTree::Select:
            return isSelected();
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
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag;
    default:
        return itemFlag;
    }
}

void Shape::menu(QMenu& menu, FileTree::View* /*tv*/) const
{
    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(name()), [this] {
        App::fileModel()->removeRow(m_node->row(), m_node->index().parent());
    });
    auto action = menu.addAction(QIcon::fromTheme("hint"), QObject::tr("&Visible \"%1\"").arg(name()), this, &GraphicsItem::setVisible);
    action->setCheckable(true);
    action->setChecked(isVisible());
    //    action->connect(action, &QAction::toggled );
}

// write to project
void Shape::write_(QDataStream& stream) const
{
    stream << qint32(handlers.size());
    for (const auto& item : handlers) {
        stream << item->pos();
        stream << item->m_hType;
    }
    write(stream);
}

// read from project
void Shape::read_(QDataStream& stream)
{
    isFinal = true;
    qint32 size;
    stream >> size;
    handlers.reserve(size);
    while (size--) {
        QPointF pos;
        int type;
        stream >> pos;
        stream >> type;
        handlers.emplace_back(std::make_unique<Handler>(this, static_cast<Handler::HType>(type)));
        handlers.back()->QGraphicsItem::setPos(pos);
        handlers.back()->setVisible(false);
    }
    read(stream);
    redraw();
}

}
