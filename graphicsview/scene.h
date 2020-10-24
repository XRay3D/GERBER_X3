/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
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

    bool drawPdf();

    void setCross1(const QPointF& cross);
    void setCross2(const QPointF& cross2);
    void setDrawRuller(bool drawRuller);

private:
    bool m_drawPdf = false;
    bool m_drawRuller = false;
    QPointF m_cross1;
    QPointF m_cross2;
    double m_scale = std::numeric_limits<double>::max();
    QRectF m_rect;
    QMap<long, long> hGrid;
    QMap<long, long> vGrid;
    void drawRuller(QPainter* painter);

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#include "app.h"
