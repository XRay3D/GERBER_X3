// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "shape.h"
#include "ft_view.h"
#include "qgraphicsscene.h"
#include "shhandler.h"

#include "mainwindow.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Shapes {

AbstractShape::AbstractShape()
    : FileTree::Node(FileTree::AbstractShape)
//    : node_(new Node(this))
{
    paths_.resize(1);
    changeColor();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setVisible(true);
    // setZValue(std::numeric_limits<double>::max());
}

AbstractShape::~AbstractShape() {
    App::project().deleteShape(id_);
}

void AbstractShape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
    if(App::drawPdf()) [[unlikely]] {
        pathColor_ = Qt::black;
        pathColor_.setAlpha(255);
        pen_.setColor(pathColor_);
    } else [[likely]] {
        pathColor_ = bodyColor_;
        pathColor_.setAlpha(255);
        pen_.setColor(pathColor_);
    }
    painter->setPen(pen_);
    painter->setBrush(QGraphicsItem::flags().testFlag(ItemIsSelectable) ? QBrush{bodyColor_} : QBrush{Qt::NoBrush});
    painter->drawPath(shape_);
}

QRectF AbstractShape::boundingRect() const { return shape_.boundingRect(); }

QPainterPath AbstractShape::shape() const { return shape_; }

Paths AbstractShape::paths(int) const { return paths_; }

void AbstractShape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mouseMoveEvent(event);
    const auto dp(App::settings().getSnappedPos(event->pos(), event->modifiers()) - initPos);
    for(auto& [shape, hPos]: hInitPos) {
        for(size_t i = 0, e = hPos.size(); i < e; ++i)
            shape->handlers[i]->setPos(hPos[i] + dp);
        shape->redraw();
    }
}

void AbstractShape::mousePressEvent(QGraphicsSceneMouseEvent* event) { // групповое перемещение
    QGraphicsItem::mousePressEvent(event);
    if(currentHandler)
        return;
    currentHandler = {};
    hInitPos.clear();
    const auto p(App::settings().getSnappedPos(event->pos(), event->modifiers()) - event->pos());
    initPos = event->pos() + p;
    for(auto item: scene()->selectedItems()) {
        if(item->type() >= Gi::Type::ShCircle) {
            auto* shape = static_cast<AbstractShape*>(item);
            hInitPos[shape].reserve(shape->handlers.size());
            for(auto&& h: shape->handlers) {
                h->setFlag(ItemSendsScenePositionChanges, false);
                hInitPos[shape].emplace_back(h->pos());
            }
        }
    }
}

void AbstractShape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    App::mainWindow().setDockWidget(App::shapePlugin(type())->editor());
}

void AbstractShape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu_;
    menu(menu_, App::fileTreeViewPtr());
    menu_.exec(event->screenPos());
}

QVariant AbstractShape::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemSelectedChange) {
        const bool selected = value.toInt();
        for(auto& item: handlers)
            item->setVisible(selected);
        if(/*node_->*/ index().isValid())
            App::fileTreeView().selectionModel()->select(/*node_->*/ index(),
                (selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect)
                    | QItemSelectionModel::Rows);
    } else if(change == ItemVisibleChange) {
        emit App::fileModel().dataChanged(/*node_->*/ index(), /*node_->*/ index(), {Qt::CheckStateRole});
    }
    return Gi::Item::itemChange(change, value);
}

void AbstractShape::updateOtherHandlers(Handle* h, int mode) { currentHandler = h, redraw(), currentHandler = nullptr; }

void AbstractShape::changeColor() {
    //    animation.setStartValue(bodyColor_);

    switch(colorState) {
    case Default:
        bodyColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        bodyColor_.setAlpha(50);
        break;
    case Hovered:
        bodyColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        bodyColor_.setAlpha(100);
        break;
    case Selected:
        bodyColor_ = QColor(255, 0x0, 0x0, 100);
        break;
    case Hovered | Selected:
        bodyColor_ = QColor(255, 0x0, 0x0, 150);
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}

// Node* AbstractShape::node() const { return node_; }

bool AbstractShape::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        // case Qt::DisplayRole:
        //     return QString("%1 (%2)").arg(name()).arg(giId_);
        case Qt::CheckStateRole:
            setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        // case Qt::DecorationRole:
        //     return icon();
        case FileTree::Id:
            id_ = value.toInt();
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

QVariant AbstractShape::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole:
            return QString("%1 (%2)").arg(name()).arg(id_);
        case Qt::CheckStateRole:
            return isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return icon();
        case FileTree::Id:
            return id_;
        case FileTree::Select:
            return isSelected();
        default:
            return {};
        }
    default:
        return {};
    }
}

Qt::ItemFlags AbstractShape::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag;
    default:
        return itemFlag;
    }
}

void AbstractShape::menu(QMenu& menu, FileTree::View* /*tv*/) {
    auto action = menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(name()), [this] {
        auto r = row();
        auto p = index().parent();
        App::fileModel().removeRow(r, p);
    });

    action = menu.addAction(QIcon::fromTheme("hint"), QObject::tr("&Visible \"%1\"").arg(name()), [this](bool fl) { Gi::Item::setVisible(fl); });
    action->setCheckable(true);
    action->setChecked(isVisible());

    action = menu.addAction(QIcon::fromTheme(""), QObject::tr("&Selectable \"%1\"").arg(name()), [item = const_cast<AbstractShape*>(this)](bool fl) {
        item->setFlag(ItemIsSelectable, fl);
    });
    action->setCheckable(true);
    action->setChecked(Gi::Item::flags() & ItemIsSelectable);

    action = menu.addAction(QIcon::fromTheme("document-edit"), QObject::tr("Edit Selected"), [this] {
        App::mainWindow().setDockWidget(App::shapePlugin(type())->editor());
    });

    //    action->connect(action, &QAction::toggled );
}

// write to project
void AbstractShape::write(QDataStream& stream) const {
    stream << bool(Gi::Item::flags() & ItemIsSelectable);
    stream << qint32(handlers.size());
    for(const auto& item: handlers) {
        stream << item->pos();
        stream << item->type_;
    }
}

// read from project
void AbstractShape::read(QDataStream& stream) {
    stream >> isFinal;
    setFlag(ItemIsSelectable, isFinal);

    isFinal = true;

    qint32 size;
    stream >> size;
    handlers.reserve(size);
    QPointF pos;
    Handle::Type type;
    for(int i{}; i < size; ++i) {
        stream >> pos;
        stream >> type;
        if(handlers.size() < size)
            handlers.emplace_back(std::make_unique<Handle>(this, type));
        handlers[i]->QGraphicsItem::setPos(pos);
        handlers[i]->setVisible(false);
    }
    redraw();
}

} // namespace Shapes
