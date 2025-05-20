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

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <set>

namespace Shapes {

static constexpr int HandleR = 10; // R pix

QDataStream& operator<<(QDataStream& stream, const Handle& handle) {
    return stream << BlockWrite{static_cast<const QPointF&>(handle), handle.type_}, stream;
}

QDataStream& operator>>(QDataStream& stream, Handle& handle) {
    return stream >> BlockRead{static_cast<QPointF&>(handle), handle.type_}, stream;
}

QDataStream& operator<<(QDataStream& stream, const AbstractShape& shape) {
    BlockWrite write{shape.id_, shape.isVisible(), shape.isEditable(), shape.handles};
    shape.write(write);
    return stream << write;
}

#if 0
template <typename T, auto Get, auto Set>
struct RWS {
    T* obj;
    using Arg = std::remove_cvref_t<decltype((obj->*Get)())>;
    operator Arg() const { (obj->*Get)(); }
    auto& operator=(const Arg& val) { return (obj->*Set)(val), *obj; }
    template <typename Stream>
    Stream& operator>>(Stream& stream) {
        Arg arg;
        stream >> arg;
        (obj->*Set)(arg);
        return stream;
    }
    template <typename Stream>
    Stream& operator<<(Stream& stream) const {
        return stream << (obj->*Get)();
    }
};

using setVisible = RWS<AbstractShape*, &AbstractShape::isVisible, &AbstractShape::setVisible>;
using setEditable = RWS<AbstractShape*, &AbstractShape::isEditable, &AbstractShape::setEditable>;
#endif

QDataStream& operator>>(QDataStream& stream, AbstractShape& shape) {
    bool bFlag[2];
    if(App::project().ver() == Project::Ver_7) { // Load Prewios
        BlockRead in{shape.id_, bFlag[0]};
        stream >> in;
        uint32_t size;
        in >> bFlag[1] >> size;
        shape.handles.resize(size);
        for(auto&& handle: shape.handles)
            in >> static_cast<QPointF&>(handle) >> handle.type_;
        shape.readAndInit(in);
    } else {
        bool bFlag[2];
        shape.readAndInit(stream >> BlockRead{shape.id_, bFlag[0], bFlag[1], shape.handles});
    }
    shape.setVisible(bFlag[0]);
    shape.setEditable(bFlag[1]);
    shape.setToolTip(shape.name() % QString::number(shape.id_));
    shape.setZValue(shape.id_);
    return stream;
}

void drawPos(QPainter* painter, const QPointF& pos, double scale) {
    auto text = std::format(
        "  X = {0:.6g} {2}\n  Y = {1:.6g} {2}\n",
        pos.x() / App::settings().lenUnit(),
        pos.y() / App::settings().lenUnit(),
        App::settings().isBanana() ? "in" : "mm");

    const QRectF textRect = QFontMetricsF{painter->font()}.boundingRect(QRectF{}, Qt::AlignLeft, text.data());

    painter->save();
    painter->translate(pos);
    painter->scale(scale, -scale);

    QPainterPath path;

    for(int i{}; auto&& txt: std::views::split(text, "\n"sv))
        path.addText(QPointF{textRect.left(), textRect.height() * 0.25 * ++i},
            painter->font(), QString::fromLatin1(txt.data(), txt.size()));
    // TODO цвет текста в соответствии с темой?...
    static const auto zip = std::views::zip(
        std::array{
            QPen{Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin},
            QPen{Qt::NoPen}
    },
        std::array{QBrush{Qt::NoBrush}, QBrush{Qt::white}});
    for(auto&& [pen, brush]: zip) {
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPath(path);
    }

    painter->restore();
}

AbstractShape::AbstractShape(Plugin* plugin)
    : FileTree::Node{FileTree::AbstractShape}
    , plugin{plugin} {
    paths_.resize(1);
    AbstractShape::changeColor();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setVisible(true);
    // setZValue(std::numeric_limits<double>::max());
}

AbstractShape::~AbstractShape() {
    if(App::projectPtr()) App::project().deleteShape(id_);
    plugin->editor()->remove(this);
}

double AbstractShape::scale(bool* hasUpdate) const {
    static double scale_ = 1.0;
    if(static auto views = scene()->views(); views.empty()) return scale_ = 1.0;
    else if(const auto scale = 1.0 / views[0]->transform().m11(); scale != scale_) {
        scale_ = scale;
        hSize = std::pow(HandleR * scale, 2);
        if(hasUpdate) *hasUpdate = true;
    }
    return scale_;
}

bool AbstractShape::test(const QPointF& point) {
    curHandle = {};
    for(auto&& var: handles) {
        const auto pt = var - point;
        const auto length = pt.x() * pt.x() + pt.y() * pt.y();
        if(length <= hSize) {
            curHandle = HIter{&var};
            curHandlePos = *curHandle;
            return true;
        }
    }
    return false;
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

#if 0
    painter->setPen({Qt::red, 1.0 * scale()});
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(pPathHandle.boundingRect());
#endif

    painter->setPen(Qt::NoPen);
    bool fl{};
    const auto hs = HandleR * scale(&fl);
    if(fl) AbstractShape::redraw();

    painter->setPen(QPen(Qt::black, 0.0));

    for(auto&& var: handles) {
        auto color = var.color();
        if(&var == curHandle.base()) {
            color = Qt::magenta;
            drawPos(painter, var, scale());
        }
        color.setAlpha(100);
        painter->setBrush(color);
        painter->drawEllipse(var, hs, hs);
    }
}

QRectF AbstractShape::boundingRect() const {
    if(isSelected()) [[unlikely]]
        return pPathHandle.boundingRect().united(shape_.boundingRect());
    return shape_.boundingRect();
}

QPainterPath AbstractShape::shape() const {
    if(isSelected()) [[unlikely]]
        return pPathHandle | shape_;
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
        brushColor_ = QColor{255, 0x0, 0x0, 100};
        break;
    case Hovered | Selected:
        brushColor_ = QColor{255, 0x0, 0x0, 150};
        break;
    }

    //    animation.setEndValue(bodyColor_);
    //    animation.start();
}

void AbstractShape::redraw() {
    if(static int fl; !fl) ++fl, redraw(), --fl; // call child overload once

    pPathHandle.clear();
    if(!isEditable()) return;

    const auto hs = HandleR * scale();

    std::ranges::for_each(handles,
        std::bind(qOverload<const QPointF&, qreal, qreal>(&QPainterPath::addEllipse),
            &pPathHandle, _1, hs, hs));

    setPos(1, 1), setPos(0, 0); // NOTE needed to update internal data (BSP etc),
                                // calling update() has no effect.
}

// FileTree::Node interface ////////////////////////////////////////////////////

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
    case FileTree::Column::ItemsType:
        switch(role) {
        case Qt::CheckStateRole: /**/ return isEditable() ? Qt::Checked : Qt::Unchecked;
        case Qt::DisplayRole: /*   */ return QObject::tr("Editable");
        default: /*                */ return {};
        }
    default: return {};
    }
}

bool AbstractShape::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::CheckStateRole:
            return setVisible(value.value<Qt::CheckState>() == Qt::Checked), true;
        case FileTree::Id:
            return id_ = value.toInt(), true;
        case FileTree::Select:
            return setSelected(value.toBool()), true;
        }
        break;
    case FileTree::Column::ItemsType:
        if(role == Qt::CheckStateRole)
            return setEditable(value.value<Qt::CheckState>() == Qt::Checked), true;
        break;
    default: break;
    }
    return false;
}

Qt::ItemFlags AbstractShape::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
    case FileTree::Column::ItemsType:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag;
    default:
        return itemFlag;
    }
}

void AbstractShape::menu(QMenu& menu, FileTree::View* /*tv*/) {

    auto addAction = [&menu](const QString& icon, const QString& text,
                         const QKeySequence& ks, auto&& func, auto... fl) {
        [[maybe_unused]] auto action = menu.addAction(
            QIcon::fromTheme(icon), text, ks, std::move(func));
        ((action->setCheckable(true), action->setChecked(fl)), ...);
    };

    addAction(
        "edit-delete", QObject::tr(R"(&Delete object "%1")").arg(name()), {},
        [this] { App::fileModel().removeRow(row(), index().parent()); });

    addAction(
        "hint", QObject::tr(R"(&Visible "%1")").arg(name()), {},
        [this](bool fl) { setVisible(fl); }, isVisible());

    addAction(
        "", QObject::tr(R"(&Editable "%1")").arg(name()), {},
        [this](bool fl) { setEditable(fl), AbstractShape::redraw(); }, isEditable());

    addAction(
        "document-edit", QObject::tr("Edit Selected"), {},
        [this] { App::shapePlugin(type())->requestEditor(); });
}

// QGraphicsItem interface /////////////////////////////////////////////////////

static std::set<AbstractShape*> set;

QVariant AbstractShape::itemChange(GraphicsItemChange change, const QVariant& value) {
    auto value_ = Gi::Item::itemChange(change, value);
    if(change == ItemSelectedHasChanged) // && value.toBool())
    {
        if(value.toBool())
            set.insert(this);
        else
            set.erase(this);
        plugin->editor()->updateData();
    }
    return value_;
}

void AbstractShape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) { // групповое перемещение
    if(isEditable() && curHandle.base()) {
        *curHandle = event->pos();
        AbstractShape::redraw();
        event->accept();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void AbstractShape::mousePressEvent(QGraphicsSceneMouseEvent* event) { // групповое перемещение
    if(isEditable()) {
        if(bool fl{}; scale(&fl) && fl) AbstractShape::redraw();
        initPos = event->pos();
        test(event->pos());
        qInfo() << event;
    }
    QGraphicsItem::mousePressEvent(event);
}

void AbstractShape::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if(isEditable()) {
        class ShapeMoveCommand : public QUndoCommand {
            AbstractShape* const shape;
            // ssize_t curHandle{};
            QPointF pos;
            // mvector<Handle> handles;

        public:
            ShapeMoveCommand(AbstractShape* shape, QUndoCommand* parent = nullptr)
                : QUndoCommand{parent}
                , shape{shape}
                , pos{shape->pos()} {
                // , curHandle{std::distance(shape->handles.begin(), shape->curHandle)}
                // , handles{shape->handles} {
                setText("AbstractShape Handle Moved");
            }

            ~ShapeMoveCommand() override = default;

            void undo() override {
                // shape->handles = handles;
                // if(-1 < curHandle && curHandle < static_cast<ssize_t>(shape->handles.size())) {
                //     shape->handles[curHandle] = shape->curHandlePos;
                //     shape->curHandle = shape->handles.begin() + curHandle;
                // }
                // shape->AbstractShape::redraw();
                // shape->curHandle = {};
                if(pos.isNull())
                    for(auto* shape: set)
                        if(!shape->curHandle.base() && !shape->pos().isNull()) {
                            for(auto pos = shape->pos(); auto&& h: shape->handles) h -= pos;
                            shape->curHandle = {};
                            shape->AbstractShape::redraw();
                        }
            }

            void redo() override {
                if(pos.isNull())
                    for(auto* shape: set)
                        if(!shape->curHandle.base() && !shape->pos().isNull()) {
                            for(auto pos = shape->pos(); auto&& h: shape->handles) h += pos;
                            shape->curHandle = {};
                            shape->AbstractShape::redraw();
                        }
            }
        };
        // App::undoStack().push(new ShapeMoveCommand{this});
        if(!curHandle.base() && !pos().isNull()) {
            for(auto pos_ = pos(); auto&& h: handles) h += pos_;
            curHandle = {};
            AbstractShape::redraw();
        }
    }
    AbstractShape::redraw();
    QGraphicsItem::mouseReleaseEvent(event);
}

void AbstractShape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    App::shapePlugin(type())->requestEditor();
}

void AbstractShape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    class Dialog final : public QDialog {
        AbstractShape* as;

    public:
        Dialog(AbstractShape* as_, const QPoint& screenPos)
            : as{as_} {
            move(screenPos);
            setWindowFlags(Qt::Popup);
            setModal(false);
            auto dsbx = [this](auto get, auto set) {
                auto ds = new DoubleSpinBox{this};
                ds->setDecimals(3);
                ds->setRange(-1000, +1000);
                ds->setSuffix(QObject::tr(" mm"));
                // ds->setValue((*as->curHandle.*get)());
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
        Dialog{this, event->screenPos()}.exec();
    } else {
        QMenu menu;
        AbstractShape::menu(menu, App::fileTreeViewPtr());
        menu.exec(event->screenPos());
    }
}

} // namespace Shapes
