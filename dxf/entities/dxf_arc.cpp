#include "dxf_arc.h"
#include "dxf_insert.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "settings.h"
#include <QGraphicsEllipseItem>

#include <QPainter>

namespace Dxf {

Arc::Arc(SectionParser* sp)
    : Entity(sp)
{
}

void Arc::draw(const InsertEntity* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(toGo());
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        attachToLayer(toGo());
    }
}

void Arc::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (static_cast<VarType>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case CenterPointX:
            centerPoint.rx() = code;
            break;
        case CenterPointY:
            centerPoint.ry() = code;
            break;
        case CenterPointZ:
            break;
        case Radius:
            radius = code;
            break;
        case StartAngle:
            startAngle = code;
            break;
        case EndAngle:
            endAngle = code;
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

GraphicObject Arc::toGo() const
{
    if (qFuzzyIsNull(radius) || (qFuzzyCompare(startAngle, endAngle)))
        return { sp->file, this, {}, {} };

    double aspan = endAngle - startAngle;

    if (const bool ccw = endAngle > startAngle;
        endAngle >= 0 && startAngle >= 0) {
        if (!ccw)
            aspan += 360;
    } else {
        if (aspan < -180 || (qFuzzyCompare(aspan, -180) && !ccw))
            aspan += 360;
        else if (aspan > 180 || (qFuzzyCompare(aspan, 180) && ccw))
            aspan -= 360;
    }

    QPainterPath path;
    path.moveTo(QLineF::fromPolar(radius, -startAngle).translated(centerPoint).p2());
    QPointF rad(radius, radius);
    path.arcTo(QRectF(centerPoint - rad, centerPoint + rad), -startAngle, -aspan);
    QMatrix m;
    m.scale(1000, 1000);

    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);

    QMatrix m2;
    m2.scale(0.001, 0.001);
    auto p(path2.toSubpathPolygons(m2).first());

    return { sp->file, this, p, {} };
}

}
