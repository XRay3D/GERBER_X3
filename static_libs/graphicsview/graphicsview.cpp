/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "graphicsview.h"
#include "gi.h"
#include "gi_datasolid.h"
#include "gi_drill.h"
#include "gi_point.h"
#include "myclipper.h"
#include "project.h"
#include "ruler.h"
#include "utils.h"

#include <QDrag>
#include <QDragEnterEvent>
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
#include <limits>
#include <unordered_set>

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

    setCacheMode(CacheBackground);
    setOptimizationFlag(DontSavePainterState);
    setOptimizationFlag(DontAdjustForAntialiasing);
    setViewportUpdateMode(SmartViewportUpdate);
    setDragMode(RubberBandDrag);

    setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true);

    ////////////////////////////////////
    setScene(new QGraphicsScene{this});
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
    QPushButton* corner = new QPushButton{App::settings().isBanana() ? "I" : "M", this};
    connect(corner, &QPushButton::clicked, this, [corner, this](bool fl) {
        corner->setText(fl ? "I" : "M");
        corner->setToolTip(fl ? "Banana" : "Metric");
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
        settings.beginGroup("Viewer");
        setOpenGL(settings.value("chbxOpenGl").toBool());
        setRenderHint(QPainter::Antialiasing, settings.value("chbxAntialiasing", false).toBool());
        viewport()->setObjectName("viewport");
        settings.endGroup();
    }

    setStyleSheet("QGraphicsView { background: "
        % App::settings().guiColor(GuiColors::Background).name(QColor::HexRgb)
        % " }");

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
        ? animate(this, "scale", getScale(), x * zoomFactorAnim)
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
    if(getScale() > 10000.0) return;
    App::settings().guiSmoothScSh()
        ? animate(this, "scale", getScale(), getScale() * zoomFactorAnim)
        : scale(zoomFactor, zoomFactor);
}

void GraphicsView::zoomOut() {
    if(getScale() < 1.0) return;
    App::settings().guiSmoothScSh()
        ? animate(this, "scale", getScale(), getScale() * (1.0 / zoomFactorAnim))
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
    //    const auto r1(getViewRect().toRect());
    //    const auto r2(dstRect.toRect());
    //    if (r1 == r2)
    //        return;
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

QPointF GraphicsView::mappedPos(QMouseEvent* event) const {
    return App::settings().getSnappedPos(mapToScene(event->position().toPoint()), event->modifiers());
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
    QPoint p = mapFromScene(QPointF{});
    vRuler->setOrigin(p.y());
    hRuler->setOrigin(p.x());
    vRuler->setRulerZoom(std::abs(transform().m22() * 0.1));
    hRuler->setRulerZoom(std::abs(transform().m11() * 0.1));
}

template <class T>
void GraphicsView::animate(QObject* target, const QByteArray& propertyName, T begin, T end) {
    auto* animation = new QPropertyAnimation{target, propertyName};
    connect(animation, &QPropertyAnimation::finished, [propertyName, end, this] {
        setProperty(propertyName, end);
        updateRuler();
        point = mapToScene(viewport()->mapFromGlobal(QCursor::pos()));
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
    QString text;
    std::format_to(std::back_insert_iterator(text),
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
    );

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
        double distance = 0;
        center = item->boundingRect().center();

        for(const auto& paths: ditem->getPaths()) {
            for(const auto& path: paths) {
                QPointF point{path.x * dScale, path.y * dScale};
                distance = std::max(distance,
                    std::sqrt(std::pow(point.x() - center.x(), 2)
                        + std::pow(point.y() - center.y(), 2)));
                maxX = std::max(maxX, point.x());
                maxY = std::max(maxY, point.y());
            }
        }

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
        emit fileDroped(var.path().remove(0, 1));

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
        default:
            // QGraphicsView::wheelEvent(event);
            return;
        }
    }
    mouseMove(mapToScene(pos));
    event->accept();
    update();
}

void GraphicsView::mousePressEvent(QMouseEvent* event) {
    // QGraphicsView::mousePressEvent(event);
    if(event->buttons() & Qt::MiddleButton) {
        qInfo("MiddleButton");
        QMouseEvent releaseEvent{
            QEvent::MouseButtonRelease,
            event->position(),
            event->scenePosition(),
            event->globalPosition(),
            Qt::LeftButton,
            event->buttons() | Qt::LeftButton,
            event->modifiers(),
        };
        QGraphicsView::mouseReleaseEvent(&releaseEvent);
        setDragMode(ScrollHandDrag);
        setInteractive(false);
        QMouseEvent fakeEvent{
            event->type(),
            event->position(),
            event->scenePosition(),
            event->globalPosition(),
            Qt::LeftButton,
            event->buttons() | Qt::LeftButton,
            event->modifiers(),
        };
        QGraphicsView::mousePressEvent(&fakeEvent);
    } else if(event->button() == Qt::RightButton) {
        qInfo("RightButton");
        //        { // удаление мостика
        //            QGraphicsItem* item = scene()->itemAt(mapToScene(event->position().toPoint()), transform());
        //            if (item && item->type() == Gi::Type::Bridge && !static_cast<BridgeItem*>(item)->ok())
        //                delete item;
        //        }
        // это что бы при вызове контекстного меню ничего постороннего не было
        setDragMode(NoDrag);
        emit mouseClickR(mappedPos(event));

        // Ruler

        if(ruler_) {
            const QPointF point(mappedPos(event));
            emit mouseClickR(point);
            // scene_->setDrawRuller(true);
            // scene_->setCross2(point);
        }

        QGraphicsItem* item = scene()->itemAt(mapToScene(event->position().toPoint()), transform());
        if(item && contains(item->type(), Gi::Type::Drill, Gi::Type::DataSolid))
            GiToShapeEvent(event, item);
        QGraphicsView::mousePressEvent(event);
    } else {
        qInfo("else");
        setDragMode(RubberBandDrag);
        // это для выделения рамкой  - работа по-умолчанию левой кнопки мыши
        QGraphicsView::mousePressEvent(event);
        if(ruler_ && !(rulerCtr++ & 0x1)) rulPt1 = mappedPos(event);

        if(auto item{scene()->itemAt(mapToScene(event->position().toPoint()), transform())};
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
    // QGraphicsView::mousePressEvent(event);
    if(event->button() == Qt::MiddleButton) {
        qInfo("MiddleButton");
        // отпускаем левую кнопку мыши которую виртуально зажали в mousePressEvent
        QMouseEvent fakeEvent{
            event->type(),
            event->position(),
            event->scenePosition(),
            event->globalPosition(),
            Qt::LeftButton,
            event->buttons() & ~Qt::LeftButton,
            event->modifiers(),
        };
        QGraphicsView::mouseReleaseEvent(&fakeEvent);
        setDragMode(RubberBandDrag);
        setInteractive(true);
    } else if(event->button() == Qt::RightButton) {
        qInfo("RightButton");
        // это что бы при вызове контекстного меню ничего постороннего не было
        QGraphicsView::mousePressEvent(event);
        setDragMode(RubberBandDrag);
        // WTF scene_->setDrawRuller(false);
        latPos = event->position().toPoint();
    } else {
        qInfo("else");
        QGraphicsView::mouseReleaseEvent(event);
        emit mouseClickL(mappedPos(event));
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event) {
    emit mouseMove(mapToScene(event->position().toPoint()));
    QGraphicsView::mouseMoveEvent(event);

    vRuler->setCursorPos(event->position().toPoint());
    hRuler->setCursorPos(event->position().toPoint());
    point = mappedPos(event);
    if(ruler_ && rulerCtr & 0x1) rulPt2 = point;

    // расчёт смещения для нулевых координат
    emit mouseMove2(point, point - App::project().zeroPos());

    scene()->update();
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event) {
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect) {
#if 1
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHints(QPainter::TextAntialiasing); // | QPainter::HighQualityAntialiasing);

    int gs{};
    mvector<QLineF> lines[3];
    std::unordered_set<int> dublicateFilter[2]{{0}, {0}}; // reserve to draw the origin
    int size[3][2]{};

    auto calcGrid = [&](double scaleMeter) {
        bool isHor{};

        auto fillLines = [&](double start, double end, double step) {
            size[gs][isHor] = abs(ceil((end - start) / step));
            lines[gs].reserve(size[gs][0] + size[gs][1]);
            for(double current = start; (step < 0 ? current >= end : current <= end); current += step)
                if(dublicateFilter[isHor].emplace(static_cast<int>(current * (std::numeric_limits<int>::max() / 10000))).second) // filter
                    isHor ? lines[gs].emplace_back(current, rect.top(), current, rect.bottom())
                          : lines[gs].emplace_back(rect.left(), current, rect.right(), current);
        };

        auto borders = [&] {
            const double start = isHor ? rect.left() : rect.top();   //  rectangle starting mark
            const double end = isHor ? rect.right() : rect.bottom(); //  rectangle ending mark

            /*
            Условие A # Если исходная точка находится между начальной и конечной границей,
            нам нужно провести линию от левой до правой границы.
            Условие B # Если исходная точка находится слева от начальной границы,
            нам нужно провести линию от исходной точки до конечной границы.
            Условие C # Если исходная точка находится справа от конечной границы,
            нам нужно провести линию от исходной точки до начальной границы.
            */
            if(start <= 0 && 0 <= end) {
                fillLines(0, end, +scaleMeter);
                fillLines(0, start, -scaleMeter);
            } else if(0 < start) {
                int tickCount = +static_cast<int>(start / scaleMeter);
                fillLines(+scaleMeter * tickCount, end, +scaleMeter);
            } else if(0 > end) {
                int tickCount = -static_cast<int>(end / scaleMeter);
                fillLines(-scaleMeter * tickCount, start, -scaleMeter);
            }
        };
        borders();
        isHor = true;
        borders();
    };
    const auto gridStep = App::settings().gridStep(getScale());
    // drawing a scale of 1.0
    gs = 2;
    calcGrid(gridStep * 10);

    // drawing a scale of 0.2
    gs = 1;
    calcGrid(gridStep * 5);

    // drawing a scale of 0.1
    gs = 0;
    calcGrid(gridStep * 1);

    const QColor color[3]{
        App::settings().guiColor(GuiColors::Grid01),
        App::settings().guiColor(GuiColors::Grid05),
        App::settings().guiColor(GuiColors::Grid10),
    };
    const double penWidth = 1.0 / getScale();
    // draw grid
    for(int i{}; i < 3; ++i) {
        painter->setPen({color[i], penWidth});
        painter->drawLines(lines[i].data(), lines[i].size());
    }

    { // draw mouse cross
        const double k = 100 /*px*/ / getScale();
        painter->setPen({Qt::red, penWidth});
        QLineF lines[2]{
            {point.x() - k,     point.y(), point.x() + k,     point.y()},
            {    point.x(), point.y() - k,     point.x(), point.y() + k}
        };
        painter->drawLines(lines, 2);
    }

    // draw the origin
    // if(rect.contains(QPointF{}))
    {
        auto color = App::settings().guiColor(GuiColors::Grid10);
        color.setRed(255);
        painter->setPen({color, penWidth});
        QLineF lines[2]{
            {          0, rect.top(),            0, rect.bottom()},
            {rect.left(),          0, rect.right(),             0}
        };
        painter->drawLines(lines, 2);
    }

    if(ruler_) drawRuller(painter, rect);

    painter->restore();

#else
    const double scale = App::settings().gridStep(hRuler->zoom());

    static std::unordered_map<int, int> gridLinesX, gridLinesY;
    gridLinesX.clear(), gridLinesY.clear();
    auto draw = [&](const auto sc) {
        double y, x;
        if(sc >= 1.0) {
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

        for(const auto end_ = rect.bottom(); y < end_; y += sc)
            ++gridLinesY[ceil(y * uScale)];

        for(const auto end_ = rect.right(); x < end_; x += sc)
            ++gridLinesX[ceil(x * uScale)];
    };

    const QColor color[4]{
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
    for(auto&& vec: lines)
        vec.clear();
    double tmp{};
    for(auto [x, colorIndex]: gridLinesX) {
        tmp = x * dScale;
        lines[colorIndex].emplace_back(tmp, rect.top(), tmp, rect.bottom());
    }
    for(auto [y, colorIndex]: gridLinesY) {
        tmp = y * dScale;
        lines[colorIndex].emplace_back(rect.left(), tmp, rect.right(), tmp);
    }
    const double penWidth = 1.0 / getScale();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    for(int colorIndex{}; auto&& vec: lines) {
        painter->setPen({color[colorIndex++], penWidth});
        painter->drawLines(vec.data(), vec.size());
    }

    auto k{100 / transform().m11()};
    painter->setPen({Qt::red, penWidth});
    painter->drawLine(QLineF{point.x() - k, point.y(), point.x() + k, point.y()});
    painter->drawLine(QLineF{point.x(), point.y() - k, point.x(), point.y() + k});

    if(ruler_) drawRuller(painter, rect);

    painter->restore();
#endif
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, Qt::black);
}

void GraphicsView::timerEvent(QTimerEvent* /*event*/) {
    //    if (event->timerId() == timerId)
    ++App::dashOffset();
    scene()->update();
}

#include "moc_graphicsview.cpp"
