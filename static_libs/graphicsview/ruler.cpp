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
// https://kernelcoder.wordpress.com/tag/ruler-in-qgraphicsview/
#include "ruler.h"
#include "app.h"
#include "gridtick.h"

// #include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QLabel>
#include <QMimeData>
// #include <QMouseEvent>
#include <QPainter>
// #include <QTextDocument>
// #include <QTextFormat>
// #include <QWindow>
// #include <QtMath>
// #include <cstring>
#if 0
static QLabel* createDragLabel(const QString& text, QWidget* parent) {
    QLabel* label = new QLabel{text, parent};
    label->setAutoFillBackground(true);
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Raised);
    return label;
}

static QString hotSpotMimeDataKey() { return u"application/x-hotspot"_s; }
#endif
Ruler::Ruler(Qt::Orientation rulerType, QWidget* parent)
    : QWidget{parent}
    , orientation_{rulerType} {
    setMouseTracking(mouseTracking);
    setAcceptDrops(true);
}

void Ruler::setCursorPos(const QPoint& newCursorPos) {
    cursorPos = newCursorPos;
    update();
}

void Ruler::setMouseTrack(const bool track) {
    if(mouseTracking != track) {
        mouseTracking = track;
        setMouseTracking(mouseTracking);
        update();
    }
}

void Ruler::setOrigin(const double newOrigin) {
    if(!qFuzzyCompare(origin_, newOrigin)) {
        origin_ = newOrigin;
        update();
    }
}

void Ruler::setRulerUnit(const double newRulerUnit) {
    if(!qFuzzyCompare(rulerUnit_, newRulerUnit)) {
        rulerUnit_ = newRulerUnit;
        update();
    }
}

void Ruler::setRulerZoom(const double newRulerZoom) {
    if(!qFuzzyCompare(rulerZoom_, newRulerZoom)) {
        rulerZoom_ = newRulerZoom;
        update();
    }
}

void Ruler::dragEnterEvent(QDragEnterEvent* event) {
    // if (event->mimeData()->hasText()) {
    // if (event->source() == this) {
    // event->setDropAction(Qt::MoveAction);
    // event->accept();
    // } else {
    // event->acceptProposedAction();
    // }
    // } else {
    // event->ignore();
    // }

    // auto mimeData {event->mimeData()};
    // if (mimeData->hasText() && mimeData->text() == Ruler::MimeType)
    if(event->mimeData()->hasFormat(MimeType))
        event->acceptProposedAction(); // event->accept();
    else
        event->ignore();
}

void Ruler::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
    // if (event->mimeData()->hasFormat(MimeType)) {
    // event->setDropAction(Qt::MoveAction);
    // event->accept();
    // } else {
    // event->ignore();
    // }
}

void Ruler::dropEvent(QDropEvent* event) {
    // if (event->mimeData()->hasFormat(MimeType)) {
    // QByteArray pieceData = event->mimeData()->data(MimeType);
    // QDataStream dataStream{&pieceData, QIODevice::ReadOnly};
    // QPixmap pixmap;
    // QPoint location;
    // dataStream >> pixmap >> location;

    // // addPiece(pixmap, location);

    // event->setDropAction(Qt::MoveAction);
    // event->accept();
    // } else {
    // event->ignore();
    // }
    auto mimeData{event->mimeData()};
    if(mimeData->hasText() && mimeData->data(MimeType).size() == sizeof(void*)) {
        void* ptr{};
        std::memcpy(&ptr, mimeData->data(MimeType).data(), sizeof(nullptr));
        qDebug() << __FUNCTION__ << mimeData->data(MimeType) << ptr;
        // delete ptr;

        // const QMimeData* mime = event->mimeData();
        // QStringList pieces {}; // = mime->text().split(QRegularExpression(u"\\s+"_s), Qt::SkipEmptyParts);
        // QPoint position = event->pos();
        // QPoint hotSpot;

        // QByteArrayList hotSpotPos = mime->data(hotSpotMimeDataKey()).split(u' ');
        // if (hotSpotPos.size() == 2) {
        // hotSpot.setX(hotSpotPos.first().toInt());
        // hotSpot.setY(hotSpotPos.last().toInt());
        // }

        // for (const QString& piece : pieces) {
        // QLabel* newLabel = createDragLabel(piece, this);
        // newLabel->move(position - hotSpot);
        // newLabel->show();
        // newLabel->setAttribute(Qt::WA_DeleteOnClose);

        // position += QPoint(newLabel->width(), 0);
        // }

        // if (event->source() == this) {
        // event->setDropAction(Qt::MoveAction);
        // event->accept();
        // } else {
        event->acceptProposedAction();
        // }
    } else {
        event->ignore();
    }
    // for (QWidget* widget : findChildren<QWidget*>()) {
    // if (!widget->isVisible())
    // widget->deleteLater();
    // }
}

void Ruler::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);
    cursorPos = event->pos();
    update();
}

void Ruler::mousePressEvent(QMouseEvent* /*event*/) {
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(MimeType);
    mimeData->setData(MimeType, QByteArray{1, static_cast<char>(orientation_)});

    QPixmap pixmapIcon{Breadth, Breadth};
    pixmapIcon.fill(Qt::Horizontal == orientation_ ? Qt::red : Qt::green);

    QDrag* drag = new QDrag{this};
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmapIcon);
    drag->setHotSpot(pixmapIcon.rect().center());

    Qt::DropAction dropAction = drag->exec(); // Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

    if(dropAction == Qt::MoveAction) {
    }
}

void Ruler::paintEvent(QPaintEvent* event [[maybe_unused]]) {
    QPainter painter{this};

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHints(QPainter::TextAntialiasing); // | QPainter::HighQualityAntialiasing);
    painter.setPen({Qt::darkGray, 0.0});                // zero width pen is cosmetic pen
    QRectF rulerRect{rect()};                           // We want to work with floating point, so we are considering the rect as QRectF

    // at first fill the rect
    painter.fillRect(rulerRect, App::settings().guiColor(GuiColors::Background));
    if(qFuzzyIsNull(rulerZoom_))
        return;

    gridStep = App::settings().gridStep(rulerZoom_);

    // drawing a scale of 0.1
    if((gridStep * rulerZoom_) > 35) {
        tickKoef = 0.1;
        drawText = true;
    }
    painter.setPen({Qt::darkGray, 1.0}); // BUG when 0.0 random brightness
    DrawAScaleMeter(&painter, rulerRect, gridStep * 1, static_cast<double>(Ruler::Breadth) * 0.6);
    drawText = false;

    // drawing a scale of 0.2
    if((gridStep * rulerZoom_) <= 35) {
        tickKoef = 0.5;
        drawText = true;
    }
    painter.setPen({Qt::green, 1.0}); // BUG when 0.0 random brightness
    DrawAScaleMeter(&painter, rulerRect, gridStep * 5, static_cast<double>(Ruler::Breadth) * 0.3);
    drawText = false;

    // drawing a scale of 1.0
    painter.setPen({Qt::red, 1.0}); // BUG when 0.0 random brightness
    DrawAScaleMeter(&painter, rulerRect, gridStep * 10, static_cast<double>(Ruler::Breadth) * 0);

    // drawing the current mouse position indicator
    if(mouseTracking)
        DrawMousePosTick(&painter);

    // drawing no man's land between the ruler & view
    if(/* NOTE DISABLES CODE */ (0)) {
        QPointF starPt((Qt::Horizontal == orientation_) ? rulerRect.bottomLeft() : rulerRect.topRight());
        QPointF endPt((Qt::Horizontal == orientation_) ? rulerRect.bottomRight() : rulerRect.bottomRight()); // WTF same branches!!!!!!
        painter.setPen({Qt::red, 2});
        painter.drawLine(starPt, endPt);
    }
}

void Ruler::DrawAScaleMeter(QPainter* painter, QRectF rulerRect, double scaleMeter, double startPosition) {
    const bool isHorzRuler = Qt::Horizontal == orientation_;
    const double step = scaleMeter * rulerUnit_ * rulerZoom_;

    // Ruler rectangle starting/ending mark
    const double rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
    const double rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();

    const auto K = gridStep * tickKoef * (1.0 / App::settings().lenUnit());
    constexpr double padding = 3;

    painter->setFont(font());

    std::vector<QLineF> lines;
    lines.reserve(static_cast<size_t>(std::ceil((rulerEndMark - rulerStartMark) / step)) + 2);

    // tick position = origin_ + index * step, computed by multiplication (not
    // accumulated addition) so there is no drift when panned far from the origin.
    forEachGridTick(origin_, rulerStartMark, rulerEndMark, step, [&](int64_t index, double current) {
        lines.emplace_back(
            isHorzRuler ? current : rulerRect.left() + startPosition,
            isHorzRuler ? rulerRect.top() : current,
            isHorzRuler ? current : rulerRect.right(),
            isHorzRuler ? rulerRect.bottom() - startPosition : current);

        if(drawText) [[unlikely]] {
            painter->save();
            QColor color{0xFFFFFF ^ App::settings().guiColor(GuiColors::Background).rgb()};
            painter->setPen({color, 0.0});
            auto number{QString::number(std::abs(index) * K)};

            if(index) [[likely]] {
                const bool positive = isHorzRuler ? index > 0 : index < 0;
                number = (positive ? u"+"_s : u"-"_s) + number;
            }

            QRectF textRect(QFontMetricsF(font()).boundingRect(number));
            textRect.setWidth(textRect.width() + 1);
            if(isHorzRuler) {
                painter->translate(current + padding, textRect.height());
                painter->drawText(textRect, Qt::AlignCenter, number);
            } else {
                painter->translate(textRect.height() - padding, current - padding);
                painter->rotate(-90);
                painter->drawText(textRect, number);
            }
            painter->restore();
        }
    });

    painter->drawLines(lines.data(), lines.size());
}

void Ruler::DrawMousePosTick(QPainter* painter) {
    QPoint starPt = cursorPos;
    QPoint endPt;
    if(Qt::Horizontal == orientation_) {
        starPt.setY(this->rect().top());
        endPt.setX(starPt.x());
        endPt.setY(this->rect().bottom());
    } else {
        starPt.setX(this->rect().left());
        endPt.setX(this->rect().right());
        endPt.setY(starPt.y());
    }
    painter->drawLine(starPt, endPt);
}

#include "moc_ruler.cpp"
