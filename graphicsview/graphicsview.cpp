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
#include <settings.h>

#include <sh/circle.h>
#include <sh/constructor.h>

#include <forms/thermal/thermalmodel.h>
#include <forms/thermal/thermalpreviewitem.h>

const double zoomFactor = 1.5;

GraphicsView* GraphicsView::self = nullptr;

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setCacheMode(/*CacheBackground*/ CacheNone);
    setOptimizationFlags(DontSavePainterState | DontClipPainter | DontAdjustForAntialiasing);
    setViewportUpdateMode(FullViewportUpdate /*SmartViewportUpdate*/ /*NoViewportUpdate*/);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    ////////////////////////////////////

    // add two rulers on top and left.
    setViewportMargins(RulerBreadth, 0, 0, RulerBreadth);

    // add grid layout
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    //gridLayout->setMargin(0);

    // create rulers
    hRuler = new QDRuler(QDRuler::Horizontal, this);
    vRuler = new QDRuler(QDRuler::Vertical, this);
    hRuler->SetMouseTrack(true);
    vRuler->SetMouseTrack(true);

    // add items to grid layout

    //QLabel* corner = new QLabel("<html><head/><body><p><span style=\" color:#ffffff;\">mm</span></p></body></html>", this);
    QPushButton* corner = new QPushButton(Settings::inch() ? "I" : "M", this);
    connect(corner, &QPushButton::clicked, [this, corner](bool fl) {
        corner->setText(fl ? "I" : "M");
        Settings::setInch(fl);
        scene()->update();
        hRuler->update();
        vRuler->update();
    });
    corner->setCheckable(true);
    //corner->setAlignment(Qt::AlignCenter);
    corner->setFixedSize(RulerBreadth, RulerBreadth);

    gridLayout->addWidget(corner, 1, 0);
    gridLayout->addWidget(hRuler, 1, 1);
    gridLayout->addWidget(vRuler, 0, 0);
    gridLayout->addWidget(viewport(), 0, 1);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);

    scale(1.0, -1.0); //flip vertical
    zoom100();
    QGLWidget* glw = nullptr;
    QSettings settings;
    settings.beginGroup(objectName());
    setViewport(settings.value("OpenGl").toBool()
            ? (glw = new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba)))
            : new QWidget);
    if (glw) {
    }

    setRenderHint(QPainter::Antialiasing, settings.value("Antialiasing", false).toBool());
    viewport()->setObjectName("viewport");
    settings.endGroup();

    m_scene = new Scene;
    setScene(m_scene);
    connect(this, &GraphicsView::mouseMove, m_scene, &Scene::setCross1);
    //    connect(this, &GraphicsView::mouseMove, hRuler, &QDRuler::setCross);
    //    connect(this, &GraphicsView::mouseMove, vRuler, &QDRuler::setCross);

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
    updateRuler();
}

void GraphicsView::zoomFit()
{
    self->scene()->setSceneRect(self->scene()->itemsBoundingRect());
    self->fitInView(self->scene()->sceneRect(), Qt::KeepAspectRatio);
    //scale(0.95, 0.95);
    self->updateRuler();
}

void GraphicsView::zoomToSelected()
{
    if (self == nullptr)
        return;
    QRectF rect;
    for (const QGraphicsItem* item : self->scene()->selectedItems()) {
        const QRectF tmpRect(item->pos().isNull() ? item->boundingRect() : item->boundingRect().translated(item->pos()));
        rect = rect.isEmpty() ? tmpRect : rect.united(tmpRect);
    }
    if (rect.isEmpty())
        return;

    const double k = 5.0 / self->transform().m11();
    rect += QMarginsF(k, k, k, k);
    if (Settings::smoothScSh() && /* DISABLES CODE */ (0)) {
        //        // Reset the view scale to 1:1.
        //        QRectF unity = d->matrix.mapRect(QRectF(0, 0, 1, 1));
        //        if (unity.isEmpty())
        //            return;
        //        scale(1 / unity.width(), 1 / unity.height());
        //        // Find the ideal x / y scaling ratio to fit \a rect in the view.
        //        int margin = 2;
        //        QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
        //        if (viewRect.isEmpty())
        //            return;
        //        QRectF sceneRect = d->matrix.mapRect(rect);
        //        if (sceneRect.isEmpty())
        //            return;
        //        qreal xratio = viewRect.width() / sceneRect.width();
        //        qreal yratio = viewRect.height() / sceneRect.height();
        //        // Respect the aspect ratio mode.
        //        switch (aspectRatioMode) {
        //        case Qt::KeepAspectRatio:
        //            xratio = yratio = qMin(xratio, yratio);
        //            break;
        //        case Qt::KeepAspectRatioByExpanding:
        //            xratio = yratio = qMax(xratio, yratio);
        //            break;
        //        case Qt::IgnoreAspectRatio:
        //            break;
        //        }
        //        // Scale and center on the center of \a rect.
        //        scale(xratio, yratio);
        //        centerOn(rect.center());
        //        self->anim(self, "sceneRect", self->scene()->sceneRect(), rect);
        self->fitInView(rect, Qt::KeepAspectRatio);
        self->updateRuler();
    } else {
        const double k = 10 * self->scaleFactor();
        self->fitInView(rect + QMarginsF(k, k, k, k), Qt::KeepAspectRatio);
        self->updateRuler();
    }
}

void GraphicsView::zoom100()
{
    if (self == nullptr)
        return;
    double x = 1.0, y = 1.0;
    const double m11 = self->QGraphicsView::transform().m11(), m22 = self->QGraphicsView::transform().m22();
    if (/* DISABLES CODE */ (0)) {
        x = qAbs(1.0 / m11 / (25.4 / self->physicalDpiX()));
        y = qAbs(1.0 / m22 / (25.4 / self->physicalDpiY()));
    } else {
        const QSizeF size(GetEdid()); // size in mm
        const QRect scrGeometry(QGuiApplication::primaryScreen()->geometry()); // size in pix
        x = qAbs(1.0 / m11 / (size.height() / scrGeometry.height()));
        y = qAbs(1.0 / m22 / (size.width() / scrGeometry.width()));
    }
    self->scale(x, y);
    self->updateRuler();
}

void GraphicsView::zoomIn()
{
    if (self == nullptr)
        return;
    if (getScale() > 10000.0)
        return;

    if (Settings::smoothScSh()) {
        self->anim(self, "scale", getScale(), getScale() * zoomFactor);
    } else {
        self->scale(zoomFactor, zoomFactor);
        self->updateRuler();
    }
}

void GraphicsView::zoomOut()
{
    if (self == nullptr)
        return;
    if (getScale() < 1.0)
        return;

    if (Settings::smoothScSh()) {
        self->anim(self, "scale", getScale(), getScale() * (1.0 / zoomFactor));
    } else {
        self->scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        self->updateRuler();
    }
}

double GraphicsView::scaleFactor()
{
    return 1.0 / getScale();
}

QPointF GraphicsView::mappedPos(QMouseEvent* event) const
{
    if (event->modifiers() & Qt::AltModifier || ShapePr::Constructor::snap()) {
        const double gs = Settings::gridStep(matrix().m11());
        QPointF px(mapToScene(event->pos()) / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return mapToScene(event->pos());
}

void GraphicsView::setScale(double s)
{
    const auto trf = transform();
    setTransform({ +s /*11*/, trf.m12(), trf.m13(),
        /*      */ trf.m21(), -s /*22*/, trf.m23(),
        /*      */ trf.m31(), trf.m32(), trf.m33() });
}

double GraphicsView::getScale()
{
    if (self == nullptr)
        return 1.0;
    return self->transform().m11();
}

void GraphicsView::wheelEvent(QWheelEvent* event)
{
    const int scbarScale = 3;
    switch (event->modifiers()) {
    case Qt::ControlModifier:
        if (abs(event->delta()) == 120) {
            setInteractive(false);
            if (event->delta() > 0)
                zoomIn();
            else
                zoomOut();
            setInteractive(true);
        }
        break;
    case Qt::ShiftModifier:
        if (!event->angleDelta().x()) {
            auto scrollBar = QAbstractScrollArea::horizontalScrollBar();
            if (Settings::smoothScSh()) {
                anim(scrollBar, "value", scrollBar->value(), scrollBar->value() - scrollBar->pageStep() / (event->delta() > 0 ? scbarScale : -scbarScale));
            } else {
                scrollBar->setValue(scrollBar->value() - event->delta());
            }
        }
        break;
    case Qt::NoModifier:
        if (!event->angleDelta().x()) {
            auto scrollBar = QAbstractScrollArea::verticalScrollBar();
            if (Settings::smoothScSh()) {
                anim(scrollBar, "value", scrollBar->value(), scrollBar->value() - scrollBar->pageStep() / (event->delta() > 0 ? scbarScale : -scbarScale));
            } else {
                scrollBar->setValue(scrollBar->value() - event->delta());
            }
        } else {
            //   QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        }
        break;
    default:
        //QGraphicsView::wheelEvent(event);
        return;
    }
    mouseMove(mapToScene(event->pos()));
    event->accept();
    update();
}

void GraphicsView::updateRuler()
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

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) { event->acceptProposedAction(); }

void GraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateRuler();
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
            if (item && item->type() == GiBridge && !static_cast<BridgeItem*>(item)->ok())
                delete item;
        }
        // это что бы при вызове контекстного меню ничего постороннего не было
        setDragMode(NoDrag);
        setInteractive(false);
        //Ruler
        m_scene->setDrawRuller(true);
        m_scene->setCross2(mappedPos(event));
        QGraphicsView::mousePressEvent(event);
    } else {
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        QGraphicsView::mousePressEvent(event);
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
        m_scene->setDrawRuller(false);
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        ShapePr::Constructor::addShapePoint(mappedPos(event));
    }
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto item = scene()->itemAt(mapToScene(event->pos()), transform()); //itemAt(event->pos());
    if (item && item->type() == GiThermalPr) {
        if (item->flags() & QGraphicsItem::ItemIsSelectable)
            reinterpret_cast<ThermalPreviewItem*>(item)->node()->disable();
        else
            reinterpret_cast<ThermalPreviewItem*>(item)->node()->enable();
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    vRuler->SetCursorPos(event->pos());
    hRuler->SetCursorPos(event->pos());
    const QPointF point(mappedPos(event));
    mouseMove(point);
    ShapePr::Constructor::updateShape(point);
    QGraphicsView::mouseMoveEvent(event);
}
class PropertyAnimation : public QPropertyAnimation {
public:
    PropertyAnimation(QObject* target, const QByteArray& propertyName, QObject* parent = nullptr)
        : QPropertyAnimation(target, propertyName, parent)
    {
    }
    ~PropertyAnimation()
    {
        qDebug() << "~PropertyAnimation()";
    }
};
template <class T>
void GraphicsView::anim(QObject* target, const QByteArray& propertyName, T begin, T end)
{
    auto* animation = new PropertyAnimation(target, propertyName);
    connect(animation, &QPropertyAnimation::finished, this, &GraphicsView::updateRuler);
    animation->setDuration(100);
    animation->setStartValue(begin);
    animation->setEndValue(end);
    animation->start();
    QTimer::singleShot(101, [animation] { delete animation; });
}
