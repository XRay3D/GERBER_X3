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

#include "graphicsitem.h"

struct Row;

class AbstractDrillPrGI : public QGraphicsObject {
    Q_OBJECT

    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor pathColor READ pathColor WRITE setPathColor NOTIFY colorChanged FINAL)

    // clang-format off
    QColor bodyColor() { return m_bodyColor; }
    void setBodyColor(const QColor& c) { m_bodyColor = c; colorChanged();  }
    QColor pathColor() { return m_pathColor; }
    void setPathColor(const QColor& c) { m_pathColor = c; colorChanged(); }
    // clang-format on
#ifdef GBR_
    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
#endif
    friend class ThermalNode;

signals:
    void colorChanged();

public:
    AbstractDrillPrGI(int id, Row& row);
#ifdef GBR_
    explicit DrillPrGI(const Gerber::GraphicObject* go, int id, Row& row);
#endif
#ifdef EX_
    explicit DrillPrGI(const Excellon::Hole* hole, Row& row);
#endif
    ~AbstractDrillPrGI() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;

    //////////////////////////////////////////
    int type() const override;
    double sourceDiameter() const;
    int toolId() const;
    virtual void updateTool() = 0;
    //    {
    //    if (row.toolId > -1)
    //        colorState |= Tool;
    //    else
    //        colorState &= ~Tool;

    //    if (row.toolId > -1) {
    //        m_toolPath = QPainterPath(); //.clear();
    //        const double diameter = App::toolHolder().tool(row.toolId).diameter();
    //        const double lineKoeff = diameter * 0.7;
    //        switch (m_type) {
    //        case GiType::SlotPr: {
    //            Paths tmpPpath;
    //            ClipperOffset offset;
    //            offset.AddPath(hole->item->paths().first(), jtRound, etOpenRound);
    //            offset.Execute(tmpPpath, diameter * 0.5 * uScale);
    //            for (Path& path : tmpPpath) {
    //                path.push_back(path.first());
    //                m_toolPath.addPolygon(path);
    //            }
    //            Path path(hole->item->paths().first());
    //            if (path.size()) {
    //                for (Point64& pt : path) {
    //                    m_toolPath.moveTo(pt - QPointF(0.0, lineKoeff));
    //                    m_toolPath.lineTo(pt + QPointF(0.0, lineKoeff));
    //                    m_toolPath.moveTo(pt - QPointF(lineKoeff, 0.0));
    //                    m_toolPath.lineTo(pt + QPointF(lineKoeff, 0.0));
    //                }
    //                m_toolPath.moveTo(path.first());
    //                for (Point64& pt : path) {
    //                    m_toolPath.lineTo(pt);
    //                }
    //            }
    //        } break;
    //        case GiType::DrillPr: {
    //            const QPointF offsetedPos(hole->state.offsetedPos());
    //            m_toolPath.addEllipse(offsetedPos, diameter * 0.5, diameter * 0.5);
    //            m_toolPath.moveTo(offsetedPos - QPointF(0.0, lineKoeff));
    //            m_toolPath.lineTo(offsetedPos + QPointF(0.0, lineKoeff));
    //            m_toolPath.moveTo(offsetedPos - QPointF(lineKoeff, 0.0));
    //            m_toolPath.lineTo(offsetedPos + QPointF(lineKoeff, 0.0));
    //        } break;
    //#ifdef GBR_
    //        case GiType::ApetrurePr: {
    //            const QPointF curPos(gbrObj->state().curPos());
    //            m_toolPath.addEllipse(curPos, diameter * 0.5, diameter * 0.5);
    //            m_toolPath.moveTo(curPos - QPointF(0.0, lineKoeff));
    //            m_toolPath.lineTo(curPos + QPointF(0.0, lineKoeff));
    //            m_toolPath.moveTo(curPos - QPointF(lineKoeff, 0.0));
    //            m_toolPath.lineTo(curPos + QPointF(lineKoeff, 0.0));
    //        } break;
    //#endif
    //        default:
    //            break;
    //        }
    //    }
    //        changeColor();
    //    }
    virtual Point64 pos() const = 0;
    //    {
    //        switch (m_type) {
    //        //    case GiType::SlotPr:
    //        //        return hole->state.offsetedPos();
    //        //    case GiType::DrillPr:
    //        //        return hole->state.offsetedPos();
    //        //#ifdef GBR_
    //        //    case GiType::ApetrurePr:
    //        //        return gbrObj->state().curPos();
    //        //#endif
    //        default:
    //            return {};
    //        }
    //    }
    virtual Paths paths() const = 0;
    //    {
    //        switch (m_type) {
    //        //    case GiType::SlotPr:
    //        //        return hole->item->paths();
    //        //    case GiType::DrillPr: {
    //        //        Paths paths(hole->item->paths());
    //        //        return ReversePaths(paths);
    //        //    }
    //        //#ifdef GBR_
    //        //    case GiType::ApetrurePr:
    //        //        return gbrObj->paths();
    //        //#endif
    //        default:
    //            return {};
    //        }
    //    }
    virtual bool fit(double depth) = 0;
    //    {
    //        switch (m_type) {
    //        //    case GiType::SlotPr:
    //        //    case GiType::DrillPr:
    //        //        return m_sourceDiameter > App::toolHolder().tool(row.toolId).getDiameter(depth);
    //        //#ifdef GBR_
    //        //    case GiType::ApetrurePr:
    //        //        return gbrObj->gFile()->apertures()->at(id)->fit(App::toolHolder().tool(row.toolId).getDiameter(depth));
    //        //#endif
    //        default:
    //            return false;
    //        }
    //    }

    void changeColor();

protected:
#ifdef GBR_
    static QPainterPath drawApetrure(const Gerber::GraphicObject* go, int id);
#endif
#ifdef EX_
    static QPainterPath drawDrill(const Excellon::Hole* hole);
    static QPainterPath drawSlot(const Excellon::Hole* hole);
#endif

    struct Row& row;

    const int id = 0;
#ifdef GBR_
    const Gerber::GraphicObject* const gbrObj = nullptr;
#endif
#ifdef EX_
    const Excellon::Hole* const hole = nullptr;
#endif

    QPainterPath m_sourcePath;

    double m_sourceDiameter;
    GiType m_type;

    QColor m_bodyColor;
    QColor m_pathColor;

    enum class Colors : int {
        Default,
        DefaultHovered,
        Selected,
        SelectedHovered,
        Used,
        UsedHovered,
        UnUsed,
        Tool,
    };

    static constexpr int dark = 180;
    static constexpr int light = 255;
    inline static constexpr QRgb colors[] {
        qRgba(128, 128, 128, dark), //  Default         gray dark
        qRgba(255, 255, 255, light), // DefaultHovered  gray light
        qRgba(0, 255, 0, dark), //      Selected        green dark
        qRgba(0, 255, 0, light), //     SelectedHovered green light
        qRgba(255, 0, 0, dark), //      Used            red dark
        qRgba(255, 0, 0, light), //     UsedHovered     red light
        qRgba(255, 255, 255, /*0*/ 255), //     UnUsed          transparent
        qRgba(255, 255, 255, dark), //  Tool            white
    };

    enum ColorState {
        Default,
        Hovered = 1,
        Selected = 2,
        Used = 4,
        Tool = 8,
    };
    int colorState = Default;

    // QGraphicsItem interface
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

//namespace Gerber {

//class DrillPrGI : public AbstractDrillPrGI {
//    Q_OBJECT

//#ifdef GBR_
//    static QPainterPath drawPoly(const Gerber::GraphicObject& go);
//#endif

//public:
//#ifdef GBR_
//    explicit DrillPrGI(const Gerber::GraphicObject* go, int id, Row& row);
//#endif
//#ifdef EX_
//    explicit DrillPrGI(const Excellon::Hole* hole, Row& row);
//#endif

//private:
//#ifdef GBR_
//    static QPainterPath drawApetrure(const Gerber::GraphicObject* go, int id);
//#endif
//#ifdef EX_
//    static QPainterPath drawDrill(const Excellon::Hole* hole);
//    static QPainterPath drawSlot(const Excellon::Hole* hole);
//#endif

//#ifdef GBR_
//    const Gerber::GraphicObject* const gbrObj = nullptr;
//#endif
//#ifdef EX_
//    const Excellon::Hole* const hole = nullptr;
//#endif
//};

//}
