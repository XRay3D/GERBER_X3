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
#include "abstract_shape.h"
#include "ft_view.h"
#include "graphicsview.h"
#include "qgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

namespace Shapes {

static constexpr int HandleSize = 20;

enum Move : bool {
    SimgleMove,
    MultipleMove,
};

struct UndoMove final : QUndoCommand {
    UndoMove(UndoHandles&& undo, Move all = SimgleMove)
        : undo_{std::move(undo)} {
        if(all)
            for(auto&& [sh, hs]: undo_) // update handle pos after moving
                for(auto&& h: hs) h += sh->pos();
        else {
            std::swap(undo_.front().first->handles, undo_.front().second);
        }
    }

    void undo() override {
        for(auto&& [sh, hs]: undo_)
            std::swap(sh->handles, hs),
                sh->curHandle = {},
                sh->AbstractShape::redraw(); // set pos to zero
    }

    void redo() override {
        for(auto&& [sh, hs]: undo_)
            std::swap(sh->handles, hs),
                sh->curHandle = {},
                sh->AbstractShape::redraw(); // set pos to zero
    }

    UndoHandles undo_;
};

void drawPos(QPainter* painter, const QPointF& pos, double scale) {
    auto text = std::format(
        "  X = {0:.6g} {2}\n  Y = {1:.6g} {2}\n",
        pos.x() / App::settings().lenUnit(),
        pos.y() / App::settings().lenUnit(),
        App::settings().isBanana() ? "in" : "mm");

    const QRectF textRect = QFontMetricsF{painter->font()}.boundingRect(QRectF{}, Qt::AlignLeft, QString::fromStdString(text)); // FIXME

    painter->save();
    painter->translate(pos);
    painter->scale(scale, -scale);

    QPainterPath path;

    for(int i{}; auto&& txt: v::split(text, "\n"sv))
        path.addText(
            QPointF{textRect.left(), textRect.height() * 0.25 * ++i},
            painter->font(),
            QString::fromLatin1(txt));
    // TODO цвет текста в соответствии с темой?...
    static const auto zip = v::zip(
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
    // paths_.resize(1);
    AbstractShape::updateColors(); // в конструкторе -- без update(): сцены ещё нет
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable, false);
    setAcceptHoverEvents(true);
    setVisible(true);
    // setZValue(std::numeric_limits<double>::max());
}

AbstractShape::~AbstractShape() {
    if(App::projectPtr()) App::project().deleteShape(id_);
    plugin->editor()->remove(this);
}

std::vector<AbstractShape*> AbstractShape::shapes() {
    return {std::from_range,
        App::grView().items<AbstractShape>()
            | v::filter(&Gi::Item::isVisible)
            | v::filter(&Gi::Item::isSelected)
            | v::filter(&Gi::Item::isEditable)};
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
    painter->setBrush(QGraphicsItem::flags().testFlag(ItemIsSelectable)
            ? QBrush{brushColor_}
            : QBrush{Qt::NoBrush});
    painter->drawPath(shape_);

    if(!isEditable()) return;

    auto test = [this, painter](const QPointF& p) {
        double k = 10 * scaleFactor();
        // shape_.moveTo(p + QPointF{+k, 0});
        // shape_.lineTo(p + QPointF{-k, 0});
        // shape_.moveTo(p + QPointF{0, +k});
        // shape_.lineTo(p + QPointF{0, -k});
        painter->setPen(pen_);
        painter->setBrush(Qt::NoBrush);
        painter->drawLines({
            QLineF{p + QPointF{+k, 0}, p + QPointF{-k, 0}},
            QLineF{p + QPointF{0, +k}, p + QPointF{0, -k}}
        });
    };
    if(auto it = r::find(handles, Handle::Center, &Handle::type); it != handles.end())
        test(*it);

    if(!isSelected()) return;
#if 0
    painter->setPen({Qt::red, 1.0 * scale()});
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(pPathHandle.boundingRect());
#endif

    const auto hs = HandleSize * 0.5 * scale();
#if 1
    painter->setPen(QPen(Qt::black, 0.0));

    for(auto& handle: handles) {
        auto color = handle.color();
        if(&handle == curHandle) {
            color = Qt::cyan; // не magenta: она занята ручкой-центром дуги
            drawPos(painter, handle, scale());
        }
        color.setAlpha(100);
        painter->setBrush(color);
        painter->drawEllipse(handle, hs, hs);
    }

    // painter->setPen({Qt::white, 0.0});
    // painter->setBrush(Qt::NoBrush);
    // painter->drawPath(pPathHandle);

#else
    QPainterPath pPath{shape_ | pPathHandle};
    // pPath.closeSubpath();
    // pPath.addPath(pPathHandle);
    pPath.setFillRule(Qt::WindingFill);
    painter->setPen({Qt::white, 0.0});
    painter->setBrush(Qt::darkMagenta);
    painter->drawPath(pPath);
#endif
}

QRectF AbstractShape::boundingRect() const {
    if(isSelected()) [[unlikely]]
        return pPathHandle.boundingRect().united(shape_.boundingRect());
    return shape_.boundingRect();
}

QPainterPath AbstractShape::shape() const {
    if(isSelected() && isEditable()) [[unlikely]] {
        // if(curHandle) return pPathHandle;
        QPainterPath pPath{pPathHandle | shape_};
        // pPath.addPath(shape_);
        // pPath.setFillRule(Qt::WindingFill);
        return pPath;
    }
    return shape_;
}

// Paths AbstractShape::paths(int) const { return paths_; }

void AbstractShape::updateColors() {
    // animation.setStartValue(bodyColor_);
    switch(colorState) {
    case Default           : (brushColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF).setAlpha(50); break;
    case Hovered           : (brushColor_ = App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF).setAlpha(100); break;
    case Selected          : brushColor_ = QColor{255, 0x0, 0x0, 100}; break;
    case Hovered | Selected: brushColor_ = QColor{255, 0x0, 0x0, 150}; break;
    }
    // animation.setEndValue(bodyColor_);
    // animation.start();
}

void AbstractShape::redraw() {
    prepareGeometryChange(); // индекс сцены инвалидируется по СТАРОМУ rect
    rebuild();
    // Групповое перемещение двигает pos() item'а, а UndoMove потом вносит
    // этот сдвиг в ручки -- сам pos() надо обнулить, иначе сдвиг удвоится.
    if(!pos().isNull()) setPos({});
    updateHandleShape();
    update();
}

QVariant AbstractShape::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole   : return u"%1 (%2)"_s.arg(name()).arg(id_);
        case Qt::CheckStateRole: return isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: return icon();
        case FileTree::Id      : return id_;
        case FileTree::Select  : return isSelected();
        default                : return {};
        }
    case FileTree::Column::ItemsType:
        switch(role) {
        case Qt::CheckStateRole: return isEditable() ? Qt::Checked : Qt::Unchecked;
        case Qt::DisplayRole   : return QObject::tr("Editable");
        default                : return {};
        }
    default: return {};
    }
}

bool AbstractShape::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::CheckStateRole: return setVisible(value.value<Qt::CheckState>() == Qt::Checked), true;
        case FileTree::Id      : return id_ = value.toInt(), true;
        case FileTree::Select  : return setSelected(value.toBool()), true;
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
    case FileTree::Column::ItemsType       : return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side            : return itemFlag;
    default                                : return itemFlag;
    }
}

// FileTree::Node interface ////////////////////////////////////////////////////

void AbstractShape::menu(QMenu& menu, FileTree::View* /*tv*/) {

    auto addAction = [&menu](const QString& icon, const QString& text,
                         const QKeySequence& ks, auto&& func, auto... fl) {
        [[maybe_unused]] auto action = menu.addAction(
            QIcon::fromTheme(icon), text, ks, std::move(func));
        ((action->setCheckable(sizeof...(fl)), action->setChecked(fl)), ...);
    };

    addAction(
        u"edit-delete"_s, QObject::tr(R"(&Delete "%1")").arg(name()), {},
        [this] { App::fileModel().removeRow(row(), index().parent()); });

    addAction(
        u"hint"_s, QObject::tr(R"(&Visible "%1")").arg(name()), {},
        [this](bool fl) { setVisible(fl); }, isVisible());

    addAction(
        u"gnumeric-column-hide-symbolic"_s, QObject::tr("&Hide"), {},
        [] {
            for(auto* gi:
                App::grView().items<AbstractShape>()
                    | v::filter(&Gi::Item::isSelected))
                gi->setVisible(false);
        });

    addAction(
        {}, QObject::tr(R"(&Editable "%1")").arg(name()), {},
        [this](bool fl) { setEditable(fl), AbstractShape::redraw(); }, isEditable());

    addAction(
        u"document-edit"_s, QObject::tr("Edit Selected"), {},
        [this] { App::shapePlugin(type())->requestEditor(); });
}

double AbstractShape::scale(bool* hasUpdate) const {
    static double scale_ = 1.0;

    // if(static auto views = scene()->views(); views.empty()) return scale_ = 1.0;
    /*else*/ if(const auto scale = 1 / App::grView().getScale(); scale != scale_) {
        scale_ = scale;
        if(hasUpdate) *hasUpdate = true;
    }
    return scale_;
}

bool AbstractShape::inHandle(const QPointF& point) {
    curHandle = {};
    const auto hSize = HandleSize * 0.5 * scale();
    for(auto& var: handles)
        if(Geo::distance(point, var) <= hSize)
            return curHandle = &var, true;
    return false;
}

void AbstractShape::updateHandleShape() {
    const auto hs = HandleSize * 0.5 * scale();
    // QRectF rect{-hs, -hs, hs * 2, hs * 2};
    pPathHandle.clear();
    auto addEllipse
        = std::bind(qOverload<const QPointF&, qreal, qreal>(&QPainterPath::addEllipse),
            &pPathHandle, _1, hs, hs);
    r::for_each(handles, addEllipse);
}

// QGraphicsItem interface /////////////////////////////////////////////////////

void AbstractShape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    if(isEditable() && inHandle(event->pos()) && handleDoubleClick(*curHandle)) return;
    App::shapePlugin(type())->requestEditor();
}

void AbstractShape::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // QPointF pos = snappedPos(event);
    QGraphicsItem::mousePressEvent(event);
    if(!isEditable()) return;
    if(bool fl{}; scale(&fl) && fl) AbstractShape::redraw();
    moveFlag = (!inHandle(event->pos())) ? +1 : -1; // +1 групповое перемещение
}

void AbstractShape::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
    if(moveFlag > +1) {
        App::undoStack() // redo with redraw
            .push(new UndoMove{
                {std::from_range, shapes() | toPairs},
                MultipleMove
        });
    } else if(moveFlag < -1) {
        App::undoStack() // redo with redraw
            .push(new UndoMove{{{this, undoHandles}}});
        curHandle = {};
    }
    moveFlag = 0;
}

void AbstractShape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if(moveFlag > 0) { // групповое перемещение
        QGraphicsItem::mouseMoveEvent(event);
        ++moveFlag;
    } else if(moveFlag < 0) {
        if(--moveFlag == -2) undoHandles = handles; // copy for undo
        if(curHandle && isEditable()) {
            *curHandle = event->pos();
            AbstractShape::redraw();
        }
    }
    if(auto editor = App::shapePlugin(type())->editor();
        editor && editor->isVisible()) {
        auto tw = editor->findChild<QTableView*>();
        if(tw) tw->reset();
    }
}

void AbstractShape::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    updateHandleShape();
}

QVariant AbstractShape::itemChange(GraphicsItemChange change, const QVariant& value) {
    auto value_ = Gi::Item::itemChange(change, value);
    if(change == ItemSelectedHasChanged) {
        // boundingRect()/shape() зависят от isSelected() (объединение с ручками)
        prepareGeometryChange();
        updateHandleShape();
        if(value.toBool()) plugin->editor()->updateData();
    }
    return value_;
}

void AbstractShape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    class Dialog final : public QDialog {
        AbstractShape* as;
        // redraw() может вставлять/удалять ручки и переехать вектор,
        // сырой curHandle к этому моменту повиснет — держим индекс.
        size_t idx;

    public:
        Dialog(AbstractShape* as_, const QPoint& screenPos)
            : as{as_}, idx(as_->curHandle - as_->handles.data()) {
            move(screenPos);
            setWindowFlags(Qt::Popup);
            setModal(false);
            auto dsbx = [this](auto get, auto set) {
                auto ds = new DoubleSpinBox{this};
                ds->setDecimals(3);
                ds->setRange(-1000, +1000);
                ds->setSuffix(QObject::tr(" mm"));
                ds->setValue((as->handles[idx].*get)());
                connect(ds, &QDoubleSpinBox::valueChanged, [this, set](auto val) {
                    if(idx >= as->handles.size()) return;
                    (as->handles[idx].*set)(val);
                    as->curHandle = as->handles.data() + idx;
                    as->redraw();
                });
                return ds;
            };
            auto gl = new QFormLayout{this};
            gl->addRow(new QLabel{u"X:"_s, this}, dsbx(&Handle::x, &Handle::setX));
            gl->addRow(new QLabel{u"Y:"_s, this}, dsbx(&Handle::y, &Handle::setY));
            gl->setContentsMargins(6, 6, 6, 6);
        }
        ~Dialog() override = default;
        void leaveEvent(QEvent* event) override {
            event->accept(), reject();
        };
    };

    if(isEditable() && inHandle(event->pos())) {
        Dialog{this, event->screenPos()}.exec();
    } else {
        QMenu menu;
        AbstractShape::menu(menu, App::fileTreeViewPtr());
        menu.exec(event->screenPos());
    }
}

} // namespace Shapes
