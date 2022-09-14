// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// https://kernelcoder.wordpress.com/tag/ruler-in-qgraphicsview/
#include "ruler.h"

#include "app.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QTextDocument>
#include <QTextFormat>
#include <QtMath>

Ruler::Ruler(Ruler::Type rulerType, QWidget* parent)
    : QWidget {parent}
    , rulerType {rulerType} {
    setMouseTracking(mouseTracking);
    // QFont txtFont("Vrinda");
    // txtFont.setStyleHint(QFont::TypeWriter, QFont::PreferOutline);
    // setFont(txtFont);
}

QSize Ruler::minimumSizeHint() const { return QSize(Ruler::Breadth, Ruler::Breadth); }

Ruler::Type Ruler::RulerType() const { return rulerType; }

double Ruler::Origin() const { return origin; }

double Ruler::RulerUnit() const { return rulerUnit; }

double Ruler::RulerZoom() const { return rulerZoom; }

void Ruler::SetOrigin(const double origin_) {
    if (!qFuzzyCompare(origin, origin_)) {
        origin = origin_;
        update();
    }
}

void Ruler::SetRulerUnit(const double rulerUnit_) {
    if (!qFuzzyCompare(rulerUnit, rulerUnit_)) {
        rulerUnit = rulerUnit_;
        update();
    }
}

void Ruler::SetRulerZoom(const double rulerZoom_) {
    if (!qFuzzyCompare(rulerZoom, rulerZoom_)) {
        rulerZoom = rulerZoom_;
        update();
    }
}

void Ruler::SetCursorPos(const QPoint cursorPos_) {
    cursorPos = cursorPos_; // this->mapFromGlobal(cursorPos_);
    // cursorPos += QPoint(RulerBreadth, RulerBreadth);
    update();
}

void Ruler::SetMouseTrack(const bool track) {
    if (mouseTracking != track) {
        mouseTracking = track;
        setMouseTracking(mouseTracking);
        update();
    }
}

void Ruler::mouseMoveEvent(QMouseEvent* event) {
    cursorPos = event->pos();
    update();
    QWidget::mouseMoveEvent(event);
}

void Ruler::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing); // | QPainter::HighQualityAntialiasing);
    painter.setPen(QPen(Qt::darkGray, 0.0));            // zero width pen is cosmetic pen
    QRectF rulerRect(rect());                           // We want to work with floating point, so we are considering the rect as QRectF

    // at first fill the rect
    painter.fillRect(rulerRect, App::settings().guiColor(GuiColors::Background));
    if (qFuzzyIsNull(rulerZoom))
        return;

    gridStep = App::settings().gridStep(rulerZoom);

    // drawing a scale of 0.1
    if ((gridStep * rulerZoom) > 35) {
        tickKoef = 0.1;
        drawText = true;
    }
    meterPen = QPen(Qt::darkGray, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 1, static_cast<double>(Ruler::Breadth) * 0.6);
    drawText = false;

    // drawing a scale of 0.2
    if ((gridStep * rulerZoom) <= 35) {
        tickKoef = 0.5;
        drawText = true;
    }
    meterPen = QPen(Qt::green, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 5, static_cast<double>(Ruler::Breadth) * 0.3);
    drawText = false;

    // drawing a scale of 1.0
    meterPen = QPen(Qt::red, 0.0);
    DrawAScaleMeter(&painter, rulerRect, gridStep * 10, static_cast<double>(Ruler::Breadth) * 0);

    // drawing the current mouse position indicator
    if (mouseTracking) {
        DrawMousePosTick(&painter);
    }

    // drawing no man's land between the ruler & view
    if (/* NOTE DISABLES CODE */ (0)) {
        QPointF starPt((Horizontal == rulerType) ? rulerRect.bottomLeft() : rulerRect.topRight());
        QPointF endPt((Horizontal == rulerType) ? rulerRect.bottomRight() : rulerRect.bottomRight()); // FIXME same branches!!!!!!
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(starPt, endPt);
    }
}

void Ruler::DrawAScaleMeter(QPainter* painter, QRectF rulerRect, double scaleMeter, double startPositoin) {
    // Flagging whether we are horizontal or vertical only to reduce
    // to cheching many times
    bool isHorzRuler = Horizontal == rulerType;

    scaleMeter = scaleMeter * rulerUnit * rulerZoom;

    // Ruler rectangle starting mark
    double rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
    // Ruler rectangle ending mark
    double rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();
    /*
    Condition A # If origin point is between the start & end mard, we have to draw both from origin to left mark & origin to right mark.
    Condition B # If origin point is left of the start mark, we have to draw from origin to end mark.
    Condition C # If origin point is right of the end mark, we have to draw from origin to start mark.
    */
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

void Ruler::DrawFromOriginTo(QPainter* painter, QRectF rect, double startMark, double endMark, int startTickNo, double step, double startPosition) {
    const auto isHorzRuler = (Horizontal == rulerType);
    const auto K = gridStep * tickKoef * (App::settings().inch() ? 1.0 / 25.4 : 1.0);

    QColor color(0xFFFFFFFF - App::settings().guiColor(GuiColors::Background).rgb());

    painter->setPen(QPen(color, 0.0));
    painter->setFont(font());

    mvector<QLineF> lines;
    lines.reserve(abs(ceil((endMark - startMark) / step)));

    constexpr double padding = 3;

    for (double current = startMark; (step < 0 ? current >= endMark : current <= endMark); current += step) {
        double x1, y1;
        lines.emplace_back(
            x1 = isHorzRuler ? current : rect.left() + startPosition,
            y1 = isHorzRuler ? rect.top() : current,
            /*x2*/ isHorzRuler ? current : rect.right(),
            /*y2*/ isHorzRuler ? rect.bottom() - startPosition : current);
        if (drawText) [[unlikely]] {
            painter->save();
            auto number {QString::number(startTickNo * K)};

            if (startTickNo) [[likely]]
                number = ((isHorzRuler ^ (step > 0.0)) ? "-" : "+") + number;

            QRectF textRect(QFontMetricsF(font()).boundingRect(number));
            textRect.setWidth(textRect.width() + 1);
            if (isHorzRuler) {
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
    painter->setPen(meterPen); // zero width pen is cosmetic pen
    painter->drawLines(lines.data(), lines.size());
}

void Ruler::DrawMousePosTick(QPainter* painter) {
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

#include "moc_ruler.cpp"
