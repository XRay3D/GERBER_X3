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
#pragma once
#include <QPen>
#include <QWidget>

class Ruler : public QWidget {
    Q_OBJECT
    //    Q_ENUMS(RulerType)
    //    Q_PROPERTY(qreal origin READ origin WRITE setOrigin)
    //    Q_PROPERTY(qreal rulerUnit READ rulerUnit WRITE setRulerUnit)
    //    Q_PROPERTY(qreal rulerZoom READ rulerZoom WRITE setRulerZoom)
public:
    enum Type {
        Horizontal,
        Vertical
    };

    enum { Breadth = 24 };

    Ruler(Ruler::Type rulerType, QWidget* parent);
    Ruler::Type RulerType() const;
    qreal Origin() const;
    qreal RulerUnit() const;
    qreal RulerZoom() const;
    QSize minimumSizeHint() const;

public slots:
    void SetCursorPos(const QPoint cursorPos_);
    void SetMouseTrack(const bool track);
    void SetOrigin(const qreal origin_);
    void SetRulerUnit(const qreal rulerUnit_);
    void SetRulerZoom(const qreal rulerZoom_);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    void DrawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin);
    void DrawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition);
    void DrawMousePosTick(QPainter* painter);

    bool drawText;
    bool mouseTracking;

    QPen meterPen;

    QPoint cursorPos;

    qreal gridStep;
    qreal origin;
    qreal rulerUnit;
    qreal rulerZoom;
    qreal tickKoef;
    Type rulerType;
};
