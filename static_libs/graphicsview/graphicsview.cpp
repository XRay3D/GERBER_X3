/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "graphicsview.h"
#include "gi.h"
#include "gi_datasolid.h"
#include "gi_drill.h"
#include "gi_point.h"
#include "gridtick.h"

#include "project.h"
#include "ruler.h"
#include "utils.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGridLayout>
#include <QGuiApplication>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollBar>
#include <QUndoCommand>

#include <cmath>
#include <format>

constexpr double zoomFactor = 1.5;
constexpr double zoomFactorAnim = 1.7;

void setCursor(QWidget* w) {
    enum {
        Size = 21,
        Mid = 10
    };
    QPixmap cursor{Size, Size};
    cursor.fill(Qt::transparent);
    QPainter p{&cursor};
    p.setPen({QColor(App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF), 1.0});
    p.drawLine(0, Mid, Size, Mid);
    p.drawLine(Mid, 0, Mid, Size);
    w->setCursor(QCursor{cursor, Mid, Mid});
}

GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView{
          parent
}
    , hRuler{new Ruler{Qt::Horizontal, this}},
    vRuler{new Ruler{Qt::Vertical, this}},
    gridLayout{new QGridLayout{this}} {

    // setCacheMode(CacheBackground);
    setOptimizationFlag(DontSavePainterState);
    // setOptimizationFlag(DontAdjustForAntialiasing);
    // setViewportUpdateMode(SmartViewportUpdate);
    setDragMode(RubberBandDrag);

    // setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true);

    ////////////////////////////////////
    struct Scene final : public QGraphicsScene {
        QPointF last;
        using QGraphicsScene::QGraphicsScene;
        QPointF mappedPos() { return qobject_cast<GraphicsView*>(parent())->scenePos; }
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
            event->setScenePos(last = mappedPos());
            QGraphicsScene::mousePressEvent(event);
        }
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
            if(last == mappedPos()) return;
            event->setScenePos(last = mappedPos());
            QGraphicsScene::mouseMoveEvent(event);
        }
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
            event->setScenePos(last = mappedPos());
            QGraphicsScene::mouseReleaseEvent(event);
        }
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override {
            event->setScenePos(last = mappedPos());
            QGraphicsScene::mouseDoubleClickEvent(event);
        }
    };
    setScene(new Scene{this});
    App::setGraphicsView(this);

    scene()->setSceneRect(-1000, -1000, +2000, +2000); // 2x2 meters

    // add two rulers on top and left.
    setViewportMargins(Ruler::Breadth, 0, 0, Ruler::Breadth);

    // create rulers

    hRuler->setMouseTrack(true);
    vRuler->setMouseTrack(true);
    ::setCursor(hRuler);
    ::setCursor(vRuler);

    // add items to grid layout
    QPushButton* corner = new QPushButton{App::settings().isBanana() ? u"I"_s : u"M"_s, this};
    connect(corner, &QPushButton::clicked, this, [corner, this](bool fl) {
        corner->setText(fl ? u"I"_s : u"M"_s);
        corner->setToolTip(fl ? u"Banana"_s : u"Metric"_s);
        App::settings().setBanana(fl);
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

    connect(horizontalScrollBar(), &QScrollBar::valueChanged,
        this, &GraphicsView::updateRuler);
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
        this, &GraphicsView::updateRuler);

    {
        QSettings settings;
        settings.beginGroup(u"Viewer"_s);
        setOpenGL(settings.value(u"chbxOpenGl"_s).toBool());
        setRenderHint(QPainter::Antialiasing, settings.value(u"chbxAntialiasing"_s, false).toBool());
        viewport()->setObjectName(u"viewport"_s);
        settings.endGroup();
    }

    setStyleSheet(u"QGraphicsView { background: "_s
        % App::settings().guiColor(GuiColors::Background).name(QColor::HexRgb)
        % u" }"_s);

    startUpdateTimer(20);

    scale(1.0, -1.0); // flip vertical
}

GraphicsView::~GraphicsView() { App::setGraphicsView(nullptr); }

void GraphicsView::zoom100() {
    double x = 1.0, y = 1.0;

    const double m11 = QGraphicsView::transform().m11(), m22 = QGraphicsView::transform().m22();
    if(/* DISABLES CODE */ (0)) {
        x = std::abs(1.0 / m11 / (25.4 / physicalDpiX()));
        y = std::abs(1.0 / m22 / (25.4 / physicalDpiY()));
    } else {
        const QSizeF size = screen()->physicalSize();                          // size in mm
        const QRect scrGeometry(QGuiApplication::primaryScreen()->geometry()); // size in pix
        x = std::abs(1.0 / m11 / (size.height() / scrGeometry.height()));
        y = std::abs(1.0 / m22 / (size.width() / scrGeometry.width()));
    }

    (0 && App::settings().guiSmoothScSh())
        ? animate(this, "scale"_ba, getScale(), x * zoomFactorAnim)
        : scale(x, y);
}

void GraphicsView::zoomFit() {
    auto rect{App::layoutFrames().boundingRect()};
    auto size{rect.size() * 0.1};
    rect += QMarginsF(size.width(), size.height(), size.width(), size.height());
    fitInView(rect /*scene()->itemsBoundingRect()*/, false);
}

void GraphicsView::zoomToSelected() {
    QRectF rect;
    for(const QGraphicsItem* item: scene()->selectedItems()) {
        const QRectF tmpRect(item->pos().isNull()
                ? item->boundingRect()
                : item->boundingRect().translated(item->pos()));
        rect = /*rect.isEmpty() ? tmpRect :*/ rect.united(tmpRect);
    }
    if(rect.isEmpty()) return;
    fitInView(rect);
}

void GraphicsView::zoomIn() {
    if(getScale() > 100000.0) return;
    App::settings().guiSmoothScSh()
        ? animate(this, "scale"_ba, getScale(), getScale() * zoomFactorAnim)
        : scale(zoomFactor, zoomFactor);
}

void GraphicsView::zoomOut() {
    if(getScale() < 1.0) return;
    App::settings().guiSmoothScSh()
        ? animate(this, "scale"_ba, getScale(), getScale() * (1.0 / zoomFactorAnim))
        : scale(1.0 / zoomFactor, 1.0 / zoomFactor);
}

void GraphicsView::fitInView(QRectF dstRect, bool withBorders) {
    if(dstRect.isNull()) return;
    if(withBorders) {
        const auto padding = dstRect.size() / 5; // 5 mm
        dstRect += QMarginsF{
            padding.width(),
            padding.height(),
            padding.width(),
            padding.height(),
        };
    }
    // const auto r1(getViewRect().toRect());
    // const auto r2(dstRect.toRect());
    // if (r1 == r2)
    // return;
    if(App::settings().guiSmoothScSh()) {
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

QPointF GraphicsView::toScenePos(QMouseEvent* event) {
    scenePos = App::settings().getSnappedPos(
        mapToScene(event->position()), event->modifiers());
    return scenePos;
}

QPointF GraphicsView::mapFromScene(const QPointF& point) const {
    static QPainterPath pp;
    pp.clear();
    pp.moveTo(point);
    return QGraphicsView::mapFromScene(pp).elementAt(0);
}

QPointF GraphicsView::mapToScene(const QPointF& point) const {
    static QPainterPath pp;
    pp.clear();
    pp.moveTo(point);
    return QGraphicsView::mapToScene(pp).elementAt(0);
}

void GraphicsView::setScale(double s) noexcept {
    const auto trf(transform());
    setTransform({+s /*11*/, trf.m12(), trf.m13(),
        /*      */ trf.m21(), -s /*22*/, trf.m23(),
        /*      */ trf.m31(), trf.m32(), trf.m33()});
}

void GraphicsView::scale(double sx, double sy) {
    QGraphicsView::scale(sx, sy);
    updateRuler();
}

void GraphicsView::setOpenGL(bool useOpenGL) {
    // do {
    if(useOpenGL) {
        // if(dynamic_cast<QOpenGLWidget*>(viewport())) break;
        auto oglWidget = new QOpenGLWidget{this};
        QSurfaceFormat format;
        format.setSamples(8);
        oglWidget->setFormat(format);
        setViewport(oglWidget);
    } else {
        // if(dynamic_cast<QWidget*>(viewport())) break;
        setViewport(new QWidget{this});
    }
    // } while(false);
    ::setCursor(viewport());
    gridLayout->addWidget(viewport(), 0, 1);
}

void GraphicsView::setViewRect(const QRectF& r) {
    QGraphicsView::fitInView(r, Qt::KeepAspectRatio);
}

QRectF GraphicsView::getViewRect() {
    return mapToScene(rect()).boundingRect();
    // QPointF topLeft(horizontalScrollBar()->value(), verticalScrollBar()->value());
    // QPointF bottomRight(topLeft + viewport()->rect().bottomRight());
    // QRectF visible_scene_rect(transform().inverted().mapRect({topLeft, bottomRight}));
    // return visible_scene_rect;
}

QRectF GraphicsView::getSelectedBoundingRect() {
    auto selectedItems{scene()->selectedItems()};
    if(selectedItems.isEmpty()) return {};
    ScopedTrue sTrue{boundingRect_};
    QRectF rect = selectedItems.front()->boundingRect();
    for(auto* gi: selectedItems) rect = rect.united(gi->boundingRect());
    if(!rect.isEmpty()) App::project().setWorckRect(rect);
    return rect;
}

void GraphicsView::updateRuler() {
    // layout()->setContentsMargins(0, 0, 0, horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0);
    updateSceneRect(QRectF{}); // actualize mapFromScene
    QPoint p = QGraphicsView::mapFromScene(QPointF{});
    vRuler->setOrigin(p.y());
    hRuler->setOrigin(p.x());
    vRuler->setRulerZoom(std::abs(transform().m22() * 0.1));
    hRuler->setRulerZoom(std::abs(transform().m11() * 0.1));
}

template <class T>
void GraphicsView::animate(QObject* target, const QByteArray& propertyName, T begin, T end) {
    auto* animation = new QPropertyAnimation{target, propertyName};
    connect(animation, &QPropertyAnimation::finished, [propertyName, end, this] {
        setProperty(propertyName.data(), end);
        updateRuler();
        scenePos = mapToScene(viewport()->mapFromGlobal(QCursor::pos()));
        scene()->update();
    });
    if constexpr(std::is_same_v<T, QRectF>) {
        animation->setEasingCurve(QEasingCurve(QEasingCurve::InOutSine));
        animation->setDuration(200);
    } else if constexpr(std::is_same_v<decltype(target), QScrollBar>) {
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

void GraphicsView::drawRuller(QPainter* painter, const QRectF& rect_) const {
    if(rulPt2 == rulPt1) return;

    QLineF line{rulPt2, rulPt1};

    const QRectF rect{rulPt1, rulPt2};
    const double angle = line.angle();
    const auto sz = QPointF{rect.width(), rect.height()} /= App::settings().lenUnit();
    QString text = QString::fromStdString(
        std::format(
            " ∆X: {0:.3f} {5}\n"
            " ∆Y: {1:.3f} {5}\n"
            " ∆/: {2:.3f} {5}\n"
            " Area: {3:.3f} {5}²\n"
            " Angle: {4:.3f}°",
            sz.x(),                                  // 0
            sz.y(),                                  // 1
            line.length(),                           // 2
            std::abs(sz.x() * sz.y()),               // 3
            normalizeAngleDegrees(angle),            // 4
            App::settings().isBanana() ? "in" : "mm" // 5
            ));

    const double scaleFactor = App::grView().scaleFactor();
    const double crossLength = 20.0 * scaleFactor;
    const double penWidth = 1.0 / getScale();

    painter->setPen({Qt::green, penWidth});
#if 0
    // draw rect
    painter->setBrush(QColor(127, 127, 127, 100));
    painter->drawRect(rect);
#endif
    auto drawCross = [&crossLength, painter](QPointF pt) { // draw cross
        painter->drawLine(QLineF{
            {pt.x(), pt.y() - crossLength},
            {pt.x(), pt.y() + crossLength}
        });
        painter->drawLine(QLineF{
            {pt.x() - crossLength, pt.y()},
            {pt.x() + crossLength, pt.y()}
        });
    };
    drawCross(rulPt1);
    drawCross(rulPt2);

    // draw arrow
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen({Qt::white, penWidth});
    painter->drawLine(line);
    line.setLength(20.0 * scaleFactor);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);

    // draw text
    const auto size{QFontMetrics{font()}.size(Qt::AlignLeft | Qt::AlignJustify | Qt::TextDontClip, text)};
    auto pt{rect.center()};
    pt.rx() -= size.width() * 0.5 * scaleFactor;
    pt.ry() += size.height() * 0.5 * scaleFactor;
    pt.rx() = std::clamp(pt.x(), rect_.left(), rect_.right() - size.width() * scaleFactor);
    pt.ry() = std::clamp(pt.y(), rect_.top() + size.height() * scaleFactor, rect_.bottom());
    painter->translate(pt);
    painter->scale(scaleFactor, -scaleFactor);
    painter->setFont(font());

    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->fillRect(QRect{{}, size}, QColor{0, 0, 0, 127});
    painter->setBrush(Qt::white);
    painter->drawText(QRect{{}, size}, Qt::AlignLeft, text);
}

namespace Gi {

class Guide : public QGraphicsItem {

    Qt::Orientation orientation;
    double scaleFactor() const {
        if(scene() && scene()->views().size())
            return 1.0 / scene()->views().first()->transform().m11();
        return 1.0;
    };

public:
    Guide(QPointF pos, Qt::Orientation orientation)
        : orientation{orientation} {
        setFlags(ItemIsSelectable | ItemIsMovable);
        orientation == Qt::Horizontal ? setPos(0, pos.y()) : setPos(pos.x(), 0);
    }
    ~Guide() override = default;

public:
    // QGraphicsItem interface
    QRectF boundingRect() const override {
        qDebug(__FUNCTION__);
        auto sceneRect{scene()->sceneRect()};
        auto k = 2 * scaleFactor();
        if(orientation == Qt::Horizontal)
            return {sceneRect.left(), -k, sceneRect.width(), k * 2};
        else
            return {-k, sceneRect.top(), k * 2, sceneRect.height()};
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override {
        painter->fillRect(boundingRect(), Qt::magenta);
    }

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        QGraphicsItem::mouseReleaseEvent(event);
        orientation == Qt::Horizontal ? setPos(0, y()) : setPos(x(), 0);
    }
};

} // namespace Gi

void GraphicsView::GiToShapeEvent(QMouseEvent* event, QGraphicsItem* item) {
    QPointF center, radius, rect;
    double maxX = -1000000, maxY = -1000000;

    if(item->type() == Gi::Type::Drill) {
        Gi::Drill* gitem = dynamic_cast<Gi::Drill*>(item);
        center = item->boundingRect().center();
        radius = QPointF{gitem->diameter() / 2, 0};
        rect = QPointF{gitem->diameter() / 2, gitem->diameter() / 2};
    }

    if(item->type() == Gi::Type::DataSolid) {
        Gi::DataFill* ditem = dynamic_cast<Gi::DataFill*>(item);
        double distance{};
        center = item->boundingRect().center();
        qFatal("edit");
        // for(const auto& paths: ditem->getPaths()) {
        //     for(const auto& path: paths) {
        //         QPointF point{path.x * dScale, path.y * dScale};
        //         distance = std::max(distance,
        //             std::sqrt(std::pow(point.x() - center.x(), 2)
        //                 + std::pow(point.y() - center.y(), 2)));
        //         maxX = std::max(maxX, point.x());
        //         maxY = std::max(maxY, point.y());
        //     }
        // }

        radius = QPointF{distance, 0};
        rect = QPointF{maxX, maxY} - center;
    }

    QMenu menu;
    QAction makeCircle(tr("Circle from Item"), &menu);
    QAction makeRect(tr("Rectangle from Item"), &menu);
    menu.addAction(&makeCircle);
    menu.addAction(&makeRect);

    connect(&makeCircle, &QAction::triggered, [&center, &radius]() {
        App::project().makeShapeCircle(center, center + radius);
    });

    connect(&makeRect, &QAction::triggered, [&center, &rect]() {
        App::project().makeShapeRectangle(center - rect, center + rect);
    });
    menu.exec(event->globalPosition().toPoint());
    scene()->update();
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    qDebug(__FUNCTION__);
    auto mimeData{event->mimeData()};

    if(mimeData->hasFormat(Ruler::MimeType))
        event->acceptProposedAction();
    else if(mimeData->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

void GraphicsView::dropEvent(QDropEvent* event) {
    qDebug(__FUNCTION__);
    auto mimeData{event->mimeData()};
    for(QUrl& var: mimeData->urls())
        emit fileDroped(var.path()
#ifdef Q_OS_WINDOWS
                .remove(0, 1)
#endif
        );

    if(mimeData->hasFormat(Ruler::MimeType) && event->source() != this)
        scene()->addItem(new Gi::Guide{
            mapToScene(event->position().toPoint()),
            Qt::Orientation(*mimeData->data(Ruler::MimeType).data()),
        });

    event->accept();
}

void GraphicsView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    updateRuler();
}

void GraphicsView::wheelEvent(QWheelEvent* event) {
    const auto delta = event->angleDelta();
    const auto pos = event->position().toPoint();
    if(event->buttons() & Qt::RightButton) {
        if(abs(delta.y()) == 120) (delta.y() > 0) ? zoomIn() : zoomOut();
    } else {
        auto sbUpdate = [&delta, this, scale = 3](QScrollBar* sb) {
            auto value = sb->value();
            App::settings().guiSmoothScSh()
                ? animate(sb, "value", value, value - sb->pageStep() / (delta.y() > 0 ? +scale : -scale))
                : sb->setValue(value - delta.y());
        };
        switch(event->modifiers()) {
        case Qt::ControlModifier:
            if(abs(delta.y()) == 120) (delta.y() > 0) ? zoomIn() : zoomOut();
            break;
        case Qt::ShiftModifier:
            if(!delta.x()) sbUpdate(horizontalScrollBar());
            break;
        case Qt::NoModifier:
            if(!delta.x()) sbUpdate(verticalScrollBar());
            break;
        default: // QGraphicsView::wheelEvent(event);
            return;
        }
    }
    mouseMove(mapToScene(pos));
    event->accept();
    update();
}

auto mouseDragEvent(QMouseEvent* event, QEvent::Type type, Qt::MouseButton button, bool add) {
    return std::make_unique<QMouseEvent>(
        type,
        event->position(),
        event->scenePosition(),
        event->globalPosition(),
        button,
        add ? event->buttons() | button : event->buttons() & ~button,
        event->modifiers());
}

void GraphicsView::mousePressEvent(QMouseEvent* event) {
    pressPos = mapToScene(event->position());
    toScenePos(event);
    qDebug() << event->button();
    if(event->buttons() & Qt::MiddleButton) {
        event->accept();
        setDragMode(ScrollHandDrag);
        setInteractive(false);
        QMouseEvent fakeEvent{event->type(),
            event->position(), event->scenePosition(), event->globalPosition(),
            Qt::LeftButton, event->buttons() | Qt::LeftButton, event->modifiers()};
        QGraphicsView::mousePressEvent(&fakeEvent);
    } else if(event->button() == Qt::RightButton) {
        // { // удаление мостика
        // QGraphicsItem* item = scene()->itemAt(mapToScene(event->position().toPoint()), transform());
        // if (item && item->type() == Gi::Type::Bridge && !static_cast<BridgeItem*>(item)->ok())
        // delete item;
        // }

        // setDragMode(NoDrag);
        // setInteractive(true);
        // QGraphicsView::mousePressEvent(event);

        // это что бы при вызове контекстного меню ничего постороннего не было
        emit mouseClickR(scenePos);
#if 0
        QGraphicsItem* item = scene()->itemAt(/*mapToScene(event->position().toPoint())*/
            scenePos, transform());

        if(item && contains<Gi::Type::Drill, Gi::Type::DataSolid>(item->type()))
            GiToShapeEvent(event, item);
#endif
    } else {
        qInfo("else");
        QGraphicsView::mousePressEvent(event);
        // setDragMode(RubberBandDrag);
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        if(ruler_ && !(rulerCtr++ & 0x1)) rulPt1 = scenePos;

        if(auto item{scene()->itemAt(scenePos, transform())};
            0 && item
            && item->type() == QGraphicsPixmapItem::Type) { // NOTE  возможно DD для направляющих не сделаю.
            QMimeData* mimeData = new QMimeData;
            mimeData->setText(Ruler::MimeType);
            mimeData->setData(Ruler::MimeType,
                QByteArray::fromRawData(reinterpret_cast<const char*>(item),
                    sizeof(void*)));

            QPixmap pixmapIcon{Ruler::Breadth, Ruler::Breadth};
            pixmapIcon.fill(Qt::magenta);

            QDrag* drag = new QDrag{this};
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmapIcon);
            drag->setHotSpot(pixmapIcon.rect().center());
            Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
            if(dropAction == Qt::MoveAction) { }
        }
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event) {
    qWarning() << event->button();
    if(event->button() == Qt::MiddleButton) {
        event->accept();
        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
        QMouseEvent fakeEvent{event->type(),
            event->position(), event->scenePosition(), event->globalPosition(),
            Qt::LeftButton, event->buttons() & ~Qt::LeftButton, event->modifiers()};
        QGraphicsView::mouseReleaseEvent(&fakeEvent);
        setDragMode(RubberBandDrag);
        setInteractive(true);
    } else if(event->button() == Qt::RightButton) {
        // это что бы при вызове контекстного меню ничего постороннего не было
        // QGraphicsView::mousePressEvent(event);
        setDragMode(RubberBandDrag);
        // setDragMode(RubberBandDrag);
        // setInteractive(true);
        // emit mouseClickR(toScenePos(event));
    } else {
        QGraphicsView::mouseReleaseEvent(event);
        emit mouseClickL(toScenePos(event));
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);

    auto dir = pressPos - mapToScene(event->position());
    if(dir.x() > 0 && dir.y() < 0)
        setRubberBandSelectionMode(Qt::IntersectsItemShape);
    else
        setRubberBandSelectionMode(Qt::ContainsItemShape);

    QPoint pt = event->position().toPoint();
    vRuler->setCursorPos(pt);
    hRuler->setCursorPos(pt);

    emit mouseMove(toScenePos(event));

    if(ruler_ && rulerCtr & 0x1) rulPt2 = scenePos;

    // scene()->update();
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event) {
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect) {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHints(QPainter::TextAntialiasing); // | QPainter::HighQualityAntialiasing);

    // Single pass over the finest grid: every 5th tick also belongs to the
    // medium grid, every 10th also to the coarse grid, so classifying by
    // index replaces the old float-hashed dedup set entirely.
    mvector<QLineF> lines[3]; // 0 = Grid01, 1 = Grid05, 2 = Grid10
    const auto gridStep = App::settings().gridStep(getScale());

    const auto tickCountX = static_cast<size_t>(std::floor((rect.right() - rect.left()) / gridStep)) + 2;
    const auto tickCountY = static_cast<size_t>(std::floor((rect.bottom() - rect.top()) / gridStep)) + 2;
    for(auto& l: lines) l.reserve(tickCountX + tickCountY);

    forEachGridTick(0.0, rect.left(), rect.right(), gridStep, [&](int64_t index, double x) {
        lines[gridTickLevel(index)].emplace_back(x, rect.top(), x, rect.bottom());
    });
    forEachGridTick(0.0, rect.top(), rect.bottom(), gridStep, [&](int64_t index, double y) {
        lines[gridTickLevel(index)].emplace_back(rect.left(), y, rect.right(), y);
    });

    const QColor color[3]{
        App::settings().guiColor(GuiColors::Grid01),
        App::settings().guiColor(GuiColors::Grid05),
        App::settings().guiColor(GuiColors::Grid10),
    };
    const double penWidth = 1.0 / getScale();
    // draw grid, coarsest on top
    for(int i{}; i < 3; ++i) {
        painter->setPen({color[i], penWidth});
        painter->drawLines(lines[i].data(), lines[i].size());
    }

    // draw the origin
    // if(rect.contains(QPointF{}))
    {
        auto color = App::settings().guiColor(GuiColors::Grid10);
        color.setRed(255);
        painter->setPen({color, penWidth});
        QLineF lines[2]{
            {0,           rect.top(), 0,            rect.bottom()},
            {rect.left(), 0,          rect.right(), 0            }
        };
        painter->drawLines(lines, 2);
    }

    { // draw mouse cross
        const double k = 100 /*px*/ / getScale();
        painter->setPen({Qt::red, penWidth});
        QLineF lines[2]{
            {scenePos.x() - k, scenePos.y(),     scenePos.x() + k, scenePos.y()    },
            {scenePos.x(),     scenePos.y() - k, scenePos.x(),     scenePos.y() + k}
        };
        painter->drawLines(lines, 2);
    }

    if(ruler_) drawRuller(painter, rect);

    painter->restore();
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, Qt::black);
}

void GraphicsView::timerEvent(QTimerEvent* /*event*/) {
    // if (event->timerId() == timerId)
    ++App::dashOffset();
    scene()->update();
}

#include "moc_graphicsview.cpp"
