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
Marker* Marker::m_markers[2] { nullptr, nullptr };

bool updateRect()
{
    QRectF rect(Project::instance()->getSelectedBoundingRect());
    if (rect.isEmpty()) {
        if (QMessageBox::question(nullptr, "",
                QObject::tr("There are no selected items to define the border.\n"
                            "The old border will be used."),
                QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::No)
            return false;
    }
    LayoutFrames::updateRect();
    return true;
}

Marker::Marker(Type type)
    : m_type(type)
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
    Scene::addItem(this);
}

Marker::~Marker()
{
    m_markers[m_type] = nullptr;
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

    QColor c(m_type == Home ? Settings::guiColor(Colors::Home) : Settings::guiColor(Colors::Zero));
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

int Marker::type() const { return m_type ? GiPointHome : GiPointZero; }

void Marker::resetPos(bool fl)
{
    if (fl)
        updateRect();

    const QRectF rect(LayoutFrames::instance()->boundingRect()); //Project::instance()->worckRect()

    if (m_type == Home)
        switch (Settings::mkrHomePos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + Settings::mkrHomeOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + Settings::mkrHomeOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + Settings::mkrHomeOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + Settings::mkrHomeOffset());
            break;
        default:
            setPos({});
            break;
        }
    else {
        switch (Settings::mkrZeroPos()) {
        case Qt::BottomLeftCorner:
            setPos(rect.topLeft() + Settings::mkrZeroOffset());
            break;
        case Qt::BottomRightCorner:
            setPos(rect.topRight() + Settings::mkrZeroOffset());
            break;
        case Qt::TopLeftCorner:
            setPos(rect.bottomLeft() + Settings::mkrZeroOffset());
            break;
        case Qt::TopRightCorner:
            setPos(rect.bottomRight() + Settings::mkrZeroOffset());
            break;
        default:
            setPos({});
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
    Project::instance()->setChanged();
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
/// \param parent
///
Pin::Pin(QObject* parent)
    : QObject(parent)
    , QGraphicsItem(nullptr)
    , m_index(m_pins.size())
{
    setObjectName("Pin");
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

    QColor c(Settings::guiColor(Colors::Pin));
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

    const QPointF center(LayoutFrames::instance()->boundingRect().center());
    //const QPointF center(Project::instance()->worckRect().center());

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

    Project::instance()->setChanged();
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

int Pin::type() const { return GiPin; }

QVector<Pin*> Pin::pins() { return m_pins; }

void Pin::resetPos(bool fl)
{
    if (fl)
        updateRect();

    const QPointF offset(Settings::mkrPinOffset());

    const QRectF rect(LayoutFrames::instance()->boundingRect()); //Project::instance()->worckRect()

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

    Project::instance()->setChanged();
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
LayoutFrames* LayoutFrames ::m_instance = nullptr;

LayoutFrames::LayoutFrames()
{
    if (m_instance) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class LayoutFrames more than 2 times!!!");
        exit(1);
    }
    setZValue(-std::numeric_limits<double>::max());
    m_instance = this;
    Scene::addItem(this);
}

LayoutFrames::~LayoutFrames()
{
    m_instance = nullptr;
}

int LayoutFrames::type() const
{
    return GiLayoutFrames;
}

QRectF LayoutFrames::boundingRect() const
{
    if (Scene::drawPdf())
        return QRectF();
    return m_path.boundingRect();
}

void LayoutFrames::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setBrush(Qt::NoBrush);
    QPen pen(QColor(100, 0, 100), 0.0);
    pen.setWidthF(2.0 * GraphicsView::scaleFactor());
    pen.setJoinStyle(Qt::MiterJoin);
    //    pen.setStyle(Qt::CustomDashLine);
    //    pen.setCapStyle(Qt::FlatCap);
    //    pen.setDashPattern({ 2.0, 5.0 });
    painter->setPen(pen);
    painter->drawPath(m_path);
    painter->restore();
}
LayoutFrames* LayoutFrames::instance() { return m_instance; }

void LayoutFrames::updateRect()
{
    if (!m_instance)
        return;
    QPainterPath path;
    QRectF rect(Project::instance()->worckRect());
    for (int x = 0; x < Project::instance()->stepsX(); ++x) {
        for (int y = 0; y < Project::instance()->stepsY(); ++y) {
            path.addRect(rect.translated(
                (rect.width() + Project::instance()->spasingX()) * x,
                (rect.height() + Project::instance()->spasingY()) * y));
        }
    }
    m_instance->m_path = path;
    m_instance->QGraphicsItem::update();
    m_instance->scene()->setSceneRect(m_instance->scene()->itemsBoundingRect());
    Marker::get(Marker::Home)->resetPos(false);
    Marker::get(Marker::Zero)->resetPos(false);
    Pin::resetPos(false);
}
