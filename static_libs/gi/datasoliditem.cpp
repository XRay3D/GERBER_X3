// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "datasoliditem.h"

#include "file.h"
#include "graphicsview.h"
#include "scene.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

GiDataSolid::GiDataSolid(Paths& paths, FileInterface* file)
    : GraphicsItem(file)
    , m_paths(paths) {
    for (Path path : qAsConst(m_paths)) {
        if (path.size())
            path.push_back(path.front());
        m_shape.addPolygon(path);
    }
    fillPolygon = m_shape.toFillPolygon();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

GiDataSolid::~GiDataSolid() { }

QRectF GiDataSolid::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath GiDataSolid::shape() const { return m_shape; }

void GiDataSolid::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if (App::scene()->drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    painter->setBrush(m_bodyColor);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(fillPolygon);

    //    m_pen.setWidthF(option->state & QStyle::State_Selected
    //                || option->state & QStyle::State_MouseOver
    //            ? 2.0 * scaleFactor()
    //            : 0);
    m_pen.setColor(m_pathColor);
    painter->strokePath(m_shape, m_pen);
}

int GiDataSolid::type() const { return static_cast<int>(GiType::DataSolid); }

void GiDataSolid::redraw() {
    m_shape = QPainterPath();
    for (Path path : qAsConst(m_paths)) {
        path.push_back(path.front());
        m_shape.addPolygon(path);
    }
    fillPolygon = m_shape.toFillPolygon();
    setPos({ 1, 1 }); // костыли
    setPos({ 0, 0 });
    // update();
}

Paths GiDataSolid::paths(int) const { return m_paths; }

Paths* GiDataSolid::rPaths() { return &m_paths; }

void GiDataSolid::changeColor() {
    //    auto animation = new QPropertyAnimation(this, "bodyColor");
    //    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    animation.setDuration(100);

    animation.setStartValue(m_bodyColor);

    m_bodyColor = m_colorPtr ? *m_colorPtr : m_color;

    switch (colorState) {
    case Default:
        break;
    case Hovered:
        m_bodyColor.setAlpha(255);
        break;
    case Selected:
        m_bodyColor.setAlpha(255);
        break;
    case Hovered | Selected:
        m_bodyColor.setAlpha(255);
        m_bodyColor = m_bodyColor.lighter(150);
        break;
    }

    m_pathColor = m_colorPtr ? *m_colorPtr : m_color;
    m_pathColor.setAlpha(100);
    switch (colorState) {
    case Default:
        //        m_pathColor.setAlpha(100);
        break;
    case Hovered:
        m_pathColor.setAlpha(255);
        //        m_pathColor = m_pathColor.darker(125);
        break;
    case Selected:
        m_pathColor.setAlpha(150);
        break;
    case Hovered | Selected:
        m_pathColor.setAlpha(255);
        m_pathColor = m_pathColor.lighter(150);
        break;
    }

    animation.setEndValue(m_bodyColor);
    animation.start();
}
