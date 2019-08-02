#include "graphicsview.h"
#include "edid.h"
#include "qdruler.h"
#include "scene.h"
#include <QGLWidget>
#include <QSettings>
#include <QTimer>
#include <QTransform>
#include <QtWidgets>
#include <gi/bridgeitem.h>
//#include <gi/ruler.h>
#include <mainwindow.h>

#include <sh/circle.h>

#ifdef ANIM
enum {
    DURATION = 300,
    INTERVAL = 30
};
#endif

GraphicsView* GraphicsView::self = nullptr;

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setCacheMode(/*CacheBackground*/ CacheNone);
    setOptimizationFlags(DontSavePainterState | DontClipPainter | DontAdjustForAntialiasing);
    setViewportUpdateMode(FullViewportUpdate /*NoViewportUpdate*/);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    ////////////////////////////////////

    // add two rulers on top and left.
    setViewportMargins(RulerBreadth, 0, 0, RulerBreadth);

    // add grid layout
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    // create rulers
    hRuler = new QDRuler(QDRuler::Horizontal, this);
    vRuler = new QDRuler(QDRuler::Vertical, this);
    hRuler->SetMouseTrack(true);
    vRuler->SetMouseTrack(true);

    // add items to grid layout
    QLabel* corner = new QLabel("<html><head/><body><p><span style=\" color:#ffffff;\">mm</span></p></body></html>", this);
    corner->setAlignment(Qt::AlignCenter);
    corner->setFixedSize(RulerBreadth, RulerBreadth);

    gridLayout->addWidget(corner, 1, 0);
    gridLayout->addWidget(hRuler, 1, 1);
    gridLayout->addWidget(vRuler, 0, 0);
    gridLayout->addWidget(viewport(), 0, 1);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::UpdateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::UpdateRuler);

    scale(1.0, -1.0); //flip vertical
    zoom100();

    QSettings settings;
    settings.beginGroup("Viewer");
    setViewport(settings.value("OpenGl").toBool()
            ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
            : new QWidget);
    setRenderHint(QPainter::Antialiasing, settings.value("Antialiasing", false).toBool());
    viewport()->setObjectName("viewport");
    settings.endGroup();

    m_scene = new Scene;
    setScene(m_scene);
    connect(this, &GraphicsView::mouseMove, m_scene, &Scene::setCross);

    setStyleSheet("QGraphicsView { background: black }");

    //    QTimer* t = new QTimer(this);
    //    connect(t, &QTimer::timeout, [this] {
    //        qDebug("timeout");
    //        updateSceneRect(QRectF());
    //    });
    //    t->start(16);

    self = this;
}

GraphicsView::~GraphicsView()
{
    self = nullptr;
}

void GraphicsView::setScene(QGraphicsScene* Scene)
{
    QGraphicsView::setScene(Scene);
    UpdateRuler();
}

void GraphicsView::zoomFit()
{
    scene()->setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    //scale(0.95, 0.95);
    UpdateRuler();
}

void GraphicsView::zoomToSelected()
{
    QRectF rect;
    for (const QGraphicsItem* item : scene()->selectedItems()) {
        const QRectF tmpRect(item->pos().isNull() ? item->boundingRect() : item->boundingRect().translated(item->pos()));
        rect = rect.isEmpty() ? tmpRect : rect.united(tmpRect);
    }
    if (rect.isEmpty())
        return;

    const double k = 5.0 / transform().m11();
    rect += QMarginsF(k, k, k, k);
    fitInView(rect, Qt::KeepAspectRatio);

#ifdef ANIM
    numScheduledScalings -= 1;
    QTimeLine* anim = new QTimeLine(DURATION, this);
    anim->setUpdateInterval(INTERVAL);
    connect(anim, &QTimeLine::valueChanged, this, &GraphicsView::ScalingTime);
    connect(anim, &QTimeLine::finished, this, &GraphicsView::AnimFinished);
    anim->start();
#else
    //scale(0.9, 0.9);
    UpdateRuler();
#endif
}

void GraphicsView::zoom100()
{
    double x = 1.0, y = 1.0;
    if (0) {
        x = qAbs(1.0 / transform().m11() / (25.4 / physicalDpiX()));
        y = qAbs(1.0 / transform().m22() / (25.4 / physicalDpiY()));
    } else {
        QSizeF size(GetEdid()); // size in mm
        QRect scrGeometry = QGuiApplication::primaryScreen()->geometry(); // size in pix
        x = qAbs(1.0 / transform().m11() / (size.height() / scrGeometry.height()));
        y = qAbs(1.0 / transform().m22() / (size.width() / scrGeometry.width()));
    }
    scale(x, y);
    UpdateRuler();
}

void GraphicsView::zoomIn()
{
    if (transform().m11() > 10000.0)
        return;
#ifdef ANIM
    numScheduledScalings += 1;
    QTimeLine* anim = new QTimeLine(DURATION, this);
    anim->setUpdateInterval(INTERVAL);
    connect(anim, &QTimeLine::valueChanged, this, &GraphicsView::ScalingTime);
    connect(anim, &QTimeLine::finished, this, &GraphicsView::AnimFinished);
    anim->start();
#else
    scale(zoomFactor, zoomFactor);
    UpdateRuler();
#endif
}

void GraphicsView::zoomOut()
{
    if (transform().m11() < 1.0)
        return;
#ifdef ANIM
    numScheduledScalings -= 1;
    QTimeLine* anim = new QTimeLine(DURATION, this);
    anim->setUpdateInterval(INTERVAL);
    connect(anim, &QTimeLine::valueChanged, this, &GraphicsView::ScalingTime);
    connect(anim, &QTimeLine::finished, this, &GraphicsView::AnimFinished);
    anim->start();
#else
    scale(1.0 / zoomFactor, 1.0 / zoomFactor);
    UpdateRuler();
#endif
}

PrType GraphicsView::pt() const
{
    return m_pt;
}

void GraphicsView::setPt(const PrType& pt)
{
    m_pt = pt;
}

#ifdef ANIM
void GraphicsView::AnimFinished()
{
    if (numScheduledScalings > 0)
        numScheduledScalings--;
    else
        numScheduledScalings++;

    sender()->~QObject();
    UpdateRuler();
}

void GraphicsView::ScalingTime(qreal x)
{
    qreal factor = 1.0 + qreal(numScheduledScalings) / 100 * x;

    if (numScheduledScalings < 0 && factor > 10000.0)
        return;
    if (numScheduledScalings > 0 && factor < 1.0)
        return;

    scale(factor, factor);
}
#endif

void GraphicsView::wheelEvent(QWheelEvent* event)
{
    switch (event->modifiers()) {
    case Qt::ControlModifier:
        if (abs(event->delta()) == 120) {
            if (event->delta() > 0)
                zoomIn();
            else
                zoomOut();
        }
        break;
    case Qt::ShiftModifier:
        if (!event->angleDelta().x())
            QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        break;
    case Qt::NoModifier:
        if (!event->angleDelta().x())
            QAbstractScrollArea::verticalScrollBar()->setValue(QAbstractScrollArea::verticalScrollBar()->value() - (event->delta()));
        //            else
        //                QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        break;
    default:
        QGraphicsView::wheelEvent(event);
        return;
    }

    mouseMove(mapToScene(event->pos()));

    //        switch (event->modifiers()) {
    //        case Qt::ControlModifier:
    //            if (event->angleDelta().x() != 0)
    //                QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
    //            else
    //                QAbstractScrollArea::verticalScrollBar()->setValue(QAbstractScrollArea::verticalScrollBar()->value() - (event->delta()));
    //            break;
    //        case Qt::ShiftModifier:
    //            QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
    //            break;
    //        case Qt::NoModifier:
    //            if (abs(event->delta()) == 120) {
    //                if (event->delta() > 0)
    //                    zoomIn();
    //                else
    //                    zoomOut();
    //            }
    //            break;
    //        default:
    //            QGraphicsView::wheelEvent(event);
    //            return;
    //        }
    event->accept();
}

void GraphicsView::UpdateRuler()
{
    layout()->setContentsMargins(0, 0, 0, horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0);
    updateSceneRect(QRectF()); //actualize mapFromScene
    QPoint p = mapFromScene(QPointF());
    vRuler->SetOrigin(p.y());
    hRuler->SetOrigin(p.x());
    vRuler->SetRulerZoom(qAbs(transform().m22() * 0.1));
    hRuler->SetRulerZoom(qAbs(transform().m11() * 0.1));
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    event->ignore();
}

void GraphicsView::dropEvent(QDropEvent* event)
{
    for (QUrl& var : event->mimeData()->urls())
        emit fileDroped(var.path().remove(0, 1));
    event->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void GraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    UpdateRuler();
}

void GraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton) {
        setInteractive(false);
        // по нажатию средней кнопки мыши создаем событие ее отпускания выставляем моду перетаскивания и создаем событие зажатой левой кнопки мыши
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, nullptr, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        setDragMode(ScrollHandDrag);
        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
    } else if (event->button() == Qt::RightButton) {
        { // удаление мостика
            QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), transform());
            if (item && item->type() == BridgeType && !static_cast<BridgeItem*>(item)->ok())
                delete item;
        }
        // это что бы при вызове контекстного меню ничего постороннего не было
        setDragMode(NoDrag);
        setInteractive(false);
        //Ruler
        m_scene->setCross2(mapToScene(event->pos()));
        //ruller = new Ruler(mapToScene(event->pos()));
        //connect(this, &GraphicsView::mouseMove, ruller, &Ruler::setPoint2);
        //scene()->addItem(ruller);
        QGraphicsView::mousePressEvent(event);
    } else {
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        QGraphicsView::mousePressEvent(event);
        pt1 = mapToScene(event->pos());
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fakeEvent);
        setDragMode(RubberBandDrag);
        setInteractive(true);
    } else if (event->button() == Qt::RightButton) {
        // это что бы при вызове контекстного меню ничего постороннего не было
        QGraphicsView::mousePressEvent(event);
        setDragMode(RubberBandDrag);
        setInteractive(true);
        //Ruler
        //delete ruller;
        //ruller = nullptr;
        m_scene->setCross2(QPointF());
        m_scene->update();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        pt2 = mapToScene(event->pos());
        switch (m_pt) {
        case Rect: {
            QGraphicsRectItem* item = scene()->addRect(
                std::min(pt1.x(), pt2.x()),
                std::min(pt1.y(), pt2.y()),
                std::max(pt1.x(), pt2.x()) - std::min(pt1.x(), pt2.x()),
                std::max(pt1.y(), pt2.y()) - std::min(pt1.y(), pt2.y()),
                QPen(Qt::red, 0.0), QColor(255, 255, 255, 100));
            item->setFlags(
                QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            m_pt = NullPT;
        } break;
        case Line: {
            QGraphicsLineItem* item = scene()->addLine(QLineF(pt1, pt2), QPen(Qt::red, 0.0));
            item->setFlags(
                QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            m_pt = NullPT;
        } break;
        case Elipse: {
            auto* shapeItem = new ::Shape::Circle(pt1, pt2);
            scene()->addItem(shapeItem);
            shapeItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            m_pt = NullPT;
            scene()->setSceneRect(scene()->itemsBoundingRect());
        } break;
        case ArcPT: {
            QGraphicsEllipseItem* item = scene()->addEllipse(
                std::min(pt1.x(), pt2.x()),
                std::min(pt1.y(), pt2.y()),
                std::max(pt1.x(), pt2.x()) - std::min(pt1.x(), pt2.x()),
                std::max(pt1.y(), pt2.y()) - std::min(pt1.y(), pt2.y()),
                QPen(Qt::red, 0.0), QColor(255, 255, 255, 100));
            item->setStartAngle(0);
            item->setSpanAngle(123);
            item->setFlags(
                QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            m_pt = NullPT;
        } break;
        case Text: {
            QGraphicsTextItem* item = scene()->addText("QGraphicsTextItem");
            // QPen(Qt::red, 0.0), QColor(255, 255, 255, 100));
            item->setDefaultTextColor(QColor(255, 255, 255, 100));
            item->setMatrix(QMatrix().scale(1, -1));
            item->setPos(pt1);
            item->setFlags(
                QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
            m_pt = NullPT;
        } break;
        default:
            break;
        }
    }
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event) { QGraphicsView::mouseDoubleClickEvent(event); }

void GraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    vRuler->SetCursorPos(event->pos());
    hRuler->SetCursorPos(event->pos());
    mouseMove(mapToScene(event->pos() + QPoint(1, 1)));

    QGraphicsView::mouseMoveEvent(event);
}
