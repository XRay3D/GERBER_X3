// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
*******************************************************************************/

#include "graphicsitem.h"
#include "abstractfile.h"
#include "itemgroup.h"
#include <QTimer>

#include "boost/pfr/precise.hpp"
#include <iostream>

GraphicsItem::GraphicsItem(AbstractFile* file)
    : animation(this, "bodyColor")
    , m_file(file)
    , m_pen(QPen(Qt::white, 0.0))
    , m_colorPtr(file ? &file->color() : nullptr)
    , m_color(Qt::white)
    , m_bodyColor(m_colorPtr ? *m_colorPtr : m_color)
    , m_pathColor(Qt::transparent)
{

    //    using namespace boost::pfr::ops; // out-of-the-box ostream operator for all PODs!
    //    struct my_struct { // no ostream operator defined!
    //        int i;
    //        char c;
    //        double d;
    //    };
    //    my_struct s { 100, 'H', 3.141593 };
    //    std::cout << "my_struct has " << boost::pfr::tuple_size<my_struct>::value
    //              << " fields: " << s << "\n";
    //    struct some_person {
    //        std::string name;
    //        unsigned birth_year;
    //    };
    //    some_person val { "Edgar Allan Poe", 1809 };
    //    std::cout << boost::pfr::get<0>(val) // No macro!
    //              << " was born in " << boost::pfr::get<1>(val); // Works with any aggregate initializables!

    animation.setDuration(100);
    animation.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    connect(this, &GraphicsItem::colorChanged, [this] { /*qDebug(__FUNCTION__);*/ update(); });
    QGraphicsItem::setVisible(false);
}

void GraphicsItem::setVisible(bool visible)
{
    auto visibleA = new QPropertyAnimation(this, "opacity");
    visibleA->setDuration(100);
    visibleA->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    visibleA->setStartValue(visible ? 0.0 : 1.0);
    visibleA->setEndValue(visible ? 1.0 : 0.0);
    visibleA->start();
    if (visible)
        QGraphicsObject::setVisible(visible);
    else
        connect(visibleA, &QAbstractAnimation::finished, [visible, this] { QGraphicsObject::setVisible(visible); });
}

const AbstractFile* GraphicsItem::file() const { return m_file; }

int GraphicsItem::id() const { return m_id; }

void GraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant GraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        const bool fl = value.toInt();
        fl ? colorState |= Selected : colorState &= ~Selected;
        changeColor();
    } else if (change == ItemSceneChange) {
        //qDebug() << value;
    }
    return QGraphicsItem::itemChange(change, value);
}
