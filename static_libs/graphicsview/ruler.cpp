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
// https://kernelcoder.wordpress.com/tag/ruler-in-qgraphicsview/
#include "ruler.h"
#include "app.h"

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
    //    if (event->mimeData()->hasText()) {
    //        if (event->source() == this) {
    //            event->setDropAction(Qt::MoveAction);
    //            event->accept();
    //        } else {
    //            event->acceptProposedAction();
    //        }
    //    } else {
    //        event->ignore();
    //    }

    //    auto mimeData {event->mimeData()};
    //    if (mimeData->hasText() && mimeData->text() == Ruler::MimeType)
    if(event->mimeData()->hasFormat(MimeType))
        event->acceptProposedAction(); // event->accept();
    else
        event->ignore();
}

void Ruler::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
    //    if (event->mimeData()->hasFormat(MimeType)) {
    //        event->setDropAction(Qt::MoveAction);
    //        event->accept();
    //    } else {
    //        event->ignore();
    //    }
}

void Ruler::dropEvent(QDropEvent* event) {
    //    if (event->mimeData()->hasFormat(MimeType)) {
    //        QByteArray pieceData = event->mimeData()->data(MimeType);
    //        QDataStream dataStream(&pieceData, QIODevice::ReadOnly);
    //        QPixmap pixmap;
    //        QPoint location;
    //        dataStream >> pixmap >> location;

    //        //        addPiece(pixmap, location);

    //        event->setDropAction(Qt::MoveAction);
    //        event->accept();
    //    } else {
    //        event->ignore();
    //    }
    auto mimeData{event->mimeData()};
    if(mimeData->hasText() && mimeData->data(MimeType).size() == sizeof(void*)) {
        void* ptr{};
        std::memcpy(&ptr, mimeData->data(MimeType).data(), sizeof(nullptr));
        qDebug() << __FUNCTION__ << mimeData->data(MimeType) << ptr;
        //        delete ptr;

        //        const QMimeData* mime = event->mimeData();
        //        QStringList pieces {}; // = mime->text().split(QRegularExpression(u"\\s+"_s), Qt::SkipEmptyParts);
        //        QPoint position = event->pos();
        //        QPoint hotSpot;

        //        QByteArrayList hotSpotPos = mime->data(hotSpotMimeDataKey()).split(' ');
        //        if (hotSpotPos.size() == 2) {
        //            hotSpot.setX(hotSpotPos.first().toInt());
        //            hotSpot.setY(hotSpotPos.last().toInt());
        //        }

        //        for (const QString& piece : pieces) {
        //            QLabel* newLabel = createDragLabel(piece, this);
        //            newLabel->move(position - hotSpot);
        //            newLabel->show();
        //            newLabel->setAttribute(Qt::WA_DeleteOnClose);

        //            position += QPoint(newLabel->width(), 0);
        //        }

        //        if (event->source() == this) {
        //            event->setDropAction(Qt::MoveAction);
        //            event->accept();
        //        } else {
        event->acceptProposedAction();
        //        }
    } else {
        event->ignore();
    }
    //    for (QWidget* widget : findChildren<QWidget*>()) {
    //        if (!widget->isVisible())
    //            widget->deleteLater();
    //    }
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

void Ruler::DrawAScaleMeter(QPainter* painter, QRectF rulerRect, double scaleMeter, double startPositoin) {
    // Flagging whether we are horizontal or vertical only to reduce
    // to cheching many times
    bool isHorzRuler = Qt::Horizontal == orientation_;

    scaleMeter = scaleMeter * rulerUnit_ * rulerZoom_;

    // Ruler rectangle starting mark
    double rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
    // Ruler rectangle ending mark
    double rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();
    /*
    Condition A # If origin point is between the start & end mard, we have to draw both from origin to left mark & origin to right mark.
    Condition B # If origin point is left of the start mark, we have to draw from origin to end mark.
    Condition C # If origin point is right of the end mark, we have to draw from origin to start mark.
    */
    if(origin_ >= rulerStartMark && origin_ <= rulerEndMark) {
        DrawFromOriginTo(painter, rulerRect, origin_, rulerEndMark, 0, scaleMeter, startPositoin);
        DrawFromOriginTo(painter, rulerRect, origin_, rulerStartMark, 0, -scaleMeter, startPositoin);
    } else if(origin_ < rulerStartMark) {
        int tickNo = int((rulerStartMark - origin_) / scaleMeter);
        DrawFromOriginTo(painter, rulerRect, origin_ + scaleMeter * tickNo,
            rulerEndMark, tickNo, scaleMeter, startPositoin);
    } else if(origin_ > rulerEndMark) {
        int tickNo = int((origin_ - rulerEndMark) / scaleMeter);
        DrawFromOriginTo(painter, rulerRect, origin_ - scaleMeter * tickNo,
            rulerStartMark, tickNo, -scaleMeter, startPositoin);
    }
}

void Ruler::DrawFromOriginTo(QPainter* painter, QRectF rect, double startMark, double endMark, int startTickNo, double step, double startPosition) {
    const auto isHorzRuler = (Qt::Horizontal == orientation_);
    const auto K = gridStep * tickKoef * (1.0 / App::settings().lenUnit());

    painter->setFont(font());

    mvector<QLineF> lines;
    lines.reserve(abs(ceil((endMark - startMark) / step)));

    constexpr double padding = 3;

    for(double current = startMark; (step < 0 ? current >= endMark : current <= endMark); current += step) {
        double x1, y1;
        lines.emplace_back(
            x1 = isHorzRuler ? current : rect.left() + startPosition,
            y1 = isHorzRuler ? rect.top() : current,
            /*x2*/ isHorzRuler ? current : rect.right(),
            /*y2*/ isHorzRuler ? rect.bottom() - startPosition : current);
        if(drawText) [[unlikely]] {
            painter->save();
            QColor color{0xFFFFFF ^ App::settings().guiColor(GuiColors::Background).rgb()};
            painter->setPen({color, 0.0});
            auto number{QString::number(startTickNo * K)};

            if(startTickNo) [[likely]]
                number = ((isHorzRuler ^ (step > 0.0)) ? "-" : "+") + number;

            QRectF textRect(QFontMetricsF(font()).boundingRect(number));
            textRect.setWidth(textRect.width() + 1);
            if(isHorzRuler) {
                painter->translate(x1 + padding, textRect.height());
                painter->drawText(textRect, Qt::AlignCenter, number);
            } else {
                painter->translate(textRect.height() - padding, y1 - padding);
                painter->rotate(-90);
                painter->drawText(textRect, number);
            }
            painter->restore();
        }
        ++startTickNo;
    }
    // painter->setPen(meterPen); // zero width pen is cosmetic pen
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
