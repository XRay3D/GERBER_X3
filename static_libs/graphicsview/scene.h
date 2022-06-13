/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include <QGraphicsItem>
#include <QGraphicsScene>
#include "app.h"

class Scene : public QGraphicsScene {
    friend class MainWindow;

public:
    explicit Scene(QObject* parent = nullptr);
    ~Scene() override;

    void renderPdf();

    QRectF itemsBoundingRect();
    QRectF getSelectedBoundingRect();

    bool drawPdf() const { return drawPdf_; }
    bool boundingRect() const { return boundingRect_; }

    void setCross1(const QPointF& cross);
    void setCross2(const QPointF& cross);
    void setDrawRuller(bool drawRuller);

private:
    QMap<long, long> hGrid;
    QMap<long, long> vGrid;
    QPointF cross1;
    QPointF cross2;
    QRectF lastRect;
    double scale = std::numeric_limits<double>::max();

    int fpsCtr {};
    int currentFps {};

    bool drawRuller_ {};
    bool boundingRect_ {};
    bool drawPdf_ {};

    void drawRuller(QPainter* painter);
    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    // QObject interface
    void timerEvent(QTimerEvent* event) override;
};
