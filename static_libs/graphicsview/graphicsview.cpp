// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/

// import std;

#include "graphicsview.h"
#include "edid.h"
#include "myclipper.h"
#include "project.h"
#include "ruler.h"
#include "settings.h"
#include "utils.h"

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

constexpr double zoomFactor = 1.5;
constexpr double zoomFactorAnim = 1.7;

void setCursor(QWidget* w) {
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
    w->setCursor(QCursor {cursor, Mid, Mid});
}

static int rulCtr;

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , hRuler {new Ruler(Ruler::Horizontal, this)}
    , vRuler {new Ruler(Ruler::Vertical, this)}
    , gridLayout {new QGridLayout(this)} {

    setCacheMode(CacheBackground);
    setOptimizationFlag(DontSavePainterState);
    setOptimizationFlag(DontAdjustForAntialiasing);
    setViewportUpdateMode(SmartViewportUpdate);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    ////////////////////////////////////
    setScene(new QGraphicsScene(this));

    scene()->setSceneRect(-1000, -1000, +2000, +2000);

    // add two rulers on top and left.
    setViewportMargins(Ruler::Breadth, 0, 0, Ruler::Breadth);

    // create rulers

    hRuler->SetMouseTrack(true);
    vRuler->SetMouseTrack(true);
    ::setCursor(hRuler);
    ::setCursor(vRuler);

    // add items to grid layout
    QPushButton* corner = new QPushButton(App::settings().inch() ? "I" : "M", this);
    connect(corner, &QPushButton::clicked, [corner, this](bool fl) {
        corner->setText(fl ? "I" : "M");
        App::settings().setInch(fl);
        scene()->update();
        hRuler->update();
        vRuler->update();
    });
    corner->setCheckable(true);
    corner->setFixedSize(Ruler::Breadth, Ruler::Breadth);

    // add grid layout
    gridLayout->setSpacing({});
    gridLayout->setContentsMargins({});
    gridLayout->addWidget(corner, 1, 0);
    gridLayout->addWidget(hRuler, 1, 1);
    gridLayout->addWidget(vRuler, 0, 0);
    gridLayout->addWidget(viewport(), 0, 1);
    gridLayout->addWidget(horizontalScrollBar(), 2, 1);
    gridLayout->addWidget(verticalScrollBar(), 0, 2);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::updateRuler);

    scale(1.0, -1.0); // flip vertical
    zoom100();

    {
        QSettings settings;
        settings.beginGroup("Viewer");
        setOpenGL(settings.value("chbxOpenGl").toBool());
        setRenderHint(QPainter::Antialiasing, settings.value("chbxAntialiasing", false).toBool());
        viewport()->setObjectName("viewport");
        settings.endGroup();
    }
    //    auto scene_ = new QGraphicsScene(this);

    //    setScene(scene_ = new Scene(this));
    //    connect(this, &GraphicsView::mouseMove, scene_, &Scene::setCross1);

    setStyleSheet("QGraphicsView { background: " + App::settings().guiColor(GuiColors::Background).name(QColor::HexRgb) + " }");
    App::setGraphicsView(this);
}

GraphicsView::~GraphicsView() {
    App::setGraphicsView(nullptr);
}

// void GraphicsView::setScene(QGraphicsScene* Scene) {
//     QGraphicsView::setScene(Scene);
//     updateRuler();
// }

void GraphicsView::zoomFit() { fitInView(scene()->itemsBoundingRect(), false); }

void GraphicsView::zoomToSelected() {
    QRectF rect;
    for (const QGraphicsItem* item : scene()->selectedItems()) {
        const QRectF tmpRect(item->pos().isNull() ? item->boundingRect() : item->boundingRect().translated(item->pos()));
        rect = /*rect.isEmpty() ? tmpRect :*/ rect.united(tmpRect);
    }
    if (rect.isEmpty())
        return;
    fitInView(rect);
}

void GraphicsView::zoom100() {
    double x = 1.0, y = 1.0;
    const double m11 = QGraphicsView::transform().m11(), m22 = QGraphicsView::transform().m22();
    if (/* DISABLES CODE */ (0)) {
        x = qAbs(1.0 / m11 / (25.4 / physicalDpiX()));
        y = qAbs(1.0 / m22 / (25.4 / physicalDpiY()));
    } else {
        const QSizeF size(GetRealSize());                                      // size in mm
        const QRect scrGeometry(QGuiApplication::primaryScreen()->geometry()); // size in pix
        x = qAbs(1.0 / m11 / (size.height() / scrGeometry.height()));
        y = qAbs(1.0 / m22 / (size.width() / scrGeometry.width()));
    }

    if (0 && App::settings().guiSmoothScSh()) {
        animate(this, "scale", getScale(), x * zoomFactorAnim);
    } else {
        scale(x, y);
        updateRuler();
    }
}

void GraphicsView::zoomIn() {
    if (getScale() > 10000.0)
        return;

    if (App::settings().guiSmoothScSh()) {
        animate(this, "scale", getScale(), getScale() * zoomFactorAnim);
    } else {
        scale(zoomFactor, zoomFactor);
        updateRuler();
    }
}

void GraphicsView::zoomOut() {
    if (getScale() < 1.0)
        return;
    if (App::settings().guiSmoothScSh()) {
        animate(this, "scale", getScale(), getScale() * (1.0 / zoomFactorAnim));
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        updateRuler();
    }
}

void GraphicsView::fitInView(QRectF dstRect, bool withBorders) {
    if (dstRect.isNull())
        return;
    if (withBorders)
        dstRect += QMarginsF(dstRect.width() / 5, dstRect.height() / 5, dstRect.width() / 5, dstRect.height() / 5); // 5 mm
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

void GraphicsView::setRuler(bool ruller) {
    rulCtr = 0;
    ruler_ = ruller;
    scene()->update();
}

double GraphicsView::scaleFactor() {
    return 1.0 / getScale();
}

QPointF GraphicsView::mappedPos(QMouseEvent* event) const {
    if (event->modifiers() & Qt::AltModifier || App::settings().snap()) {
        const double gs = App::settings().gridStep(transform().m11());
        QPointF px(mapToScene(event->pos()) / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return mapToScene(event->pos());
}

void GraphicsView::setScale(double s) noexcept {
    const auto trf(transform());
    setTransform({+s /*11*/, trf.m12(), trf.m13(),
        /*      */ trf.m21(), -s /*22*/, trf.m23(),
        /*      */ trf.m31(), trf.m32(), trf.m33()});
}

double GraphicsView::getScale() noexcept { return transform().m11(); }

void GraphicsView::setOpenGL(bool useOpenGL) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setViewport(useOpenGL ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba)) : new QWidget());
#else
    auto oglw = new QOpenGLWidget();
    QSurfaceFormat sf;
    sf.setSamples(8);
    oglw->setFormat(sf);
    setViewport(useOpenGL ? oglw : new QWidget);
#endif
    ::setCursor(viewport());
    gridLayout->addWidget(viewport(), 0, 1);
}

void GraphicsView::setViewRect(const QRectF& r) {
    QGraphicsView::fitInView(r, Qt::KeepAspectRatio);
}

QRectF GraphicsView::getViewRect() {
    QPointF topLeft(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QPointF bottomRight(topLeft + viewport()->rect().bottomRight());

    QRectF visible_scene_rect(transform().inverted().mapRect({topLeft, bottomRight}));

    return visible_scene_rect;
}

QRectF GraphicsView::getSelectedBoundingRect() {
    auto selectedItems {scene()->selectedItems()};

    if (selectedItems.isEmpty())
        return {};

    QRectF rect;

    {
        ScopedTrue sTrue(boundingRect_);
        rect = selectedItems.front()->boundingRect();
        for (auto gi : selectedItems)
            rect = rect.united(gi->boundingRect());
    }

    if (!rect.isEmpty())
        App::project()->setWorckRect(rect);

    return rect;
}

void GraphicsView::wheelEvent(QWheelEvent* event) {
    const auto delta = event->angleDelta().y();

    const auto pos = event->position().toPoint();

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
            // QGraphicsView::wheelEvent(event);
            return;
        }
    }
    mouseMove(mapToScene(pos));
    event->accept();
    update();
}

void GraphicsView::updateRuler() {
    // layout()->setContentsMargins(0, 0, 0, horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0);
    updateSceneRect(QRectF()); // actualize mapFromScene
    QPoint p = mapFromScene(QPointF());
    vRuler->SetOrigin(p.y());
    hRuler->SetOrigin(p.x());
    vRuler->SetRulerZoom(qAbs(transform().m22() * 0.1));
    hRuler->SetRulerZoom(qAbs(transform().m11() * 0.1));
}

void GraphicsView::drawRuller(QPainter* painter, const QRectF& rect_) {
    const QPointF pt2(rulPt2);
    const QPointF pt1(rulPt1);
    QLineF line(pt2, pt1);
    const QRectF rect(
        QPointF(qMin(pt1.x(), pt2.x()), qMin(pt1.y(), pt2.y())),
        QPointF(std::max(pt1.x(), pt2.x()), std::max(pt1.y(), pt2.y())));
    const double length = line.length();
    const double angle = line.angle();

    QFont font;
    font.setPixelSize(16);
    const QString text = QString(App::settings().inch() ? "  ∆X: %1 in\n"
                                                          "  ∆Y: %2 in\n"
                                                          "  ∆/: %3 in\n"
                                                          "  Area: %4 in²\n"
                                                          "  Angle: %5°" :
                                                          "  ∆X: %1 mm\n"
                                                          "  ∆Y: %2 mm\n"
                                                          "  ∆/: %3 mm\n"
                                                          "  Area: %4 mm²\n"
                                                          "  Angle: %5°")
                             .arg(rect.width() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(rect.height() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(length / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg((rect.width() / (App::settings().inch() ? 25.4 : 1.0))
                                     * (rect.height() / (App::settings().inch() ? 25.4 : 1.0)),
                                 4, 'f', 3, '0')
                             .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);

    if (qFuzzyIsNull(line.length()))
        return;

    const double scaleFactor = App::graphicsView()->scaleFactor();
    painter->save();
    painter->setBrush(QColor(127, 127, 127, 100));
    painter->setPen(QPen(Qt::green, 0.0));

    // draw rect
    painter->drawRect(rect);
    const double crossLength = 20.0 * scaleFactor;
    // draw cross pt1
    painter->drawLine(pt1 - QPointF(0, crossLength), pt1 + QPointF(0, crossLength));
    painter->drawLine(pt1 - QPointF(crossLength, 0), pt1 + QPointF(crossLength, 0));

    // draw cross pt1
    painter->drawLine(pt2 - QPointF(0, crossLength), pt2 + QPointF(0, crossLength));
    painter->drawLine(pt2 - QPointF(crossLength, 0), pt2 + QPointF(crossLength, 0));

    // draw arrow
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawLine(line);
    line.setLength(20.0 * scaleFactor);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);

    // draw text
    // painter->setFont(font);
    // painter->drawText(textRect, Qt::AlignLeft, text);
    //    QPointF pt(pt2);
    //    if ((pt.x() + textRect.width() * scaleFactor) > rect.right())
    //        pt.rx() -= textRect.width() * scaleFactor;
    //    if ((pt.y() - textRect.height() * scaleFactor) < rect.top())
    //        pt.ry() += textRect.height() * scaleFactor;

    auto pt {rect.intersected(rect_).center()};
    pt.rx() = std::clamp(pt.x(), rect_.left(), rect_.right() - textRect.width() * scaleFactor);
    pt.ry() = std::clamp(pt.y(), rect_.top() + textRect.height() * scaleFactor* 1.5, rect_.bottom());

    painter->translate(pt);
    painter->scale(scaleFactor, -scaleFactor);
    int i = 0;
    for (const QString& txt : text.split('\n')) {
        QPainterPath path;
        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 0.25 * ++i), font, txt);
        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawPath(path);
    }
    painter->restore();
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    event->ignore();
}

void GraphicsView::dropEvent(QDropEvent* event) {
    for (QUrl& var : event->mimeData()->urls())
        emit fileDroped(var.path().remove(0, 1));
    event->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) { event->acceptProposedAction(); }

void GraphicsView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    updateRuler();
}

void GraphicsView::mousePressEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::MiddleButton) {
        setInteractive(false);
        // по нажатию средней кнопки мыши создаем событие ее отпускания выставляем моду перетаскивания и создаем событие зажатой левой кнопки мыши
        //#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        //        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        //        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        //        setDragMode(ScrollHandDrag);
        //        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        //        QGraphicsView::mousePressEvent(&fakeEvent);
        //#else
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->pos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        setDragMode(ScrollHandDrag);
        QMouseEvent fakeEvent(event->type(), event->pos(), Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
        //#endif
    } else if (event->button() == Qt::RightButton) {
        //        { // удаление мостика
        //            QGraphicsItem* item = scene()->itemAt(mapToScene(event->pos()), transform());
        //            if (item && item->type() == GiType::Bridge && !static_cast<BridgeItem*>(item)->ok())
        //                delete item;
        //        }
        // это что бы при вызове контекстного меню ничего постороннего не было
        setDragMode(NoDrag);
        setInteractive(false);
        // Ruler

        if (ruler_) {
            // FIXME           scene_->setDrawRuller(true);
            const QPointF point(mappedPos(event));
            // FIXME           scene_->setCross2(point);
            emit mouseClickR(point);
        }
    } else {
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        QGraphicsView::mousePressEvent(event);
        if (ruler_ && !(rulCtr++ & 0x1))
            rulPt1 = mappedPos(event);
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
        //#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        //        QMouseEvent fakeEvent(event->type(), event->localPos(), event->screenPos(), event->windowPos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
        //#else
        QMouseEvent fakeEvent(event->type(), event->pos(), Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers());
        //#endif
        QGraphicsView::mouseReleaseEvent(&fakeEvent);
        setDragMode(NoDrag);
        setInteractive(true);
    } else if (event->button() == Qt::RightButton) {
        // это что бы при вызове контекстного меню ничего постороннего не было
        QGraphicsView::mousePressEvent(event);
        setDragMode(RubberBandDrag);
        setInteractive(true);
        // FIXME scene_->setDrawRuller(false);
        emit mouseClickR(mappedPos(event));
        latPos = event->pos();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        emit mouseClickL(mappedPos(event));
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);

    vRuler->SetCursorPos(event->pos());
    hRuler->SetCursorPos(event->pos());
    point = mappedPos(event);
    //    if (event->button() == Qt::RightButton)
    if (ruler_ && rulCtr & 0x1)
        rulPt2 = point;

    emit mouseMove(point);
    scene()->update();
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect) {
    Timer t {__FUNCTION__};
    const auto scale = App::settings().gridStep(hRuler->RulerZoom());
    std::unordered_map<int, int> linesx, linesy;
    auto draw = [&](const auto sc) {
        qreal y, x;

        if (sc >= 1.0) {
            int top = floor(rect.top());
            int left = floor(rect.left());
            y = top - top % int(sc);
            x = left - left % int(sc);
        } else {
            const auto k = 1.0 / sc;
            int top = floor(rect.top()) * k;
            int left = floor(rect.left()) * k;
            y = (top - top % int(k)) / k;
            x = (left - left % int(k)) / k;
        }

        for (const auto end_ = rect.bottom(); y < end_; y += sc) {
            //            painter->drawLine(QLineF {rect.left(), y, rect.right(), y});
            ++linesy[ceil(y * uScale)];
        }
        for (const auto end_ = rect.right(); x < end_; x += sc) {
            //            painter->drawLine(QLineF {x, rect.top(), x, rect.bottom()});
            ++linesx[ceil(x * uScale)];
        }
    };

    QColor color[4] {
        {255, 255, 255, 00},
        {255, 255, 255, 20},
        {255, 255, 255, 40},
        {255, 255, 255, 60},
    };
    //    painter->setPen({color, 0.0 /*2.0 / transform().m11()*/});

    draw(scale * 0.1);
    draw(scale * 0.5);
    draw(scale * 1.0);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    for (auto [x, val] : linesx) {
        auto tmp {x * dScale};
        painter->setPen({color[val], 0.0 /*2.0 / transform().m11()*/});
        painter->drawLine(QLineF {tmp, rect.top(), tmp, rect.bottom()});
    }
    for (auto [y, val] : linesy) {
        auto tmp {y * dScale};
        painter->setPen({color[val], 0.0 /*2.0 / transform().m11()*/});
        painter->drawLine(QLineF {rect.left(), tmp, rect.right(), tmp});
    }

    painter->setPen({Qt::red, 0.0 /*2.0 / transform().m11()*/});

    auto k {10 / hRuler->RulerZoom()};

    painter->drawLine(QLineF {point.x() - k, point.y(), point.x() + k, point.y()});
    painter->drawLine(QLineF {point.x(), point.y() - k, point.x(), point.y() + k});

    painter->drawRect(rect);
    painter->restore();
    if (ruler_)
        drawRuller(painter, rect);
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, Qt::black);
}

template <class T>
void GraphicsView::animate(QObject* target, const QByteArray& propertyName, T begin, T end) {
    auto* animation = new QPropertyAnimation(target, propertyName);
    connect(animation, &QPropertyAnimation::finished, [propertyName, end, this] {
        setProperty(propertyName, end);
        updateRuler();
        point = mapToScene(viewport()->mapFromGlobal(QCursor::pos()));
        scene()->update();
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
