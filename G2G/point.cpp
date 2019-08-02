#include "point.h"
#include "forms/gcodepropertiesform.h"
#include "gi/graphicsitem.h"
#include "mainwindow.h"
#include "project.h"
#include "settings.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSettings>
#include <clipper.hpp>

using namespace ClipperLib;

QVector<Pin*> Pin::m_pins;
QRectF Pin::worckRect;

bool updateRect()
{
    QRectF rect(Project::getSelectedBoundingRect());
    if (rect.isEmpty()) {
        if (QMessageBox::question(nullptr, "", QObject::tr("There is no dedicated data to define boundaries.\nOld data will be used."), QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::No)
            return false;
        return true;
    }
    Pin::worckRect = rect;
    return true;
}

Point::Point(int type)
    : m_type(type)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
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

    QSettings settings;
    settings.beginGroup("Point");
    setPos(settings.value("pos" + QString::number(m_type)).toPointF());
    setFlag(QGraphicsItem::ItemIsMovable, settings.value("fixed").toBool());
    Scene::addItem(this);
}

Point::~Point()
{
    QSettings settings;
    settings.beginGroup("Point");
    settings.setValue("pos" + QString::number(m_type), pos());
    settings.setValue("fixed", bool(flags() & QGraphicsItem::ItemIsMovable));
}

QRectF Point::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_rect;
}

void Point::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf())
        return;

    QColor c(m_type == Home ? Settings::color(Colors::Home) : Settings::color(Colors::Zero));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(50);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Point::shape() const
{
    if (Scene::drawPdf())
        return QPainterPath();

    return m_shape;
}

void Point::resetPos(bool fl)
{
    if (fl) {
        updateRect();
    }
    if (m_type == Home)
        switch (Settings::homePos()) {
        case HomePosition::BottomLeft:
            setPos(Pin::worckRect.topLeft() + Settings::homeOffset());
            break;
        case HomePosition::BottomRight:
            setPos(Pin::worckRect.topRight() + Settings::homeOffset());
            break;
        case HomePosition::TopLeft:
            setPos(Pin::worckRect.bottomLeft() + Settings::homeOffset());
            break;
        case HomePosition::TopRight:
            setPos(Pin::worckRect.bottomRight() + Settings::homeOffset());
            break;
        default:
            break;
        }
    else {
        setPos(Pin::worckRect.topLeft());
    }
    updateGCPForm();
}

void Point::setPosX(double x)
{
    QPointF point(pos());
    if (point.x() == x)
        return;
    point.setX(x);
    setPos(point);
}

void Point::setPosY(double y)
{
    QPointF point(pos());
    if (point.y() == y)
        return;
    point.setY(y);
    setPos(point);
}

void Point::updateGCPForm()
{
    GCodePropertiesForm::updatePosDsbxs();
    QSettings settings;
    settings.beginGroup("Points");
    settings.setValue("pos" + QString::number(m_type), pos());
    Project::setChanged();
    if (m_type == Zero)
        for (Pin* pin : Pin::pins())
            pin->updateToolTip();
}

void Point::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    updateGCPForm();
}

void Point::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
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

////////////////////////////////////////////////
/// \brief Pin::Pin
/// \param type
/// \param num
///

Pin::Pin()
    : QGraphicsItem(nullptr)
{
    setAcceptHoverEvents(true);

    if (m_pins.size() % 2) {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 0, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 270, -90);
    } else {
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 90, 90);
        m_path.arcTo(QRectF(QPointF(-3, -3), QSizeF(6, 6)), 360, -90);
    }
    m_shape.addEllipse(QRectF(QPointF(-3, -3), QSizeF(6, 6)));
    m_rect = m_path.boundingRect();

    setZValue(std::numeric_limits<qreal>::max() - m_pins.size());

    Scene::addItem(this);

    QSettings settings;
    settings.beginGroup("Pin");
    setFlag(QGraphicsItem::ItemIsMovable, settings.value("fixed").toBool());
    setPos(settings.value("pos" + QString::number(m_pins.size())).toPointF());
    if (m_pins.isEmpty())
        worckRect = settings.value("worckRect").toRectF();
    m_pins.append(this);
}

Pin::~Pin()
{
}

QRectF Pin::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_rect;
}

void Pin::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf())
        return;

    QColor c(Settings::color(Colors::Pin));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        c.setAlpha(50);

    painter->setPen(Qt::NoPen);
    painter->setBrush(c);
    painter->drawPath(m_path);
    painter->setPen(c);
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QPoint(0, 0), 2, 2);
}

QPainterPath Pin::shape() const
{
    if (Scene::drawPdf())
        return QPainterPath();
    return m_shape;
}

void Pin::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    QPointF p[4]{
        m_pins[0]->pos(),
        m_pins[1]->pos(),
        m_pins[2]->pos(),
        m_pins[3]->pos()
    };

    switch (m_pins.indexOf(this)) {
    case 0:
        if (p[0].x() > Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
            p[0].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
        if (p[0].y() > Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
            p[0].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;
        p[2] = m_pins[2]->m_lastPos - (p[0] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 1:
        if (p[1].x() < Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
            p[1].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
        if (p[1].y() > Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
            p[1].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;
        p[3] = m_pins[3]->m_lastPos - (p[1] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    case 2:
        if (p[2].x() < Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
            p[2].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
        if (p[2].y() < Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
            p[2].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;
        p[0] = m_pins[0]->m_lastPos - (p[2] - m_lastPos);
        p[1].rx() = p[2].x();
        p[1].ry() = p[0].y();
        p[3].rx() = p[0].x();
        p[3].ry() = p[2].y();
        break;
    case 3:
        if (p[3].x() > Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
            p[3].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
        if (p[3].y() < Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
            p[3].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;
        p[1] = m_pins[1]->m_lastPos - (p[3] - m_lastPos);
        p[0].rx() = p[3].x();
        p[0].ry() = p[1].y();
        p[2].rx() = p[1].x();
        p[2].ry() = p[3].y();
        break;
    }
    for (int i = 0; i < 4; ++i) {
        m_pins[i]->setPos(p[i]);
    }
    QSettings settings;
    settings.beginGroup("Pin");
    settings.setValue("pos" + QString::number(m_pins.indexOf(this)), pos());
    Project::setChanged();
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

void Pin::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) { QGraphicsItem::mouseReleaseEvent(event); }

int Point::type() const { return m_type ? PointHomeType : PointZeroType; }

int Pin::type() const { return PinType; }

QVector<Pin*> Pin::pins() { return m_pins; }

void Pin::resetPos(bool fl)
{
    if (fl) {
        updateRect();
    }
    const QPointF o(Settings::pinOffset());
    QPointF p[]{
        QPointF(Pin::worckRect.topLeft() + QPointF(-o.x(), -o.y())),
        QPointF(Pin::worckRect.topRight() + QPointF(+o.x(), -o.y())),
        QPointF(Pin::worckRect.bottomRight() + QPointF(+o.x(), +o.y())),
        QPointF(Pin::worckRect.bottomLeft() + QPointF(-o.x(), +o.y())),
    };

    if (p[0].x() > Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
        p[0].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
    if (p[0].y() > Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
        p[0].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;

    if (p[1].x() < Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
        p[1].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
    if (p[1].y() > Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
        p[1].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;

    if (p[2].x() < Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
        p[2].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
    if (p[2].y() < Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
        p[2].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;

    if (p[3].x() > Pin::worckRect.left() + Pin::worckRect.width() * 0.5)
        p[3].rx() = Pin::worckRect.left() + Pin::worckRect.width() * 0.5;
    if (p[3].y() < Pin::worckRect.top() + Pin::worckRect.height() * 0.5)
        p[3].ry() = Pin::worckRect.top() + Pin::worckRect.height() * 0.5;

    for (int i = 0; i < 4; ++i) {
        m_pins[i]->setPos(p[i]);
    }
    QSettings settings;
    settings.beginGroup("Pin");
    settings.setValue("pos" + QString::number(m_pins.indexOf(this)), pos());
    Project::setChanged();
}

void Pin::updateToolTip()
{
    QPointF p(pos());
    p -= GCodePropertiesForm::zeroPoint->pos();
    setToolTip(QObject::tr("Pin %1\nX %2:Y %3").arg(m_pins.indexOf(this) + 1).arg(p.x()).arg(p.y()));
}

void Pin::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    updateToolTip();
}
