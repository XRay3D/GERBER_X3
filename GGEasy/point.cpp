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
#include "point.h"
#include "project.h"

#include "gcode/gcode.h"

#include "forms/gcodepropertiesform.h"
#include "graphicsview.h"
#include "project.h"
#include "settings.h"
#include "tooldatabase/tooldatabase.h"
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "leakdetector.h"
#include "settingsdialog.h"

using namespace ClipperLib;

QVector<Pin*> Pin::m_pins;
Marker* Marker::m_markers[2] { nullptr, nullptr };

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
    App::scene()->addItem(this);
}

Marker::~Marker()
{
    m_markers[m_type] = nullptr;
}

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

    QColor c(m_type == Home ? AppSettings::guiColor(Colors::Home) : AppSettings::guiColor(Colors::Zero));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(200);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Marker::shape() const
{
    if (App::scene()->drawPdf())
        return QPainterPath();
    return /*m_shape*/ m_path;
}

int Marker::type() const { return static_cast<int>(m_type ? GiType::Home : GiType::PointZero); }

void Marker::resetPos(bool flUpdateRect)
{
    if (flUpdateRect && !updateRect())
        return;

    const QRectF rect(App::layoutFrames()->boundingRect());

    if (m_type == Home)
        switch (AppSettings::mkrHomePos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + AppSettings::mkrHomeOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + AppSettings::mkrHomeOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + AppSettings::mkrHomeOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + AppSettings::mkrHomeOffset());
            break;
        default:
            setPos({});
            break;
        }
    else {
        switch (AppSettings::mkrZeroPos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + AppSettings::mkrZeroOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + AppSettings::mkrZeroOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + AppSettings::mkrZeroOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + AppSettings::mkrZeroOffset());
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
    QSettings settings;
    settings.beginGroup("Points");
    settings.setValue("pos" + QString::number(m_type), pos());
    App::project()->setChanged();
    if (m_type == Zero)
        for (Pin* pin : Pin::pins())
            pin->updateToolTip();
}

void Marker::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
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
    , m_index(m_pins.size())
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
    App::scene()->addItem(this);
    m_pins.append(this);
}

Pin::~Pin()
{
}

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

    QColor c(AppSettings::guiColor(Colors::Pin));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(200);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(static_cast<int>(c.alpha() * 0.5));
    //c.setAlpha(50);

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

            for (Pin* item : Pin::pins()) {
                item->setFlag(QGraphicsItem::ItemIsMovable, false);
                QPointF point(item->pos());
                if (dst.contains(point))
                    continue;
                dst.append(point);
            }

            QSettings settings;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            bool ok;
            double depth = QInputDialog::getDouble(nullptr, "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 20, 1, &ok, Qt::WindowFlags(), 1);
            if (!ok)
                return;
#else
            double depth = QInputDialog::getDouble(App::graphicsView(), "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 100, 2);
#endif

            if (depth == 0.0)
                return;
            settings.setValue("Pin/depth", depth);

            GCode::GCodeParams gcp(tool, depth, GCode::Drill);

            gcp.params[GCode::GCodeParams::NotTile];

            GCode::File* gcode = new GCode::File({ { dst } }, gcp);
            gcode->setFileName(tr("Pin_") + tool.nameEnc());
            App::project()->addFile(gcode);
        }
    });
    action = menu.addAction(tr("Fixed"), [](bool fl) {
        for (Pin* pin : Pin::pins())
            pin->setFlag(QGraphicsItem::ItemIsMovable, !fl);
    });
    action->setCheckable(true);
    action->setChecked(!(Pin::pins()[0]->flags() & QGraphicsItem::ItemIsMovable));
    menu.addSeparator();
    action = menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Settings"), [] {
        SettingsDialog(nullptr, SettingsDialog::Utils).exec();
    });
    menu.exec(event->screenPos());
}

int Pin::type() const
{
    return static_cast<int>(GiType::Pin);
}

QVector<Pin*> Pin::pins()
{
    return m_pins;
}

void Pin::resetPos(bool fl)
{
    if (fl)
        if (!updateRect())
            return;

    const QPointF offset(AppSettings::mkrPinOffset());
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

void Pin::updateToolTip()
{
    const QPointF p(pos() - Marker::get(Marker::Zero)->pos());
    setToolTip(QObject::tr("Pin %1\nX %2:Y %3").arg(m_pins.indexOf(this) + 1).arg(p.x()).arg(p.y()));
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
    App::m_app->m_layoutFrames = this;
    App::scene()->addItem(this);
    setFlag(ItemIsSelectable, false);
}

LayoutFrames::~LayoutFrames()
{
    App::m_app->m_layoutFrames = nullptr;
}

int LayoutFrames::type() const
{
    return static_cast<int>(GiType::LayoutFrames);
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
    {
        QPen pen(QColor(255, 0, 255), 2.0 * App::graphicsView()->scaleFactor());
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
    }
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
                    const double len = std::min(rect.height() / 10, spaceX);

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
                    const double len = std::min(rect.width() / 10, spaceY);

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
    scene()->setSceneRect(scene()->itemsBoundingRect());
    if (fl) {
        Marker::get(Marker::Home)->resetPos(false);
        Marker::get(Marker::Zero)->resetPos(false);
        Pin::resetPos(false);
    }
}
