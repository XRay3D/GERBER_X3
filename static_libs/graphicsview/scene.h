/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QGraphicsItem>
#include <QGraphicsScene>

class Scene : public QGraphicsScene {
    friend class MainWindow;

public:
    explicit Scene(QObject* parent = nullptr);
    ~Scene() override;

    void RenderPdf();

    QRectF itemsBoundingRect();
    QRectF getSelectedBoundingRect();

    bool drawPdf() const { return m_drawPdf; }
    bool boundingRect() const { return m_boundingRect; }

    void setCross1(const QPointF& cross);
    void setCross2(const QPointF& cross2);
    void setDrawRuller(bool drawRuller);

private:
    bool m_drawPdf = false;
    bool m_drawRuller = false;
    bool m_boundingRect = false;
    QPointF m_cross1;
    QPointF m_cross2;
    double m_scale = std::numeric_limits<double>::max();
    QRectF m_rect;
    QMap<long, long> hGrid;
    QMap<long, long> vGrid;
    void drawRuller(QPainter* painter);

    int fpsCtr {};
    int currentFps {};

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    // QObject interface
    void timerEvent(QTimerEvent* event) override;
};

#include "app.h"
