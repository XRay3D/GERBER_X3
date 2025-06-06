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
// Based on https://kernelcoder.wordpress.com/tag/ruler-in-qgraphicsview/
#pragma once
#include <QPen>
#include <QWidget>

class Ruler final : public QWidget {
    Q_OBJECT
    //    Q_ENUMS(RulerType)
    //    Q_PROPERTY(double origin READ origin WRITE setOrigin)
    //    Q_PROPERTY(double rulerUnit READ rulerUnit WRITE setRulerUnit)
    //    Q_PROPERTY(double rulerZoom READ rulerZoom WRITE setRulerZoom)
public:
    static constexpr int Breadth = 24;

    explicit Ruler(Qt::Orientation rulerType, QWidget* parent);

    Qt::Orientation orientation() const { return orientation_; }
    double origin() const { return origin_; }
    double unit() const { return rulerUnit_; }
    double zoom() const { return rulerZoom_; }
    QSize minimumSizeHint() const override { return {Breadth, Breadth}; }

    static constexpr auto MimeType = "image/x-puzzle-piece";

public slots:
    void setCursorPos(const QPoint& cursorPos_);
    void setMouseTrack(const bool track);
    void setOrigin(const double newOrigin);
    void setRulerUnit(const double rulerUnit_);
    void setRulerZoom(const double rulerZoom_);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

private:
    void DrawAScaleMeter(QPainter* painter, const QRectF& rulerRect,
        double scaleMeter, double startPositoin);
    void DrawFromOriginTo(QPainter* painter, const QRectF& rulerRect,
        double startMark, double endMark,
        int startTickNo, double step, double startPosition);
    void DrawMousePosTick(QPainter* painter);

    double gridStep{1.0};
    double origin_{};
    double rulerUnit_{1.0};
    double rulerZoom_{1.0};
    double tickKoef{1.0};

    QPoint cursorPos;

    // QPen meterPen;

    const Qt::Orientation orientation_;

    bool drawText{};
    bool mouseTracking{};
};
