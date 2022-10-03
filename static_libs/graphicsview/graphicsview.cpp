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
#include "gi.h"
#include "mainwindow.h"
#include "myclipper.h"
#include "project.h"
#include "ruler.h"
#include "settings.h"
#include "utils.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLWidget>
#include <QPropertyAnimation>
#include <QUndoCommand>
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

#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>

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

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , hRuler {new Ruler(Qt::Horizontal, this)}
    , vRuler {new Ruler(Qt::Vertical, this)}
    , gridLayout {new QGridLayout(this)} {

    setCacheMode(CacheBackground);
    setOptimizationFlag(DontSavePainterState);
    setOptimizationFlag(DontAdjustForAntialiasing);
    setViewportUpdateMode(SmartViewportUpdate);
    setDragMode(RubberBandDrag);
    setInteractive(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true);

    ////////////////////////////////////
    setScene(new QGraphicsScene(this));

    scene()->setSceneRect(-1000, -1000, +2000, +2000);

    // add two rulers on top and left.
    setViewportMargins(Ruler::Breadth, 0, 0, Ruler::Breadth);

    // create rulers

    hRuler->setMouseTrack(true);
    vRuler->setMouseTrack(true);
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
    rulerCtr = 0;
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
    vRuler->setOrigin(p.y());
    hRuler->setOrigin(p.x());
    vRuler->setRulerZoom(qAbs(transform().m22() * 0.1));
    hRuler->setRulerZoom(qAbs(transform().m11() * 0.1));
}

void GraphicsView::drawRuller(QPainter* painter, const QRectF& rect_) const {

    QLineF line {rulPt2, rulPt1};
    const double length = line.length();
    if (qFuzzyIsNull(length))
        return;
    //    const QRectF rect(
    //        QPointF(std::min(rulPt1.x(), rulPt2.x()), std::min(rulPt1.y(), rulPt2.y())),
    //        QPointF(std::max(rulPt1.x(), rulPt2.x()), std::max(rulPt1.y(), rulPt2.y())));
    const QRectF rect {rulPt1, rulPt2};
    const double angle = line.angle();

    const auto text {
        App::settings().inch() ?
            QString(" ∆X: %1 in\n"
                    " ∆Y: %2 in\n"
                    " ∆/: %3 in\n"
                    " Area: %4 in²\n"
                    " Angle: %5°")
                .arg(rect.width() / 25.4, 4, 'f', 3, '0')
                .arg(rect.height() / 25.4, 4, 'f', 3, '0')
                .arg(length / 25.4, 4, 'f', 3, '0')
                .arg((rect.width() / 25.4) * (rect.height() / 25.4), 4, 'f', 3, '0')
                .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0') :
            QString(
                " ∆X: %1 mm\n"
                " ∆Y: %2 mm\n"
                " ∆/: %3 mm\n"
                " Area: %4 mm²\n"
                " Angle: %5°")
                .arg(rect.width(), 4, 'f', 3, '0')
                .arg(rect.height(), 4, 'f', 3, '0')
                .arg(length, 4, 'f', 3, '0')
                .arg(rect.width() * rect.height(), 4, 'f', 3, '0')
                .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0')

    };

    const double scaleFactor = App::graphicsView()->scaleFactor();
    const double crossLength = 20.0 * scaleFactor;

    painter->setPen(QPen(Qt::green, 0.0));
    // draw rect
    //    painter->setBrush(QColor(127, 127, 127, 100));
    //    painter->drawRect(rect);
    // draw cross
    auto drawCross = [&](auto pt) {
        painter->drawLine(pt - QPointF(0, crossLength), pt + QPointF(0, crossLength));
        painter->drawLine(pt - QPointF(crossLength, 0), pt + QPointF(crossLength, 0));
    };
    drawCross(rulPt1);
    drawCross(rulPt2);

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
    const auto size {QFontMetrics(font()).size(Qt::AlignLeft | Qt::AlignJustify | Qt::TextDontClip, text)};
    auto pt {rect.center()};
    pt.rx() -= size.width() * 0.5 * scaleFactor;
    pt.ry() += size.height() * 0.5 * scaleFactor;
    pt.rx() = std::clamp(pt.x(), rect_.left(), rect_.right() - size.width() * scaleFactor);
    pt.ry() = std::clamp(pt.y(), rect_.top() + size.height() * scaleFactor, rect_.bottom());
    painter->translate(pt);
    painter->scale(scaleFactor, -scaleFactor);
    painter->setFont(font());

    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->fillRect(QRect {{}, size}, QColor {0, 0, 0, 127});
    painter->setBrush(Qt::white);
    painter->drawText(QRect {{}, size}, Qt::AlignLeft, text);
}

class name : public QGraphicsItem {

    Qt::Orientation orientation;
    double scaleFactor() const {
        if (scene() && scene()->views().size())
            return 1.0 / scene()->views().first()->transform().m11();
        return 1.0;
    };

public:
    name(QPointF pos, Qt::Orientation orientation)
        : orientation {orientation} {
        setFlags(ItemIsSelectable | ItemIsMovable);
        orientation == Qt::Horizontal ? setPos(0, pos.y()) : setPos(pos.x(), 0);
    }
    ~name() override = default;

public:
    // QGraphicsItem interface
    QRectF boundingRect() const override {
        qDebug(__FUNCTION__);
        auto sceneRect {scene()->sceneRect()};
        auto k = 2 * scaleFactor();
        if (orientation == Qt::Horizontal) {
            return {sceneRect.left(), -k, sceneRect.width(), k * 2};
        } else {
            return {-k, sceneRect.top(), k * 2, sceneRect.height()};
        }
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        painter->fillRect(boundingRect(), Qt::magenta);
    }

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        QGraphicsItem::mouseReleaseEvent(event);
        orientation == Qt::Horizontal ? setPos(0, y()) : setPos(x(), 0);
    }
};

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    qDebug(__FUNCTION__);
    auto mimeData {event->mimeData()};

    if (mimeData->hasFormat(Ruler::mimeType()))
        event->acceptProposedAction();
    else if (mimeData->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) { event->acceptProposedAction(); }

void GraphicsView::dropEvent(QDropEvent* event) {
    qDebug(__FUNCTION__);
    auto mimeData {event->mimeData()};
    for (QUrl& var : mimeData->urls())
        emit fileDroped(var.path().remove(0, 1));

    if (mimeData->hasFormat(Ruler::mimeType()) && event->source() != this) {
        scene()->addItem(new name(mapToScene(event->pos()), Qt::Orientation(*mimeData->data(Ruler::mimeType()).data())));
    }

    event->accept();
}

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
        if (ruler_ && !(rulerCtr++ & 0x1))
            rulPt1 = mappedPos(event);

        if (auto item {scene()->itemAt(mapToScene(event->pos()), transform())}; 0 && item && item->type() == QGraphicsPixmapItem::Type) { // NOTE  возможно DD для направляющих не сделаю.
            QMimeData* mimeData = new QMimeData;
            mimeData->setText(Ruler::mimeType());
            mimeData->setData(Ruler::mimeType(), QByteArray::fromRawData(reinterpret_cast<const char*>(item), sizeof(item)));

            QPixmap pixmapIcon {Ruler::Breadth, Ruler::Breadth};
            pixmapIcon.fill(Qt::magenta);

            QDrag* drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmapIcon);
            drag->setHotSpot(pixmapIcon.rect().center());

            Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

            if (dropAction == Qt::MoveAction) {
            }
        }
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

    vRuler->setCursorPos(event->pos());
    hRuler->setCursorPos(event->pos());
    point = mappedPos(event);
    //    if (event->button() == Qt::RightButton)
    if (ruler_ && rulerCtr & 0x1)
        rulPt2 = point;

    emit mouseMove(point);
    scene()->update();
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect) {
    const double scale = App::settings().gridStep(hRuler->zoom());
    const double lineWidth {0}; //{ 2.0 / transform().m11() };
    static std::unordered_map<int, int> gridLinesX, gridLinesY;
    gridLinesX.clear(), gridLinesY.clear();
    auto draw = [&](const auto sc) {
        double y, x;
        if (sc >= 1.0) {
            int top = floor(rect.top());
            int left = floor(rect.left());
            y = top - top % int(sc);
            x = left - left % int(sc);
        } else {
            const double k = 1.0 / sc;
            int top = floor(rect.top()) * k;
            int left = floor(rect.left()) * k;
            y = (top - top % int(k)) / k;
            x = (left - left % int(k)) / k;
        }

        for (const auto end_ = rect.bottom(); y < end_; y += sc)
            ++gridLinesY[ceil(y * uScale)];

        for (const auto end_ = rect.right(); x < end_; x += sc)
            ++gridLinesX[ceil(x * uScale)];
    };

    const QColor color[4] {
        {255, 0, 0, 127}, // рисовние нулей красным
        App::settings().guiColor(GuiColors::Grid01),
        App::settings().guiColor(GuiColors::Grid05),
        App::settings().guiColor(GuiColors::Grid10),
    };

    draw(scale * 0.1);
    draw(scale * 0.5);
    draw(scale * 1.0);

    gridLinesX[0] = 0; // рисовние нулей красным
    gridLinesY[0] = 0;

    static mvector<QLineF> lines[4];
    for (auto&& vec : lines)
        vec.clear();
    double tmp {};
    for (auto [x, colorIndex] : gridLinesX) {
        tmp = x * dScale;
        lines[colorIndex].emplace_back(tmp, rect.top(), tmp, rect.bottom());
    }
    for (auto [y, colorIndex] : gridLinesY) {
        tmp = y * dScale;
        lines[colorIndex].emplace_back(rect.left(), tmp, rect.right(), tmp);
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    for (int colorIndex {}; auto&& vec : lines) {
        painter->setPen({color[colorIndex++], lineWidth});
        painter->drawLines(vec.data(), vec.size());
    }

    auto k {100 / transform().m11()};
    painter->setPen({Qt::red, 0.0 /*2.0 / transform().m11()*/});
    painter->drawLine(QLineF {point.x() - k, point.y(), point.x() + k, point.y()});
    painter->drawLine(QLineF {point.x(), point.y() - k, point.x(), point.y() + k});

    if (ruler_)
        drawRuller(painter, rect);

    painter->restore();
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

//// QTime time_;
//// int time2_;
//// int frameCount_ = 0;
//// int frameCount2_;

// Scene::Scene(QObject* parent)
//     : QGraphicsScene(parent) {
//     //  App::setScene(this);
//     double size = 1000.0; // 4 sqare meters
//     setSceneRect(-size, -size, +size * 2, +size * 2);

//    startTimer(1000);
//}

// Scene::~Scene() {
//     //    App::setScene(nullptr);
// }

// QRectF Scene::itemsBoundingRect() {
//     ScopedTrue sTrue(drawPdf_);
//     QRectF rect(QGraphicsScene::itemsBoundingRect());
//     return rect;
// }

// QRectF Scene::getSelectedBoundingRect() {
//     auto selectedItems(App::graphicsView()->scene()->selectedItems());

//    if (selectedItems.isEmpty())
//        return {};

//    QRectF rect;

//    {
//        ScopedTrue sTrue(boundingRect_);
//        rect = selectedItems.front()->boundingRect();
//        for (auto gi : selectedItems)
//            rect = rect.united(gi->boundingRect());
//    }

//    if (!rect.isEmpty())
//        App::project()->setWorckRect(rect);

//    return rect;
//}

// void Scene::setCross1(const QPointF& cross) {
//     cross1 = cross;
//     update();
// }

// void Scene::setCross2(const QPointF& cross) {
//     cross2 = cross;
// }

// void Scene::setDrawRuller(bool drawRuller) {
//     drawRuller = drawRuller;
//     update();
// }

// void Scene::drawRuller(QPainter* painter) {
//     const QPointF pt1(cross2);
//     const QPointF pt2(cross1);
//     QLineF line(pt2, pt1);
//     const QRectF rect(
//         QPointF(qMin(pt1.x(), pt2.x()), qMin(pt1.y(), pt2.y())),
//         QPointF(std::max(pt1.x(), pt2.x()), std::max(pt1.y(), pt2.y())));
//     const double length = line.length();
//     const double angle = line.angle();

//    QFont font;
//    font.setPixelSize(16);
//    const QString text = QString(App::settings().inch() ? "∆X: %1 in\n"
//                                                          "∆Y: %2 in\n"
//                                                          "∆/: %3 in\n"
//                                                          "Area: %4 in²\n"
//                                                          "Angle: %5°" :
//                                                          "∆X: %1 mm\n"
//                                                          "∆Y: %2 mm\n"
//                                                          "∆/: %3 mm\n"
//                                                          "Area: %4 mm²\n"
//                                                          "Angle: %5°")
//                             .arg(rect.width() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
//                             .arg(rect.height() / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
//                             .arg(length / (App::settings().inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
//                             .arg((rect.width() / (App::settings().inch() ? 25.4 : 1.0))
//                                     * (rect.height() / (App::settings().inch() ? 25.4 : 1.0)),
//                                 4, 'f', 3, '0')
//                             .arg(360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0), 4, 'f', 3, '0');

//    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);

//    if (qFuzzyIsNull(line.length()))
//        return;

//    const double scaleFactor = App::graphicsView()->scaleFactor();
//    painter->save();
//    painter->setBrush(QColor(127, 127, 127, 100));
//    painter->setPen(QPen(Qt::green, 0.0));
//    {
//        // draw rect
//        painter->drawRect(rect);
//        const double crossLength = 20.0 * scaleFactor;
//        // draw cross pt1
//        painter->drawLine(pt1, pt1 + QPointF(0, crossLength));
//        painter->drawLine(pt1, pt1 + QPointF(crossLength, 0));
//        painter->drawLine(pt1, pt1 - QPointF(0, crossLength));
//        painter->drawLine(pt1, pt1 - QPointF(crossLength, 0));
//        // draw cross pt1
//        painter->drawLine(pt2, pt2 + QPointF(0, crossLength));
//        painter->drawLine(pt2, pt2 + QPointF(crossLength, 0));
//        painter->drawLine(pt2, pt2 - QPointF(0, crossLength));
//        painter->drawLine(pt2, pt2 - QPointF(crossLength, 0));
//    }

//    { // draw arrow
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        painter->setPen(QPen(Qt::white, 0.0));
//        painter->drawLine(line);
//        line.setLength(20.0 * scaleFactor);
//        line.setAngle(angle + 10);
//        painter->drawLine(line);
//        line.setAngle(angle - 10);
//        painter->drawLine(line);
//    }
//    // draw text
//    // painter->setFont(font);
//    // painter->drawText(textRect, Qt::AlignLeft, text);
//    QPointF pt(pt2);
//    if ((pt.x() + textRect.width() * scaleFactor) > rect.right())
//        pt.rx() -= textRect.width() * scaleFactor;
//    if ((pt.y() - textRect.height() * scaleFactor) < rect.top())
//        pt.ry() += textRect.height() * scaleFactor;
//    painter->translate(pt);
//    painter->scale(scaleFactor, -scaleFactor);
//    int i = 0;
//    for (const QString& txt : text.split('\n')) {
//        QPainterPath path;
//        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 0.25 * ++i), font, txt);
//        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//        painter->setBrush(Qt::NoBrush);
//        painter->drawPath(path);
//        painter->setPen(Qt::NoPen);
//        painter->setBrush(Qt::white);
//        painter->drawPath(path);
//    }
//    painter->restore();
//}

// void Scene::drawBackground(QPainter* painter, const QRectF& rect) {
//     if (drawPdf_)
//         return;

//    painter->fillRect(rect, App::settings().guiColor(GuiColors::Background));
//}

// void Scene::drawForeground(QPainter* painter, const QRectF& rect) {
//     if (drawPdf_)
//         return;

//    ++fpsCtr;

//    { // draw grid
//        const long upScale = 100000;
//        const long forLimit = 1000;
//        const double downScale = 1.0 / upScale;
//        static bool in = App::settings().inch();

//        if (!qFuzzyCompare(scale, views().first()->transform().m11()) || lastRect != rect || in != App::settings().inch()) {
//            in = App::settings().inch();
//            scale = views().first()->transform().m11();
//            if (qFuzzyIsNull(scale))
//                return;

//            lastRect = rect;

//            hGrid.clear();
//            vGrid.clear();

//            // Grid Step 0.1
//            double gridStep = App::settings().gridStep(scale);
//            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
//                      right = static_cast<long>(rect.right() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 hPos < right && ++nlp < forLimit; hPos += step) {
//                hGrid[hPos] = 0;
//            }
//            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
//                      bottom = static_cast<long>(rect.bottom() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 vPos < bottom && ++nlp < forLimit; vPos += step) {
//                vGrid[vPos] = 0;
//            }
//            // Grid Step  0.5
//            gridStep *= 5;
//            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
//                      right = static_cast<long>(rect.right() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 hPos < right && ++nlp < forLimit; hPos += step) {
//                hGrid[hPos] = 1;
//            }
//            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
//                      bottom = static_cast<long>(rect.bottom() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 vPos < bottom && ++nlp < forLimit; vPos += step) {
//                vGrid[vPos] = 1;
//            }
//            // Grid Step  1.0
//            gridStep *= 2;
//            for (long hPos = static_cast<long>(qFloor(rect.left() / gridStep) * gridStep * upScale),
//                      right = static_cast<long>(rect.right() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 hPos < right && ++nlp < forLimit; hPos += step) {
//                hGrid[hPos] = 2;
//            }
//            for (long vPos = static_cast<long>(qFloor(rect.top() / gridStep) * gridStep * upScale),
//                      bottom = static_cast<long>(rect.bottom() * upScale),
//                      step = static_cast<long>(gridStep * upScale), nlp = 0;
//                 vPos < bottom && ++nlp < forLimit; vPos += step) {
//                vGrid[vPos] = 2;
//            }
//        }

//        const QColor color[] {
//            App::settings().guiColor(GuiColors::Grid1),
//            App::settings().guiColor(GuiColors::Grid5),
//            App::settings().guiColor(GuiColors::Grid10),
//        };

//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing, false);
//        QElapsedTimer t;
//        t.start();
//        const double k2 = 0.5 / scale;
//        // painter->setCompositionMode(QPainter::CompositionMode_Lighten);

//        for (int i = 0; i < 3; ++i) {
//            painter->setPen(QPen(color[i], 0.0));
//            for (long hPos : hGrid.keys(i)) {
//                if (hPos)
//                    painter->drawLine(QLineF(hPos * downScale + k2, rect.top(), hPos * downScale + k2, rect.bottom()));
//            }
//            for (long vPos : vGrid.keys(i)) {
//                if (vPos)
//                    painter->drawLine(QLineF(rect.left(), vPos * downScale + k2, rect.right(), vPos * downScale + k2));
//            }
//        }

//        // zero cross
//        painter->setPen(QPen(QColor(255, 0, 0, 100), 0.0));
//        painter->drawLine(QLineF(k2, rect.top(), k2, rect.bottom()));
//        painter->drawLine(QLineF(rect.left(), -k2, rect.right(), -k2));
//    }

//    if (1) { // screen mouse cross
//        QList<QGraphicsItem*> items = QGraphicsScene::items(cross1, Qt::IntersectsItemShape, Qt::DescendingOrder, views().first()->transform());
//        bool fl = false;
//        for (QGraphicsItem* item : items) {
//            if (item && item->type() != GiType::Bridge && item->flags() & QGraphicsItem::ItemIsSelectable) {
//                fl = true;
//                break;
//            }
//        }
//        if (fl)
//            painter->setPen(QPen(QColor(255, 000, 000, 255), 0.0));
//        else {
//            QColor c(App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF);
//            //            c.setAlpha(150);
//            painter->setPen(QPen(c, 0.0));
//        }

//        painter->drawLine(QLineF(cross1.x(), rect.top(), cross1.x(), rect.bottom()));
//        painter->drawLine(QLineF(rect.left(), cross1.y(), rect.right(), cross1.y()));
//    }

//    if (drawRuller_)
//        drawRuller(painter);

//    //    if (0) {
//    //        if (frameCount_ == 0) {
//    //            time_.start();
//    //            time2_ = time_.elapsed() + 1000;

//    //        } else {
//    //            if (time_.elapsed() > time2_) {

//    //                time2_ = time_.elapsed() + 1000;
//    //                frameCount2_ = frameCount_;
//    //                frameCount_ = 0;
//    //            }

//    //            painter->setRenderHint(QPainter::Antialiasing, true);
//    //            QString str(QString("FPS %1").arg(frameCount2_));
//    //            painter->translate(rect_.center());
//    //            const double scaleFactor = App::graphicsView()->scaleFactor();
//    //            painter->scale(scaleFactor, -scaleFactor);
//    //            QFont f;
//    //            f.setPixelSize(100);
//    //            QPainterPath path;
//    //            path.addText(QPointF(), f, str);
//    //            painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    //            painter->setBrush(Qt::NoBrush);
//    //            painter->drawPath(path);
//    //            painter->setPen(Qt::NoPen);
//    //            painter->setBrush(Qt::white);
//    //            painter->drawPath(path);
//    //        }
//    //        frameCount_++;
//    //    }

//    { // NOTE FPS counter
//        painter->setRenderHint(QPainter::Antialiasing, true);

//        const double scaleFactor = App::graphicsView()->scaleFactor();
//        painter->translate(rect.bottomLeft());
//        painter->scale(scaleFactor, -scaleFactor);

//        QPainterPath path;

//        auto txt {QString("FPS: %1").arg(currentFps)};

//        QFont font;
//        font.setPixelSize(16);
//        font.setWeight(QFont::Thin);

//        const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, txt);
//        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height()), font, txt);

//        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//        painter->setBrush(Qt::black);
//        painter->drawPath(path);
//        painter->setPen(Qt::NoPen);
//        painter->setBrush(Qt::white);
//        painter->drawPath(path);
//    }

//    painter->restore();
//}

// void Scene::timerEvent(QTimerEvent* event) {
//     currentFps = fpsCtr;
//     fpsCtr = 0;
// }

#include "moc_graphicsview.cpp"
