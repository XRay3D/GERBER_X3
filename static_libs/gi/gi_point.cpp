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
#include "gi_point.h"

#include "drill/drill_file.h"
#include "gc_propertiesform.h"
#include "gc_types.h"
#include "gi.h"
#include "project.h"
#include "settingsdialog.h"
#include "tool_database.h"

#include <QGraphicsSceneEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStyleOptionGraphicsItem>

bool updateRect() {
    QRectF rect(App::grView().getSelectedBoundingRect());
    if(rect.isEmpty()) {
        if(QMessageBox::question(nullptr, "",
               QObject::tr("There are no selected items to define the border.\n"
                           "The old border will be used."),
               QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::No)
            return false;
    }
    App::layoutFrames().updateRect();
    return true;
}

namespace Gi {

Marker::Marker(Type type)
    : QGraphicsObject{nullptr}
    , type_{type} {
    setAcceptHoverEvents(true);
    if(type_ == Home) {
        App::setHome(this);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
        setToolTip(QObject::tr("G-Code Home Point"));
    } else {
        App::setZero(this);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
        setToolTip(QObject::tr("G-Code Zero Point"));
    }
    shape_.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    rect_ = path_.boundingRect();
}

Marker::~Marker() {
    (type_ == Home) ? App::setHome(nullptr) : App::setZero(nullptr);
}

QRectF Marker::boundingRect() const {
    if(App::drawPdf())
        return {};
    return rect_;
}

void Marker::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    if(App::drawPdf())
        return;

    QColor c(type_ == Home ? App::settings().guiColor(GuiColors::Home) : App::settings().guiColor(GuiColors::Zero));
    if(option->state & QStyle::State_MouseOver)
        c.setAlpha(200);
    if(!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));
    if(App::settings().scaleHZMarkers()) {
        auto sf = App::grView().scaleFactor() * 10;
        painter->scale(sf, sf);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(path_);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Marker::shape() const { return App::drawPdf() ? QPainterPath() : path_; }

int Marker::type() const { return static_cast<int>(type_ ? Gi::Type::MarkHome : Gi::Type::MarkZero); }

void Marker::resetPos(bool flUpdateRect) {
    if(flUpdateRect && !updateRect())
        return;

    const QRectF rect(App::layoutFrames().boundingRect());

    static constexpr std::array corner{
        &QRectF::bottomLeft,
        &QRectF::bottomRight,
        &QRectF::topLeft,
        &QRectF::topRight,
        &QRectF::center,
        &QRectF::center,
    };

    if(type_ == Home)
        if(App::settings().mkrHomePos() == corner.size())
            setPos({});
        else
            setPos((rect.*corner[App::settings().mkrHomePos()])() + App::settings().mkrHomeOffset());
    else if(App::settings().mkrZeroPos() == corner.size())
        setPos({});
    else
        setPos((rect.*corner[App::settings().mkrZeroPos()])() + App::settings().mkrZeroOffset());

    updateGCPForm();

    if(type_ == Home)
        App::project().setHomePos(pos());
    else
        App::project().setZeroPos(pos());
}

void Marker::setPosX(double x) {
    QPointF point(pos());
    if(qFuzzyCompare(point.x(), x))
        return;
    point.setX(x);
    setPos(point);
}

void Marker::setPosY(double y) {
    QPointF point(pos());
    if(qFuzzyCompare(point.y(), y))
        return;
    point.setY(y);
    setPos(point);
    if(type_ == Home)
        App::project().setHomePos(pos());
    else
        App::project().setZeroPos(pos());
}

void Marker::updateGCPForm() {
    if(App::gcPropertiesFormPtr())
        App::gcPropertiesForm().updatePosDsbxs();

    if(type_ == Zero) {
        App::project().setZeroPos(pos());
        for(auto pin: App::pins())
            pin->updateToolTip();

    } else {
        App::project().setHomePos(pos());
    }
}

void Marker::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseMoveEvent(event);
    setPos(App::settings().getSnappedPos(pos(), event->modifiers()));

    updateGCPForm();
}

void Marker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if(!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    // QMatrix matrix(scene()->views().first()->matrix());
    // matrix.translate(-pos().x(), pos().y());
    // scene()->views().first()->setMatrix(matrix);
    updateGCPForm();
    QGraphicsItem::mouseDoubleClickEvent(event);
}
void Marker::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu;
    auto action = menu.addAction(QObject::tr("Fixed"), this, [this](bool fl) {
        setFlag(QGraphicsItem::ItemIsMovable, !fl);
    });
    action->setCheckable(true);
    action->setChecked(!(flags() & QGraphicsItem::ItemIsMovable));
    menu.addSeparator();
    action = menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Settings"), [] {
        SettingsDialog(nullptr, SettingsDialog::Utils).exec();
    });
    menu.exec(event->screenPos());
}

////////////////////////////////////////////////
/// \brief Pin::Pin
/// \param parent
///
Pin::Pin()
    : QGraphicsObject{nullptr}
    , index_(ctr_++) {
    setObjectName("Pin");
    setAcceptHoverEvents(true);

    if(index_ % 2) {
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        path_.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    shape_.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    rect_ = path_.boundingRect();

    setZValue(std::numeric_limits<double>::max() - index_);
}

Pin::~Pin() { }

QRectF Pin::boundingRect() const {
    if(App::drawPdf())
        return {};
    return rect_;
}

void Pin::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    if(App::drawPdf())
        return;

    QColor c(App::project().pinUsed(index_) ? App::settings().guiColor(GuiColors::Pin) : QColor(127, 127, 127, 127));
    if(option->state & QStyle::State_MouseOver)
        c.setAlpha(200);
    if(!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));
    // c.setAlpha(50);
    if(App::settings().scalePinMarkers()) {
        auto sf = App::grView().scaleFactor() * 10;
        painter->scale(sf, sf);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(path_);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Pin::shape() const {
    if(App::drawPdf())
        return {};
    return shape_;
}

void Pin::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseMoveEvent(event);
    setPos(App::settings().getSnappedPos(pos(), event->modifiers()));

    QPointF pt[4]{
        App::pin0().pos(),
        App::pin1().pos(),
        App::pin2().pos(),
        App::pin3().pos()};

    const QPointF center(App::layoutFrames().boundingRect().center());
    // const QPointF center(App::project().worckRect().center());

    switch(index_) {
    case 0:
        if(pt[0].x() > center.x())
            pt[0].rx() = center.x();
        if(pt[0].y() > center.y())
            pt[0].ry() = center.y();
        pt[2] = App::pin2().lastPos_ - (pt[0] - lastPos_);
        pt[1].rx() = pt[2].x();
        pt[1].ry() = pt[0].y();
        pt[3].rx() = pt[0].x();
        pt[3].ry() = pt[2].y();
        break;
    case 1:
        if(pt[1].x() < center.x())
            pt[1].rx() = center.x();
        if(pt[1].y() > center.y())
            pt[1].ry() = center.y();
        pt[3] = App::pin3().lastPos_ - (pt[1] - lastPos_);
        pt[0].rx() = pt[3].x();
        pt[0].ry() = pt[1].y();
        pt[2].rx() = pt[1].x();
        pt[2].ry() = pt[3].y();
        break;
    case 2:
        if(pt[2].x() < center.x())
            pt[2].rx() = center.x();
        if(pt[2].y() < center.y())
            pt[2].ry() = center.y();
        pt[0] = App::pin0().lastPos_ - (pt[2] - lastPos_);
        pt[1].rx() = pt[2].x();
        pt[1].ry() = pt[0].y();
        pt[3].rx() = pt[0].x();
        pt[3].ry() = pt[2].y();
        break;
    case 3:
        if(pt[3].x() > center.x())
            pt[3].rx() = center.x();
        if(pt[3].y() < center.y())
            pt[3].ry() = center.y();
        pt[1] = App::pin1().lastPos_ - (pt[3] - lastPos_);
        pt[0].rx() = pt[3].x();
        pt[0].ry() = pt[1].y();
        pt[2].rx() = pt[1].x();
        pt[2].ry() = pt[3].y();
        break;
    }

    for(int i = 0; i < 4; ++i)
        App::pins()[i]->setPos(pt[i]);
    App::project().setPinsPos(pt);
}

void Pin::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if(!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void Pin::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    for(int i = 0; i < 4; ++i)
        App::pins()[i]->lastPos_ = App::pins()[i]->pos();
    QGraphicsItem::mousePressEvent(event);
}
void Pin::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu;

    auto action = menu.addAction(QIcon::fromTheme("drill-path"), tr("&Create path for Pins"), [] {
        ToolDatabase tdb(App::grViewPtr(), {Tool::Drill, Tool::EndMill});
        if(tdb.exec()) {
            Tool tool(tdb.tool());

            QPolygonF dst;

            for(Pin* pin: {&App::pin0(), &App::pin1(), &App::pin2(), &App::pin3()}) {
                pin->setFlag(QGraphicsItem::ItemIsMovable, false);
                QPointF point(pin->pos());
                if(dst.contains(point))
                    continue;
                if(App::project().pinUsed(pin->index_))
                    dst.push_back(point);
            }

            QSettings settings;
            settings.beginGroup("Pin");
            bool ok;
            double depth = QInputDialog::getDouble(
                nullptr,                            // parent
                "",                                 // title
                tr("Set Depth"),                    // label
                settings.value("depth").toDouble(), // value
                0,                                  // minValue
                20,                                 // maxValue
                1,                                  // decimals
                &ok,                                // ok
                Qt::WindowFlags(),                  // flags
                1                                   // step
            );
            if(!ok)
                return;

            if(depth == 0.0)
                return;

            settings.setValue("depth", depth);
            settings.endGroup();

            GCode::Params gcp_{tool, depth};
            gcp_.params[GCode::Params::NotTile];

            auto drillls = new Drilling::File{std::move(gcp_), Pathss{{~dst}}};
            drillls->setFileName(tr("Pin_") + tool.nameEnc());
            App::project().addFile(drillls);
        }
    });

    action = menu.addAction(tr("Fixed"), [](bool fl) {
        for (Pin* pin : {&App::pin0(), &App::pin1(), &App::pin2(), &App::pin3()}) pin->setFlag(QGraphicsItem::ItemIsMovable, !fl); });
    action->setCheckable(true);
    action->setChecked(!(App::pin0().flags() & QGraphicsItem::ItemIsMovable));

    action = menu.addAction(tr("Used"), [this](bool fl) {
        App::project().setPinUsed(fl, index_); update(); });
    action->setCheckable(true);
    action->setChecked(App::project().pinUsed(index_));

    menu.addSeparator();
    action = menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Settings"), [] {
        SettingsDialog(nullptr, SettingsDialog::Utils).exec();
    });
    menu.exec(event->screenPos());
}

int Pin::type() const { return Type::MarkPin; }

void Pin::setPinsPos(QPointF pos[]) {
    for(int i = 0; i < 4; ++i)
        App::pins()[i]->setPos(pos[i]);
}

void Pin::resetPos(bool fl) {
    if(fl)
        if(!updateRect())
            return;

    const QPointF offset(App::settings().mkrPinOffset());
    const QRectF rect(App::layoutFrames().boundingRect()); // App::project().worckRect()

    QPointF pt[]{
        QPointF(rect.topLeft() + QPointF(-offset.x(), -offset.y())),
        QPointF(rect.topRight() + QPointF(+offset.x(), -offset.y())),
        QPointF(rect.bottomRight() + QPointF(+offset.x(), +offset.y())),
        QPointF(rect.bottomLeft() + QPointF(-offset.x(), +offset.y())),
    };

    const QPointF center(rect.center());

    if(pt[0].x() > center.x())
        pt[0].setX(center.x());
    if(pt[0].y() > center.y())
        pt[0].setY(center.y());
    if(pt[1].x() < center.x())
        pt[1].setX(center.x());
    if(pt[1].y() > center.y())
        pt[1].setY(center.y());
    if(pt[2].x() < center.x())
        pt[2].setX(center.x());
    if(pt[2].y() < center.y())
        pt[2].setY(center.y());
    if(pt[3].x() > center.x())
        pt[3].setX(center.x());
    if(pt[3].y() < center.y())
        pt[3].setY(center.y());

    for(int i = 0; i < 4; ++i)
        App::pins()[i]->setPos(pt[i]);

    App::project().setPinsPos(pt);
}

void Pin::setPos(const QPointF pos[]) {
    for(int i = 0; i < 4; ++i)
        App::pins()[i]->setPos(pos[i]);
}

void Pin::updateToolTip() {
    const QPointF p(pos() - App::zero().pos());
    setToolTip(QObject::tr("Pin %1\nX %2:Y %3")
                   .arg(index_ + 1)
                   .arg(p.x())
                   .arg(p.y()));
}

void Pin::setPos(const QPointF& pos) {
    QGraphicsItem::setPos(pos);
    updateToolTip();
}

} // namespace Gi

////////////////////////////////////////////////
LayoutFrames::LayoutFrames()
    : QGraphicsObject{nullptr} {
    setZValue(-std::numeric_limits<double>::max());
    setFlag(ItemIsSelectable, false);
    App::setLayoutFrames(this);
}

LayoutFrames::~LayoutFrames() {
    App::setLayoutFrames(nullptr);
}

int LayoutFrames::type() const {
    return Gi ::Type::MarkLayoutFrames;
}

QRectF LayoutFrames::boundingRect() const {
    if(App::drawPdf())
        return {};
    return rect_;
}

void LayoutFrames::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(Qt::NoBrush);

    QPen pen(QColor(255, 0, 255), 2.0 * App::grView().scaleFactor());
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);

    painter->drawPath(path_);
    painter->restore();
}

void LayoutFrames::updateRect(bool fl) {
    QPainterPath path;
    QRectF rect(App::project().worckRect());
    rect_ = rect;

    const int stepsY(App::project().stepsY()), stepsX(App::project().stepsX());
    const double spaceY_(App::project().spaceY()), spaceX_(App::project().spaceX());

    rect_.setHeight(rect_.height() * stepsY + spaceY_ * (stepsY - 1));
    rect_.setWidth(rect_.width() * stepsX + spaceX_ * (stepsX - 1));

    for(int x = 0; x < stepsX; ++x) {
        for(int y = 0; y < stepsY; ++y) {
            if(x || y) {
                path.addRect(rect.translated(
                    (rect.width() + spaceX_) * x,
                    (rect.height() + spaceY_) * y));
            } else {
                const double spaceX = qFuzzyIsNull(spaceX_) ? 2 : spaceX_ * 0.5;
                const double spaceY = qFuzzyIsNull(spaceY_) ? 2 : spaceY_ * 0.5;

                path.moveTo(rect.bottomLeft() + QPointF(-spaceX, 0));
                path.lineTo(rect.bottomLeft());
                path.lineTo(rect.bottomLeft() + QPointF(0, +spaceY));

                path.moveTo(rect.bottomRight() + QPointF(+spaceX, 0));
                path.lineTo(rect.bottomRight());
                path.lineTo(rect.bottomRight() + QPointF(0, +spaceY));

                path.moveTo(rect.topLeft() + QPointF(-spaceX, 0));
                path.lineTo(rect.topLeft());
                path.lineTo(rect.topLeft() + QPointF(0, -spaceY));

                path.moveTo(rect.topRight() + QPointF(+spaceX, 0));
                path.lineTo(rect.topRight());
                path.lineTo(rect.topRight() + QPointF(0, -spaceY));

                {
                    const double mid = rect.height() / 2;
                    const double len = rect.height() / 20;

                    path.moveTo(rect.topLeft() + QPointF(0, mid - len));
                    path.lineTo(rect.topLeft() + QPointF(0, mid + len));

                    path.moveTo(rect.topRight() + QPointF(0, mid - len));
                    path.lineTo(rect.topRight() + QPointF(0, mid + len));

                    path.moveTo(rect.topLeft() + QPointF(0, mid));
                    path.lineTo(rect.topLeft() + QPointF(-spaceX, mid));

                    path.moveTo(rect.topRight() + QPointF(0, mid));
                    path.lineTo(rect.topRight() + QPointF(+spaceX, mid));
                }

                {
                    const double mid = rect.width() / 2;
                    const double len = rect.width() / 20;

                    path.moveTo(rect.bottomLeft() + QPointF(mid - len, 0));
                    path.lineTo(rect.bottomLeft() + QPointF(mid + len, 0));

                    path.moveTo(rect.topLeft() + QPointF(mid - len, 0));
                    path.lineTo(rect.topLeft() + QPointF(mid + len, 0));

                    path.moveTo(rect.bottomLeft() + QPointF(mid, 0));
                    path.lineTo(rect.bottomLeft() + QPointF(mid, +spaceY));

                    path.moveTo(rect.topLeft() + QPointF(mid, 0));
                    path.lineTo(rect.topLeft() + QPointF(mid, -spaceY));
                }
            }
        }
    }
    path_ = std::move(path);
    QGraphicsItem::update();
    if(fl) {
        App::home().resetPos(false);
        App::zero().resetPos(false);
        Gi ::Pin::resetPos(false);
    }
}

#include "moc_gi_point.cpp"
