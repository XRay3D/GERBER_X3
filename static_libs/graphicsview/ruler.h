/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once
#include <QPen>
#include <QWidget>

class Ruler : public QWidget {
    Q_OBJECT
    //    Q_ENUMS(RulerType)
    //    Q_PROPERTY(double origin READ origin WRITE setOrigin)
    //    Q_PROPERTY(double rulerUnit READ rulerUnit WRITE setRulerUnit)
    //    Q_PROPERTY(double rulerZoom READ rulerZoom WRITE setRulerZoom)
public:
    enum Type {
        Horizontal,
        Vertical
    };

    enum { Breadth = 24 };

    Ruler(Ruler::Type rulerType, QWidget* parent);
    Ruler::Type RulerType() const;
    double Origin() const;
    double RulerUnit() const;
    double RulerZoom() const;
    QSize minimumSizeHint() const;

public slots:
    void SetCursorPos(const QPoint cursorPos_);
    void SetMouseTrack(const bool track);
    void SetOrigin(const double origin_);
    void SetRulerUnit(const double rulerUnit_);
    void SetRulerZoom(const double rulerZoom_);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    void DrawAScaleMeter(QPainter* painter, QRectF rulerRect, double scaleMeter, double startPositoin);
    void DrawFromOriginTo(QPainter* painter, QRectF rulerRect, double startMark, double endMark, int startTickNo, double step, double startPosition);
    void DrawMousePosTick(QPainter* painter);

    bool drawText {};
    bool mouseTracking {};

    QPen meterPen;

    QPoint cursorPos;

    double gridStep { 1.0 };
    double origin {};
    double rulerUnit { 1.0 };
    double rulerZoom { 1.0 };
    double tickKoef { 1.0 };
    const Type rulerType;
};
