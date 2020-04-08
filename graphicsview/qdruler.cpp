#include "qdruler.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTextDocument>
#include <QTextFormat>
#include <QtMath>
#include <settings.h>

QDRuler::QDRuler(QDRuler::RULER_TYPE rulerType, QWidget* parent)
    : QWidget(parent)
    , drawText(false)
    , mouseTracking(false)
    , gridStep(1.0)
    , origin(0.0)
    , rulerUnit(1.0)
    , rulerZoom(1.0)
    , tickKoef(1.0)
    , rulerType(rulerType)
{
    setMouseTracking(mouseTracking);
    //QFont txtFont("Vrinda");
    //txtFont.setStyleHint(QFont::TypeWriter, QFont::PreferOutline);
    //setFont(txtFont);
}

QSize QDRuler::minimumSizeHint() const
{
    return QSize(RulerBreadth, RulerBreadth);
}

QDRuler::RULER_TYPE QDRuler::RulerType() const
{
    return rulerType;
}

qreal QDRuler::Origin() const
{
    return origin;
}

qreal QDRuler::RulerUnit() const
{
    return rulerUnit;
}

qreal QDRuler::RulerZoom() const
{
    return rulerZoom;
}

void QDRuler::SetOrigin(const qreal origin_)
{
    if (!qFuzzyCompare(origin, origin_)) {
        origin = origin_;
        update();
    }
}

void QDRuler::SetRulerUnit(const qreal rulerUnit_)
{
    if (!qFuzzyCompare(rulerUnit, rulerUnit_)) {
        rulerUnit = rulerUnit_;
        update();
    }
}

void QDRuler::SetRulerZoom(const qreal rulerZoom_)
{
    if (!qFuzzyCompare(rulerZoom, rulerZoom_)) {
        rulerZoom = rulerZoom_;
        update();
    }
}

void QDRuler::SetCursorPos(const QPoint cursorPos_)
{
    cursorPos = cursorPos_; //this->mapFromGlobal(cursorPos_);
    //cursorPos += QPoint(RulerBreadth, RulerBreadth);
    update();
}

void QDRuler::SetMouseTrack(const bool track)
{
    if (mouseTracking != track) {
        mouseTracking = track;
        setMouseTracking(mouseTracking);
        update();
    }
}

void QDRuler::mouseMoveEvent(QMouseEvent* event)
{
    cursorPos = event->pos();
    update();
    QWidget::mouseMoveEvent(event);
}

void QDRuler::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    textPen = QPen(Qt::white);
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing); // | QPainter::HighQualityAntialiasing);
    painter.setPen(QPen(Qt::darkGray, 0.0)); // zero width pen is cosmetic pen
    QRectF rulerRect(rect()); // We want to work with floating point, so we are considering the rect as QRectF

    // at first fill the rect
    painter.fillRect(rulerRect, QColor(10, 10, 10));
    if (qFuzzyIsNull(rulerZoom))
        return;

    gridStep = GlobalSettings::gridStep(rulerZoom);

    // drawing a scale of 0.1
    if ((gridStep * rulerZoom) > 35) {
        tickKoef = 0.1;
        drawText = true;
    }
    meterPen = QPen(Qt::darkGray, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 1, static_cast<double>(RulerBreadth) * 0.6);
    drawText = false;

    // drawing a scale of 0.2
    if ((gridStep * rulerZoom) <= 35) {
        tickKoef = 0.5;
        drawText = true;
    }
    meterPen = QPen(Qt::green, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 5, static_cast<double>(RulerBreadth) * 0.3);
    drawText = false;

    // drawing a scale of 1.0
    meterPen = QPen(Qt::red, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 10, static_cast<double>(RulerBreadth) * 0);

    // drawing the current mouse position indicator
    if (mouseTracking) {
        DrawMousePosTick(&painter);
    }

    // drawing no man's land between the ruler & view
    if (/* DISABLES CODE */ (0)) {
        QPointF starPt = Horizontal == rulerType ? rulerRect.bottomLeft() : rulerRect.topRight();
        QPointF endPt = Horizontal == rulerType ? rulerRect.bottomRight() : rulerRect.bottomRight();
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(starPt, endPt);
    }
}

void QDRuler::DrawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin)
{
    // Flagging whether we are horizontal or vertical only to reduce
    // to cheching many times
    bool isHorzRuler = Horizontal == rulerType;

    scaleMeter = scaleMeter * rulerUnit * rulerZoom;

    // Ruler rectangle starting mark
    qreal rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
    // Ruler rectangle ending mark
    qreal rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();

    // Condition A # If origin point is between the start & end mard,
    //we have to draw both from origin to left mark & origin to right mark.
    // Condition B # If origin point is left of the start mark, we have to draw
    // from origin to end mark.
    // Condition C # If origin point is right of the end mark, we have to draw
    // from origin to start mark.
    if (origin >= rulerStartMark && origin <= rulerEndMark) {
        DrawFromOriginTo(painter, rulerRect, origin, rulerEndMark, 0, scaleMeter, startPositoin);
        DrawFromOriginTo(painter, rulerRect, origin, rulerStartMark, 0, -scaleMeter, startPositoin);
    } else if (origin < rulerStartMark) {
        int tickNo = int((rulerStartMark - origin) / scaleMeter);
        DrawFromOriginTo(painter, rulerRect, origin + scaleMeter * tickNo,
            rulerEndMark, tickNo, scaleMeter, startPositoin);
    } else if (origin > rulerEndMark) {
        int tickNo = int((origin - rulerEndMark) / scaleMeter);
        DrawFromOriginTo(painter, rulerRect, origin - scaleMeter * tickNo,
            rulerStartMark, tickNo, -scaleMeter, startPositoin);
    }
}

void QDRuler::DrawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition)
{
    bool isHorzRuler = (Horizontal == rulerType);
    for (qreal current = startMark; (step < 0 ? current >= endMark : current <= endMark); current += step) {
        qreal x1 = isHorzRuler ? current : rulerRect.left() + startPosition;
        qreal y1 = isHorzRuler ? rulerRect.top() : current;
        qreal x2 = isHorzRuler ? current : rulerRect.right();
        qreal y2 = isHorzRuler ? rulerRect.bottom() - startPosition : current;
        painter->setPen(meterPen); // zero width pen is cosmetic pen
        painter->drawLine(QLineF(x1, y1, x2, y2));
        if (drawText) {
            painter->save();
            painter->setPen(textPen); // zero width pen is cosmetic pen
            painter->setFont(font());
            QString number = QString::number((startTickNo * gridStep * tickKoef * (GlobalSettings ::inch() ? 1.0 / 25.4 : 1.0)));
            if (1) {
                if (startTickNo != 0) {
                    if (step > 0.0)
                        number = (isHorzRuler ? "+" : "-") + number;
                    else
                        number = (isHorzRuler ? "-" : "+") + number;
                }
            }
            QRectF textRect(QFontMetricsF(font()).boundingRect(number));
            textRect.setWidth(textRect.width() + 1);
            if (isHorzRuler) {
                painter->translate(x1 + 3, textRect.height() + 6);
                painter->drawText(textRect, number);
            } else {
                painter->translate(textRect.height() - 3, y1 - 3);
                painter->rotate(-90);
                painter->drawText(textRect, number);
            }
            painter->restore();
        }
        ++startTickNo;
    }
}

void QDRuler::DrawMousePosTick(QPainter* painter)
{
    QPoint starPt = cursorPos;
    QPoint endPt;
    if (Horizontal == rulerType) {
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
