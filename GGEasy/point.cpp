// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "point.h"
#include "project.h"

#include "gcode/gcode.h"

#include "forms/gcodepropertiesform.h"
#include "graphicsview.h"
#include "project.h"
#include "settings.h"
#include "toolpch.h"
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOptionGraphicsItem>


#include "settingsdialog.h"

using namespace ClipperLib;

bool updateRect()
{
    QRectF rect(App::scene()->getSelectedBoundingRect());
    if (rect.isEmpty()) {
        if (QMessageBox::question(nullptr, "",
                QObject::tr("There are no selected items to define the border.\n"
                            "The old border will be used."),
                QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::No)
            return false;
    }
    App::layoutFrames()->updateRect();
    return true;
}

Marker::Marker(Type type)
    : QGraphicsObject(nullptr)
    , m_type(type)
{
    m_markers[type] = this;
    setAcceptHoverEvents(true);
    if (m_type == Home) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
        setToolTip(QObject::tr("G-Code Home Point"));
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
        setToolTip(QObject::tr("G-Code Zero Point"));
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();
}

Marker::~Marker() { m_markers[m_type] = nullptr; }

QRectF Marker::boundingRect() const
{
    if (App::scene()->drawPdf())
        return QRectF();
    return m_rect;
}

void Marker::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (App::scene()->drawPdf())
        return;

    QColor c(m_type == Home ? App::settings().guiColor(GuiColors::Home) : App::settings().guiColor(GuiColors::Zero));
    if (option->state & QStyle::State_MouseOver)
        c.setAlpha(200);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));
    if (App::settings().scaleHZMarkers()) {
        auto sf = App::graphicsView()->scaleFactor() * 10;
        painter->scale(sf, sf);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Marker::shape() const { return App::scene()->drawPdf() ? QPainterPath() : m_path; }

int Marker::type() const { return static_cast<int>(m_type ? GiType::MarkHome : GiType::MarkZero); }

void Marker::resetPos(bool flUpdateRect)
{
    if (flUpdateRect && !updateRect())
        return;

    const QRectF rect(App::layoutFrames()->boundingRect());

    if (m_type == Home)
        switch (App::settings().mkrHomePos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + App::settings().mkrHomeOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + App::settings().mkrHomeOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + App::settings().mkrHomeOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + App::settings().mkrHomeOffset());
            break;
        default:
            setPos({});
            break;
        }
    else {
        switch (App::settings().mkrZeroPos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + App::settings().mkrZeroOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + App::settings().mkrZeroOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + App::settings().mkrZeroOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + App::settings().mkrZeroOffset());
            break;
        default:
            setPos({});
            break;
        }
    }
    updateGCPForm();
    if (m_type == Home)
        App::project()->setHomePos(pos());
    else
        App::project()->setZeroPos(pos());
}

void Marker::setPosX(double x)
{
    QPointF point(pos());
    if (qFuzzyCompare(point.x(), x))
        return;
    point.setX(x);
    setPos(point);
}

void Marker::setPosY(double y)
{
    QPointF point(pos());
    if (qFuzzyCompare(point.y(), y))
        return;
    point.setY(y);
    setPos(point);
    if (m_type == Home)
        App::project()->setHomePos(pos());
    else
        App::project()->setZeroPos(pos());
}

void Marker::updateGCPForm()
{
    if (App::gCodePropertiesForm())
        App::gCodePropertiesForm()->updatePosDsbxs();

    if (m_type == Zero) {
        App::project()->setZeroPos(pos());
        for (auto pin : Pin::pins())
            pin->updateToolTip();

    } else {
        App::project()->setHomePos(pos());
    }
}

void Marker::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    setPos(App::settings().getSnappedPos(pos(), event->modifiers()));

    updateGCPForm();
}

void Marker::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    //    QMatrix matrix(scene()->views().first()->matrix());
    //    matrix.translate(-pos().x(), pos().y());
    //    scene()->views().first()->setMatrix(matrix);
    updateGCPForm();
    QGraphicsItem::mouseDoubleClickEvent(event);
}
void Marker::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    auto action = menu.addAction(QObject::tr("Fixed"), [this](bool fl) {
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
    : QGraphicsObject(nullptr)
    , m_index(m_ctr++)
{
    setObjectName("Pin");
    setAcceptHoverEvents(true);

    if (m_index % 2) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();

    setZValue(std::numeric_limits<qreal>::max() - m_index);
    m_pins[m_index] = this;
}

Pin::~Pin() { }

QRectF Pin::boundingRect() const
{
    if (App::scene()->drawPdf())
        return QRectF();
    return m_rect;
}

void Pin::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (App::scene()->drawPdf())
        return;

    QColor c(App::project()->pinUsed(m_index) ? App::settings().guiColor(GuiColors::Pin)
                                              : QColor(127, 127, 127, 127));
    if (option->state & QStyle::State_MouseOver)
        c.setAlpha(200);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));
    //c.setAlpha(50);
    if (App::settings().scalePinMarkers()) {
        auto sf = App::graphicsView()->scaleFactor() * 10;
        painter->scale(sf, sf);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Pin::shape() const
{
    if (App::scene()->drawPdf())
        return QPainterPath();
    return m_shape;
}

void Pin::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    setPos(App::settings().getSnappedPos(pos(), event->modifiers()));

    QPointF pt[4] {
        m_pins[0]->pos(),
        m_pins[1]->pos(),
        m_pins[2]->pos(),
        m_pins[3]->pos()
    };

    const QPointF center(App::layoutFrames()->boundingRect().center());
    //const QPointF center(App::project()->worckRect().center());

    switch (m_index) {
    case 0:
        if (pt[0].x() > center.x())
            pt[0].rx() = center.x();
        if (pt[0].y() > center.y())
            pt[0].ry() = center.y();
        pt[2] = m_pins[2]->m_lastPos - (pt[0] - m_lastPos);
        pt[1].rx() = pt[2].x();
        pt[1].ry() = pt[0].y();
        pt[3].rx() = pt[0].x();
        pt[3].ry() = pt[2].y();
        break;
    case 1:
        if (pt[1].x() < center.x())
            pt[1].rx() = center.x();
        if (pt[1].y() > center.y())
            pt[1].ry() = center.y();
        pt[3] = m_pins[3]->m_lastPos - (pt[1] - m_lastPos);
        pt[0].rx() = pt[3].x();
        pt[0].ry() = pt[1].y();
        pt[2].rx() = pt[1].x();
        pt[2].ry() = pt[3].y();
        break;
    case 2:
        if (pt[2].x() < center.x())
            pt[2].rx() = center.x();
        if (pt[2].y() < center.y())
            pt[2].ry() = center.y();
        pt[0] = m_pins[0]->m_lastPos - (pt[2] - m_lastPos);
        pt[1].rx() = pt[2].x();
        pt[1].ry() = pt[0].y();
        pt[3].rx() = pt[0].x();
        pt[3].ry() = pt[2].y();
        break;
    case 3:
        if (pt[3].x() > center.x())
            pt[3].rx() = center.x();
        if (pt[3].y() < center.y())
            pt[3].ry() = center.y();
        pt[1] = m_pins[1]->m_lastPos - (pt[3] - m_lastPos);
        pt[0].rx() = pt[3].x();
        pt[0].ry() = pt[1].y();
        pt[2].rx() = pt[1].x();
        pt[2].ry() = pt[3].y();
        break;
    }

    for (int i = 0; i < 4; ++i)
        m_pins[i]->setPos(pt[i]);
    App::project()->setPinsPos(pt);
}

void Pin::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        return;
    resetPos();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void Pin::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    for (int i = 0; i < 4; ++i)
        m_pins[i]->m_lastPos = m_pins[i]->pos();
    QGraphicsItem::mousePressEvent(event);
}
void Pin::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    auto action = menu.addAction(QIcon::fromTheme("drill-path"), tr("&Create path for Pins"), [] {
        ToolDatabase tdb(App::graphicsView(), { Tool::Drill, Tool::EndMill });
        if (tdb.exec()) {
            Tool tool(tdb.tool());

            QPolygonF dst;

            for (Pin* pin : m_pins) {
                pin->setFlag(QGraphicsItem::ItemIsMovable, false);
                QPointF point(pin->pos());
                if (dst.contains(point))
                    continue;
                if (App::project()->pinUsed(pin->m_index))
                    dst.push_back(point);
            }

            QSettings settings;
            settings.beginGroup("Pin");
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            bool ok;
            double depth = QInputDialog::getDouble(
                nullptr, //                             parent
                "", //                                  title
                tr("Set Depth"), //                     label
                settings.value("depth").toDouble(), //  value
                0, //                                   minValue
                20, //                                  maxValue
                1, //                                   decimals
                &ok, //                                 ok
                Qt::WindowFlags(), //                   flags
                1 //                                    step
            );

            if (!ok)
                return;
#else
            double depth = QInputDialog::getDouble(App::graphicsView(), "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 100, 2);
#endif
            if (depth == 0.0)
                return;

            settings.setValue("depth", depth);
            settings.endGroup();

            GCode::GCodeParams gcp(tool, depth, GCode::Drill);

            gcp.params[GCode::GCodeParams::NotTile];

            GCode::File* gcode = new GCode::File({ { dst } }, gcp);
            gcode->setFileName(tr("Pin_") + tool.nameEnc());
            App::project()->addFile(gcode);
        }
    });
    {
        action = menu.addAction(tr("Fixed"), [](bool fl) {
            for (Pin* pin : m_pins)
                pin->setFlag(QGraphicsItem::ItemIsMovable, !fl);
        });
        action->setCheckable(true);
        action->setChecked(!(m_pins[0]->flags() & QGraphicsItem::ItemIsMovable));
    }
    {
        action = menu.addAction(tr("Used"), [this](bool fl) { App::project()->setPinUsed(fl, m_index); update(); });
        action->setCheckable(true);
        action->setChecked(App::project()->pinUsed(m_index));
    }
    menu.addSeparator();
    action = menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Settings"), [] {
        SettingsDialog(nullptr, SettingsDialog::Utils).exec();
    });
    menu.exec(event->screenPos());
}

int Pin::type() const { return static_cast<int>(GiType::MarkPin); }

mvector<Pin*> Pin::pins() { return mvector { m_pins[0], m_pins[1], m_pins[2], m_pins[3] }; }

void Pin::setPinsPos(QPointF pos[])
{
    for (int i = 0; i < 4; ++i)
        m_pins[i]->setPos(pos[i]);
}

void Pin::resetPos(bool fl)
{
    if (fl)
        if (!updateRect())
            return;

    const QPointF offset(App::settings().mkrPinOffset());
    const QRectF rect(App::layoutFrames()->boundingRect()); //App::project()->worckRect()

    QPointF pt[] {
        QPointF(rect.topLeft() + QPointF(-offset.x(), -offset.y())),
        QPointF(rect.topRight() + QPointF(+offset.x(), -offset.y())),
        QPointF(rect.bottomRight() + QPointF(+offset.x(), +offset.y())),
        QPointF(rect.bottomLeft() + QPointF(-offset.x(), +offset.y())),
    };

    const QPointF center(rect.center());

    if (pt[0].x() > center.x())
        pt[0].rx() = center.x();
    if (pt[0].y() > center.y())
        pt[0].ry() = center.y();
    if (pt[1].x() < center.x())
        pt[1].rx() = center.x();
    if (pt[1].y() > center.y())
        pt[1].ry() = center.y();
    if (pt[2].x() < center.x())
        pt[2].rx() = center.x();
    if (pt[2].y() < center.y())
        pt[2].ry() = center.y();
    if (pt[3].x() > center.x())
        pt[3].rx() = center.x();
    if (pt[3].y() < center.y())
        pt[3].ry() = center.y();

    for (int i = 0; i < 4; ++i)
        m_pins[i]->setPos(pt[i]);

    App::project()->setPinsPos(pt);
}

void Pin::setPos(const QPointF pos[])
{
    for (int i = 0; i < 4; ++i)
        m_pins[i]->setPos(pos[i]);
}

void Pin::updateToolTip()
{
    const QPointF p(pos() - Marker::get(Marker::Zero)->pos());
    setToolTip(QObject::tr("Pin %1\nX %2:Y %3")
                   .arg(m_index + 1)
                   .arg(p.x())
                   .arg(p.y()));
}

void Pin::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    updateToolTip();
}

////////////////////////////////////////////////
LayoutFrames::LayoutFrames()
    : QGraphicsObject(nullptr)
{
    setZValue(-std::numeric_limits<double>::max());
    setFlag(ItemIsSelectable, false);
    App::setLayoutFrames(this);
}

LayoutFrames::~LayoutFrames()
{
    App::setLayoutFrames(nullptr);
}

int LayoutFrames::type() const
{
    return static_cast<int>(GiType::MarkLayoutFrames);
}

QRectF LayoutFrames::boundingRect() const
{
    if (App::scene()->drawPdf())
        return QRectF();
    return m_rect;
}

void LayoutFrames::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(Qt::NoBrush);

    QPen pen(QColor(255, 0, 255), 2.0 * App::graphicsView()->scaleFactor());
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);

    painter->drawPath(m_path);
    painter->restore();
}

void LayoutFrames::updateRect(bool fl)
{
    QPainterPath path;
    QRectF rect(App::project()->worckRect());
    m_rect = rect;

    const int stepsY(App::project()->stepsY()), stepsX(App::project()->stepsX());
    const double spaceY_(App::project()->spaceY()), spaceX_(App::project()->spaceX());

    m_rect.setHeight(m_rect.height() * stepsY + spaceY_ * (stepsY - 1));
    m_rect.setWidth(m_rect.width() * stepsX + spaceX_ * (stepsX - 1));

    for (int x = 0; x < stepsX; ++x) {
        for (int y = 0; y < stepsY; ++y) {
            if (x || y) {
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
    m_path = std::move(path);
    QGraphicsItem::update();
    if (fl) {
        Marker::get(Marker::Home)->resetPos(false);
        Marker::get(Marker::Zero)->resetPos(false);
        Pin::resetPos(false);
    }
}
