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

#include <graphicsitem.h>

class AbstractDrillItem : public GraphicsItem {
public:
    //    DrillItem(Excellon::Hole* hole, Excellon::File* file);
    //    DrillItem(double diameter, GCode::File* file);
    AbstractDrillItem(FileInterface* file = nullptr);
    ~AbstractDrillItem() override = default;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; // QGraphicsItem
    int type() const override;

    // GraphicsItem interface
    void changeColor() override;

    double diameter() const;
    void setDiameter(double diameter);

    virtual bool isSlot() = 0;
    virtual void updateHole() {};

protected:
    virtual void create() = 0;
    double m_diameter = 0.0;
};



//namespace Excellon {

//class File;
//class Hole;

//class DrillItem : public AbstractDrillItem {
//public:
//    DrillItem(double diameter, GCode::File* file);
//    ~DrillItem() override;
//    // QGraphicsItem interface
//    QRectF boundingRect() const override; // QGraphicsItem
//    QPainterPath shape() const override; // QGraphicsItem
//    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override; //QGraphicsItem
//    int type() const override;
//    // GraphicsItem interface
//    Paths paths(int alternate = {}) const override;
//    void changeColor() override;

//    bool isSlot() override; // AbstractDrillItem
//    double diameter() const;
//    void setDiameter(double diameter);
//    void updateHole() override; //AbstractDrillItem

//private:
//    void create() override; // AbstractDrillItem
//    double m_diameter = 0.0;
//    Excellon::Hole* const m_hole = nullptr;
//    QPolygonF fillPolygon;
//};

//}
