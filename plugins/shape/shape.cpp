/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "shape.h"
#include "ft_view.h"
#include "qgraphicsscene.h"

// #include "mainwindow.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Shapes {

static constexpr int HandleSize = 20;

QDataStream& operator<<(QDataStream& stream, const Handle& handle) {
    return stream << handle.pos_ << handle.type_;
}

QDataStream& operator>>(QDataStream& stream, Handle& handle) {
    return stream >> handle.pos_ >> handle.type_;
}

QDataStream& operator<<(QDataStream& stream, const AbstractShape& shape) {
    QByteArray data;
    QDataStream out{&data, QIODevice::WriteOnly};
    Block{out}.write(shape.id_, shape.isVisible());
    out << shape.isEditable();
    out << shape.handles;
    shape.write(out);
    return stream << data;
}

QDataStream& operator>>(QDataStream& stream, AbstractShape& shape) {
    QByteArray data;
    stream >> data;
    QDataStream in{&data, QIODevice::ReadOnly};
    bool bFlag;
    Block{in}.read(shape.id_, bFlag);

    shape.setZValue(shape.id_);
    shape.setVisible(bFlag);
    shape.setToolTip(shape.name() % QString::number(shape.id_));

    in >> bFlag;
    shape.setEditable(bFlag);
    shape.isFinal = true;

    in >> shape.handles;
    assert(shape.handles.size());

    shape.readAndInit(in);

    return stream;
}

AbstractShape::AbstractShape()
    : FileTree::Node{FileTree::AbstractShape} {
    paths_.resize(1);
    changeColor();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setVisible(true);
    // setZValue(std::numeric_limits<double>::max());
}

AbstractShape::~AbstractShape() {
    if(App::projectPtr()) App::project().deleteShape(id_);
    // std::erase(model->shapes, this);
    // qobject_cast<QTableView*>(model->parent())->reset();
}

void AbstractShape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
    if(App::drawPdf()) [[unlikely]] {
        penColor_ = Qt::black;
        penColor_.setAlpha(255);
        pen_.setColor(penColor_);
    } else [[likely]] {
        penColor_ = brushColor_;
        penColor_.setAlpha(255);
        pen_.setColor(penColor_);
    }
    painter->setPen(pen_);
    painter->setBrush(QGraphicsItem::flags().testFlag(ItemIsSelectable) ? QBrush{brushColor_} : QBrush{Qt::NoBrush});
    painter->drawPath(shape_);

    if(!isSelected() || !isEditable()) return;

    painter->setPen({Qt::red, 1.0 * scale()});
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(pPathHandle.boundingRect());

    painter->setPen(Qt::NoPen);
    const auto hs = HandleSize * 0.5 * scale();

    painter->setPen(QPen(Qt::black, 0.0));

    for(auto&& var: handles) {
        auto color = var.color();
        if(&var == curHandle.base()) color = Qt::magenta;
        color.setAlpha(100);
        painter->setBrush(color);
        painter->drawEllipse(var, hs, hs);
    }
}

QRectF AbstractShape::boundingRect() const {
    if(isSelected()) return pPathHandle.boundingRect();
    return shape_.boundingRect();
}

QPainterPath AbstractShape::shape() const {
    if(isSelected()) return pPathHandle + shape_;
    return shape_;
}

Paths AbstractShape::paths(int) const { return paths_; }

void AbstractShape::changeColor() {
    //    animation.setStartValue(bodyColor_);

    switch(colorState) {
    case Default:
        brushColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        brushColor_.setAlpha(50);
        break;
    case Hovered:
        brushColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF;
        brushColor_.setAlpha(100);
        break;
    case Selected:
        brushColor_ = QColor(255, 0x0, 0x0, 100);
        break;
    case Hovered | Selected:
        brushColor_ = QColor(255, 0x0, 0x0, 150);
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}

void AbstractShape::redraw() {
    // static bool unloop;
    // if(unloop) return unloop = false, void();

    pPathHandle = {}; //.clear();
    if(!isSelected()) return;

    // unloop = true;
    redraw();
    // unloop = false;

    const auto hs = HandleSize * 0.5 * scale();

    std::ranges::for_each(handles,
        std::bind(qOverload<const QPointF&, qreal, qreal>(&QPainterPath::addEllipse),
            &pPathHandle, _1, hs, hs));

    setPos(1, 1), setPos(0, 0); // NOTE needed to update internal BSP etc, calling update() has no effect.
}

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

QVariant AbstractShape::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole: /*   */ return QString("%1 (%2)").arg(name()).arg(id_);
        case Qt::CheckStateRole: /**/ return isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: /**/ return icon();
        case FileTree::Id: /*      */ return id_;
        case FileTree::Select: /*  */ return isSelected();
        default: /*                */ return {};
        }
    default:
        return {};
    }
}

void AbstractShape::menu(QMenu& menu, FileTree::View* /*tv*/) {
    auto action = menu.addAction(
        QIcon::fromTheme("edit-delete"),
        QObject::tr("&Delete object \"%1\"").arg(name()), &menu,
        [this] { App::fileModel().removeRow(row(), index().parent()); });

    action = menu.addAction(
        QIcon::fromTheme("hint"),
        QObject::tr("&Visible \"%1\"").arg(name()), &menu,
        [this](bool fl) { Gi::Item::setVisible(fl); });
    action->setCheckable(true);
    action->setChecked(isVisible());

    action = menu.addAction(
        QIcon::fromTheme(""),
        QObject::tr("&Editable \"%1\"").arg(name()), &menu,
        [this](bool fl) { setEditable(fl); });
    action->setCheckable(true);
    action->setChecked(isEditable());

    action = menu.addAction(
        QIcon::fromTheme("document-edit"),
        QObject::tr("Edit Selected"), &menu,
        [this] { App::shapePlugin(type())->requestEditor(); });
}

void AbstractShape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) { // групповое перемещение
    event->setPos(App::settings().getSnappedPos(event->pos(), event->modifiers()));
    QGraphicsItem::mouseMoveEvent(event);
    if(curHandle.base()) {
        curHandle->setPos(event->pos());
        AbstractShape::redraw();
    }
    return;
    // const auto dp = App::settings().getSnappedPos(event->pos(), event->modifiers()) - initPos;
    // for(auto& [shape, hPos]: hInitPos) {
    //     for(auto&& [handler, pos]: std::ranges::zip_view(shape->handles, hPos))
    //         handler->setPos(pos + dp);
    //     shape->redraw();
    // }
}

void AbstractShape::mousePressEvent(QGraphicsSceneMouseEvent* event) { // групповое перемещение
    event->setPos(App::settings().getSnappedPos(event->pos(), event->modifiers()));
    if(isEditable()) {
        if(bool fl{}; scale(&fl) && fl) AbstractShape::redraw();
        initPos = event->pos();
        test(event->pos());
        qInfo() << event;
    }
    QGraphicsItem::mousePressEvent(event);
    // if(curHandle) return;
    // curHandle = &{};
    // hInitPos.clear();
    // const auto p(App::settings().getSnappedPos(event->pos(), event->modifiers()) - event->pos());
    // initPos = event->pos() + p;
    // for(auto item: scene()->selectedItems()) {
    //     if(item->type() >= Gi::Type::ShCircle) {
    //         auto* shape = static_cast<AbstractShape*>(item);
    //         hInitPos[shape].reserve(shape->handles.size());
    //         for(auto&& h: shape->handles) {
    //             h->setFlag(ItemSendsScenePositionChanges, false);
    //             hInitPos[shape].emplace_back(h->pos());
    //         }
    //     }
    // }
}

void AbstractShape::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    event->setPos(App::settings().getSnappedPos(event->pos(), event->modifiers()));
    qInfo() << event;
    QGraphicsItem::mouseReleaseEvent(event);
    if(isEditable()) {
        if(!curHandle.base() && !pos().isNull())
            for(auto&& h: handles) h.setPos(h.pos() + pos());
        curHandle = {};
    }
    AbstractShape::redraw();
}

// Node* AbstractShape::node() const { return node_; }

void AbstractShape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    App::shapePlugin(type())->requestEditor();
}

void AbstractShape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    class Dialog final : public QDialog {
        AbstractShape* as;

    public:
        Dialog(AbstractShape* as_)
            : as{as_} {
            setWindowFlags(Qt::Popup);
            setModal(false);
            auto dsbx = [this](auto get, auto set) {
                auto ds = new DoubleSpinBox{this};
                ds->setDecimals(3);
                ds->setRange(-1000, +1000);
                ds->setSuffix(QObject::tr(" mm"));
                ds->setValue((*as->curHandle.*get)());
                connect(ds, &QDoubleSpinBox::valueChanged, [this, set](auto val) {
                    (*as->curHandle.*set)(val);
                    as->AbstractShape::redraw();
                });
                return ds;
            };
            auto gl = new QFormLayout{this};
            gl->addRow(new QLabel{"X:", this}, dsbx(&Handle::x, &Handle::setX));
            gl->addRow(new QLabel{"Y:", this}, dsbx(&Handle::y, &Handle::setY));
            gl->setContentsMargins(6, 6, 6, 6);
        }
        ~Dialog() override = default;
        void leaveEvent(QEvent* event) override {
            event->accept(), reject();
        };
    };
    qInfo() << event;
    if(isEditable() && test(event->pos())) {
        Dialog dialog{this};
        dialog.move(event->screenPos());
        dialog.exec();
    } else {
        QMenu menu;
        AbstractShape::menu(menu, App::fileTreeViewPtr());
        menu.exec(event->screenPos());
    }
}

void AbstractShape::updateOtherhandles(Handle* h, int mode [[maybe_unused]]) {
    qInfo();
    setSelected(true);
    curHandle = HIter{h};
    AbstractShape::redraw();
    curHandle = {};
}

double AbstractShape::scale(bool* hasUpdate) const {
    static double scale_ = 1.0;
    if(auto views = scene()->views(); views.empty()) return scale_ = 1.0;
    else if(const auto scale = 1.0 / views[0]->transform().m11(); scale != scale_) {
        scale_ = scale;
        if(hasUpdate) *hasUpdate = true;
    }
    return scale_;
}

bool AbstractShape::test(const QPointF& point) {
    curHandle = {};
    const auto hSize = HandleSize * 0.5 * scale();
    for(auto&& var: handles) {
        QLineF line{point, var};
        if(line.length() < hSize) {
            curHandle = HIter{&var};
            return true;
        }
    }
    return curHandle.base();
}

} // namespace Shapes
