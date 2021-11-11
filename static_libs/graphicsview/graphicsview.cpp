// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "graphicsview.h"
#include "edid.h"
#include "ruler.h"
#include "scene.h"
#include "settings.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLWidget>
#include <QPropertyAnimation>
#else
#include <QtOpenGLWidgets/QOpenGLWidget>
#endif

#include <QDragEnterEvent>
#include <QGridLayout>
#include <QGuiApplication>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <cmath>

#include "leakdetector.h"

constexpr double zoomFactor = 1.5;
constexpr double zoomFactorAnim = 1.7;

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setCacheMode(/*CacheBackground*/ CacheNone);
    setOptimizationFlag(DontSavePainterState);
    setOptimizationFlag(DontAdjustForAntialiasing);

    { //set Cursor
        enum {
            Size = 21,
            Mid = 10
        };
        QPixmap cursor(Size, Size);
        cursor.fill(Qt::transparent);
        QPainter p(&cursor);
        p.setPen(QPen(QColor(App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF), 1.0));
        p.drawLine(0, Mid, Size, Mid);
        p.drawLine(Mid, 0, Mid, Size);
        setCursor(QCursor{ cursor, Mid, Mid });
    }

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    setOptimizationFlag(DontClipPainter);
#else
#endif

    setViewportUpdateMode(FullViewportUpdate);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    ////////////////////////////////////

    // add two rulers on top and left.
    setViewportMargins(Ruler::Breadth, 0, 0, Ruler::Breadth);

    // add grid layout
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);

    // create rulers
    hRuler = new Ruler(Ruler::Horizontal, this);
    vRuler = new Ruler(Ruler::Vertical, this);
    hRuler->SetMouseTrack(true);
    vRuler->SetMouseTrack(true);

    // add items to grid layout
    QPushButton* corner = new QPushButton(App::settings().inch() ? "I" : "M", this);
    connect(corner, &QPushButton::clicked, [this, corner](bool fl) {
        corner->setText(fl ? "I" : "M");
        App::settings().setInch(fl);
        scene()->update();
        hRuler->update();
        vRuler->update();
    });
    corner->setCheckable(true);
    corner->setFixedSize(Ruler::Breadth, Ruler::Breadth);

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
        setOpenGL(settings.value("chbxOpenGl").toBool());
        setRenderHint(QPainter::Antialiasing, settings.value("chbxAntialiasing", false).toBool());
        viewport()->setObjectName("viewport");
        settings.endGroup();
    }

    setScene(m_scene = new Scene(this));
    connect(this, &GraphicsView::mouseMove, m_scene, &Scene::setCross1);

    setStyleSheet("QGraphicsView { background: " + App::settings().guiColor(GuiColors::Background).name(QColor::HexRgb) + " }");
    App::setGraphicsView(this);
}

GraphicsView::~GraphicsView()
{
    App::setGraphicsView(nullptr);
}

void GraphicsView::setScene(QGraphicsScene* Scene)
{
    QGraphicsView::setScene(Scene);
    updateRuler();
}

void GraphicsView::zoomFit() { fitInView(scene()->itemsBoundingRect(), false); }

void GraphicsView::zoomToSelected()
{
    QRectF rect;
    for (const QGraphicsItem* item : scene()->selectedItems()) {
        const QRectF tmpRect(item->pos().isNull() ? item->boundingRect() : item->boundingRect().translated(item->pos()));
        rect = /*rect.isEmpty() ? tmpRect :*/ rect.united(tmpRect);
    }
    if (rect.isEmpty())
        return;
    fitInView(rect);
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

    if (App::settings().guiSmoothScSh()) {
        animate(this, "scale", getScale(), getScale() * zoomFactorAnim);
    } else {
        scale(zoomFactor, zoomFactor);
        updateRuler();
    }
}

void GraphicsView::zoomOut()
{
    if (getScale() < 1.0)
        return;
    if (App::settings().guiSmoothScSh()) {
        animate(this, "scale", getScale(), getScale() * (1.0 / zoomFactorAnim));
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        updateRuler();
    }
}

void GraphicsView::fitInView(QRectF dstRect, bool withBorders)
{
    if (dstRect.isNull())
        return;
    if (withBorders)
        dstRect += QMarginsF(5, 5, 5, 5); // 5 mm
    //    const auto r1(getViewRect().toRect());
    //    const auto r2(dstRect.toRect());
    //    if (r1 == r2)
    //        return;
    if (App::settings().guiSmoothScSh()) {
        animate(this, "viewRect", getViewRect(), dstRect);
    } else {
        QGraphicsView::fitInView(dstRect, Qt::KeepAspectRatio);
        updateRuler();
    }
}

double GraphicsView::scaleFactor()
{
    return 1.0 / getScale();
}

QPointF GraphicsView::mappedPos(QMouseEvent* event) const
{
    if (event->modifiers() & Qt::AltModifier || App::settings().snap()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        const double gs = App::settings().gridStep(matrix().m11());
#else
        const double gs = App::settings().gridStep(transform().m11());
#endif
        QPointF px(mapToScene(event->pos()) / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return mapToScene(event->pos());
}

void GraphicsView::setScale(double s) noexcept
{
    const auto trf(transform());
    setTransform({ +s /*11*/, trf.m12(), trf.m13(),
        /*      */ trf.m21(), -s /*22*/, trf.m23(),
        /*      */ trf.m31(), trf.m32(), trf.m33() });
}

double GraphicsView::getScale() noexcept { return transform().m11(); }

void GraphicsView::setOpenGL(bool useOpenGL)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setViewport(useOpenGL
            ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
            : new QWidget());
#else
    auto oglw = new QOpenGLWidget();
    QSurfaceFormat sf;
    sf.setSamples(8);
    oglw->setFormat(sf);
    setViewport(useOpenGL ? oglw : new QWidget);
#endif
}

void GraphicsView::setViewRect(QRectF r)
{
    QGraphicsView::fitInView(r, Qt::KeepAspectRatio);
}

QRectF GraphicsView::getViewRect()
{
    QPointF topLeft(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QPointF bottomRight(topLeft + viewport()->rect().bottomRight());

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QRectF visible_scene_rect(matrix().inverted().mapRect({ topLeft, bottomRight }));
#else
    QRectF visible_scene_rect(transform().inverted().mapRect({ topLeft, bottomRight }));
#endif

    return visible_scene_rect;
}

void GraphicsView::wheelEvent(QWheelEvent* event)
{
    const auto delta = event->angleDelta().y();

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    const auto pos = event->pos();
#else
    const auto pos = event->position().toPoint();
#endif

    static auto sbUpdate = [&delta, this, scale = 3](QScrollBar* sb) { // Warning if create more GraphicsView`s!!
        if (App::settings().guiSmoothScSh())
            animate(sb, "value", sb->value(), sb->value() - sb->pageStep() / (delta > 0 ? +scale : -scale));
        else
            sb->setValue(sb->value() - delta);
    };

    if (event->buttons() & Qt::RightButton) {
        if (abs(delta) == 120) {
            setInteractive(false);
            if (delta > 0)
                zoomIn();
            else
                zoomOut();
            setInteractive(true);
        }
    } else {
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
            if (!event->angleDelta().x())
                sbUpdate(QAbstractScrollArea::horizontalScrollBar());
            break;
        case Qt::NoModifier:
            if (!event->angleDelta().x())
                sbUpdate(QAbstractScrollArea::verticalScrollBar());
            break;
        default:
            //QGraphicsView::wheelEvent(event);
            return;
        }
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        setDragMode(ScrollHandDrag);
        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
#else
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->pos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        setDragMode(ScrollHandDrag);
        QMouseEvent fakeEvent(event->type(), event->pos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
#endif
    } else if (event->button() == Qt::RightButton) {
        //        { // удаление мостика
        //            QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), transform());
        //            if (item && static_cast<GiType>(item->type()) == GiType::Bridge && !static_cast<BridgeItem*>(item)->ok())
        //                delete item;
        //        }
        // это что бы при вызове контекстного меню ничего постороннего не было
        setDragMode(NoDrag);
        setInteractive(false);
        //Ruler
        m_scene->setDrawRuller(true);
        const QPointF point(mappedPos(event));
        m_scene->setCross2(point);
        emit mouseClickR(point);
    } else {
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        QGraphicsView::mousePressEvent(event);
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
#else
        QMouseEvent fakeEvent(event->type(), event->pos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
#endif
        QGraphicsView::mouseReleaseEvent(&fakeEvent);
        setDragMode(NoDrag);
        setInteractive(true);
    } else if (event->button() == Qt::RightButton) {
        // это что бы при вызове контекстного меню ничего постороннего не было
        QGraphicsView::mousePressEvent(event);
        setDragMode(RubberBandDrag);
        setInteractive(true);
        m_scene->setDrawRuller(false);
        emit mouseClickR(mappedPos(event));
        latPos = event->pos();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        emit mouseClickL(mappedPos(event));
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    vRuler->SetCursorPos(event->pos());
    hRuler->SetCursorPos(event->pos());
    const QPointF point(mappedPos(event));
    emit mouseMove(point);
    QGraphicsView::mouseMoveEvent(event);
}

template <class T>
void GraphicsView::animate(QObject* target, const QByteArray& propertyName, T begin, T end)
{
    auto* animation = new QPropertyAnimation(target, propertyName);
    connect(animation, &QPropertyAnimation::finished, [propertyName, end, this] {
        setProperty(propertyName, end);
        updateRuler();
    });
    if constexpr (std::is_same_v<T, QRectF>) {
        animation->setEasingCurve(QEasingCurve(QEasingCurve::InOutSine));
        animation->setDuration(200);
    } else if constexpr (std::is_same_v<decltype(target), QScrollBar>) {
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(50);
    } else {
        animation->setEasingCurve(QEasingCurve(QEasingCurve::InOutSine));
        animation->setDuration(100);
    }
    animation->setStartValue(begin);
    animation->setEndValue(end);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}
