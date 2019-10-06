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

Marker::Marker(int type)
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

Marker::~Marker()
{
    QSettings settings;
    settings.beginGroup("Point");
    settings.setValue("pos" + QString::number(m_type), pos());
    settings.setValue("fixed", bool(flags() & QGraphicsItem::ItemIsMovable));
}

QRectF Marker::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_rect;
}

void Marker::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (Scene::drawPdf())
        return;

    QColor c(m_type == Home ? Settings::color(Colors::Home) : Settings::color(Colors::Zero));
    if (option->state & QStyle ::State_MouseOver)
        c.setAlpha(255);
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
    if (Scene::drawPdf())
        return QPainterPath();
    return /*m_shape*/ m_path;
}

int Marker::type() const { return m_type ? PointHomeType : PointZeroType; }

void Marker::resetPos(bool fl)
{
    if (fl)
        updateRect();
    if (m_type == Home)
        switch (Settings::homePos()) {
        case Qt::BottomLeftCorner:
            setPos(Pin::worckRect.topLeft() + Settings::homeOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(Pin::worckRect.topRight() + Settings::homeOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(Pin::worckRect.bottomLeft() + Settings::homeOffset());
            break;
        case Qt::TopRightCorner:
            setPos(Pin::worckRect.bottomRight() + Settings::homeOffset());
            break;
        default:
            break;
        }
    else {
        switch (Settings::zeroPos()) {
        case Qt::BottomLeftCorner:
            setPos(Pin::worckRect.topLeft() + Settings::zeroOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(Pin::worckRect.topRight() + Settings::zeroOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(Pin::worckRect.bottomLeft() + Settings::zeroOffset());
            break;
        case Qt::TopRightCorner:
            setPos(Pin::worckRect.bottomRight() + Settings::zeroOffset());
            break;
        default:
            break;
        }
    }
    updateGCPForm();
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
}

void Marker::updateGCPForm()
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

////////////////////////////////////////////////
/// \brief Pin::Pin
/// \param type
/// \param num
///

Pin::Pin()
    : QGraphicsItem(nullptr)
    , m_index(m_pins.size())
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
    QSettings settings;
    settings.beginGroup("Pin");
    settings.setValue("pos" + QString::number(m_index), pos());
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
    if (Scene::drawPdf())
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

    const QPointF center(worckRect.center());

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

int Pin::type() const { return PinType; }

QVector<Pin*> Pin::pins() { return m_pins; }

void Pin::resetPos(bool fl)
{
    if (fl) {
        updateRect();
    }
    const QPointF offset(Settings::pinOffset());
    QPointF pt[] {
        QPointF(Pin::worckRect.topLeft() + QPointF(-offset.x(), -offset.y())),
        QPointF(Pin::worckRect.topRight() + QPointF(+offset.x(), -offset.y())),
        QPointF(Pin::worckRect.bottomRight() + QPointF(+offset.x(), +offset.y())),
        QPointF(Pin::worckRect.bottomLeft() + QPointF(-offset.x(), +offset.y())),
    };

    const QPointF center(worckRect.center());

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

    QSettings settings;
    settings.beginGroup("Pin");
    settings.setValue("pos" + QString::number(m_pins.indexOf(this)), pos());
    Project::setChanged();
}

void Pin::updateToolTip()
{
    const QPointF p(pos() - GCodePropertiesForm::zeroPoint->pos());
    setToolTip(QObject::tr("Pin %1\nX %2:Y %3").arg(m_pins.indexOf(this) + 1).arg(p.x()).arg(p.y()));
}

void Pin::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    updateToolTip();
}
