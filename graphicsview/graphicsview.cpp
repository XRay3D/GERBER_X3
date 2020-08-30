// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
#include <mainwindow.h>
#include <settings.h>

#include <sh/circle.h>
#include <sh/constructor.h>

#include <forms/thermal/thermalmodel.h>
#include <forms/thermal/thermalpreviewitem.h>

const double zoomFactor = 1.5;

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    if (App::m_graphicsView) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class GraphicsView more than 2 times!!!");
        exit(1);
    }
    setCacheMode(/*CacheBackground*/ CacheNone);
    setOptimizationFlag(DontSavePainterState);
    setOptimizationFlag(DontAdjustForAntialiasing);

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    setOptimizationFlag(DontClipPainter);
#else
#endif

    setViewportUpdateMode(FullViewportUpdate);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    ////////////////////////////////////

    // add two rulers on top and left.
    setViewportMargins(RulerBreadth, 0, 0, RulerBreadth);

    // add grid layout
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);

    // create rulers
    hRuler = new QDRuler(QDRuler::Horizontal, this);
    vRuler = new QDRuler(QDRuler::Vertical, this);
    hRuler->SetMouseTrack(true);
    vRuler->SetMouseTrack(true);

    // add items to grid layout
    QPushButton* corner = new QPushButton(GlobalSettings::inch() ? "I" : "M", this);
    connect(corner, &QPushButton::clicked, [this, corner](bool fl) {
        corner->setText(fl ? "I" : "M");
        GlobalSettings::setInch(fl);
        scene()->update();
        hRuler->update();
        vRuler->update();
    });
    corner->setCheckable(true);
    corner->setFixedSize(RulerBreadth, RulerBreadth);

    gridLayout->addWidget(corner, 1, 0);
    gridLayout->addWidget(hRuler, 1, 1);
    gridLayout->addWidget(vRuler, 0, 0);
    gridLayout->addWidget(viewport(), 0, 1);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);

    scale(1.0, -1.0); //flip vertical
    zoom100();

    {
        QSettings settings;
        settings.beginGroup("Viewer");
        setViewport(settings.value("chbxOpenGl").toBool()
                ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
                : new QWidget);
        setRenderHint(QPainter::Antialiasing, settings.value("chbxAntialiasing", false).toBool());
        viewport()->setObjectName("viewport");
        settings.endGroup();
    }

    setScene(m_scene = new Scene(this));
    connect(this, &GraphicsView::mouseMove, m_scene, &Scene::setCross1);

    setStyleSheet("QGraphicsView { background: " + GlobalSettings::guiColor(Colors::Background).name(QColor::HexRgb) + " }");
    App::m_graphicsView = this;
}

GraphicsView::~GraphicsView()
{
    App::m_graphicsView = nullptr;
}

void GraphicsView::setScene(QGraphicsScene* Scene)
{
    QGraphicsView::setScene(Scene);
    updateRuler();
}

void GraphicsView::zoomFit()
{
    scene()->setSceneRect(scene()->itemsBoundingRect());
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    //scale(0.95, 0.95);
    updateRuler();
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
    if (GlobalSettings::guiSmoothScSh() && /* DISABLES CODE */ (0)) {
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
        //        123->anim(123, "sceneRect", 123->scene()->sceneRect(), rect);
        fitInView(rect, Qt::KeepAspectRatio);
        updateRuler();
    } else {
        const double k = 10 * scaleFactor();
        fitInView(rect + QMarginsF(k, k, k, k), Qt::KeepAspectRatio);
        updateRuler();
    }
}

void GraphicsView::zoom100()
{
    double x = 1.0, y = 1.0;
    const double m11 = QGraphicsView::transform().m11(), m22 = QGraphicsView::transform().m22();
    if (/* DISABLES CODE */ (0)) {
        x = qAbs(1.0 / m11 / (25.4 / physicalDpiX()));
        y = qAbs(1.0 / m22 / (25.4 / physicalDpiY()));
    } else {
        const QSizeF size(GetRealSize()); // size in mm
        const QRect scrGeometry(QGuiApplication::primaryScreen()->geometry()); // size in pix
        x = qAbs(1.0 / m11 / (size.height() / scrGeometry.height()));
        y = qAbs(1.0 / m22 / (size.width() / scrGeometry.width()));
    }
    scale(x, y);
    updateRuler();
}

void GraphicsView::zoomIn()
{

    if (getScale() > 10000.0)
        return;

    if (GlobalSettings::guiSmoothScSh()) {
        anim(this, "scale", getScale(), getScale() * zoomFactor);
    } else {
        scale(zoomFactor, zoomFactor);
        updateRuler();
    }
}

void GraphicsView::zoomOut()
{
    if (getScale() < 1.0)
        return;

    if (GlobalSettings::guiSmoothScSh()) {
        anim(this, "scale", getScale(), getScale() * (1.0 / zoomFactor));
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        updateRuler();
    }
}

double GraphicsView::scaleFactor()
{
    return 1.0 / getScale();
}

QPointF GraphicsView::mappedPos(QMouseEvent* event) const
{
    if (event->modifiers() & Qt::AltModifier || Shapes::Constructor::snap()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        const double gs = GlobalSettings::gridStep(matrix().m11());
#else
        const double gs = GlobalSettings::gridStep(transform().m11());
#endif
        QPointF px(mapToScene(event->pos()) / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return mapToScene(event->pos());
}

void GraphicsView::setScale(double s)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const auto trf = transform();
    setTransform({ +s /*11*/, trf.m12(), trf.m13(),
        /*      */ trf.m21(), -s /*22*/, trf.m23(),
        /*      */ trf.m31(), trf.m32(), trf.m33() });
#else
    setTransform(transform().scale(s, -s));
#endif
}

double GraphicsView::getScale()
{
    return transform().m11();
}

void GraphicsView::wheelEvent(QWheelEvent* event)
{
    const int scbarScale = 3;

    const auto delta = event->angleDelta().y();

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    const auto pos = event->pos();
#else
    const auto pos = event->position().toPoint();
#endif

    switch (event->modifiers()) {
    case Qt::ControlModifier:
        if (abs(delta) == 120) {
            setInteractive(false);
            if (delta > 0)
                zoomIn();
            else
                zoomOut();
            setInteractive(true);
        }
        break;
    case Qt::ShiftModifier:
        if (!event->angleDelta().x()) {
            auto scrollBar = QAbstractScrollArea::horizontalScrollBar();
            if (GlobalSettings::guiSmoothScSh()) {
                anim(scrollBar, "value", scrollBar->value(), scrollBar->value() - scrollBar->pageStep() / (delta > 0 ? scbarScale : -scbarScale));
            } else {
                scrollBar->setValue(scrollBar->value() - delta);
            }
        }
        break;
    case Qt::NoModifier:
        if (!event->angleDelta().x()) {
            auto scrollBar = QAbstractScrollArea::verticalScrollBar();
            if (GlobalSettings::guiSmoothScSh()) {
                anim(scrollBar, "value", scrollBar->value(), scrollBar->value() - scrollBar->pageStep() / (delta > 0 ? scbarScale : -scbarScale));
            } else {
                scrollBar->setValue(scrollBar->value() - delta);
            }
        } else {
            //   QAbstractScrollArea::horizontalScrollBar()->setValue(QAbstractScrollArea::horizontalScrollBar()->value() - (event->delta()));
        }
        break;
    default:
        //QGraphicsView::wheelEvent(event);
        return;
    }
    mouseMove(mapToScene(pos));
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, nullptr, event->modifiers());
#else
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
#endif
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
        Shapes::Constructor::finalizeShape();
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
        Shapes::Constructor::finalizeShape();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        Shapes::Constructor::addShapePoint(mappedPos(event));
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
    Shapes::Constructor::updateShape(point);
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
