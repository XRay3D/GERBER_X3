#include "dxf_arc.h"
#include "dxf_insert.h"
#include "settings.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <section/blocks.h>
#include <section/entities.h>

namespace Dxf {

ARC::ARC(SectionParser* sp)
    : Entity(sp)
{
}

void ARC::draw(const INSERT_ET* const i) const
{
    auto arc = [](const QPointF& center, double radius, double start, double stop) -> Path {
        Path points;
        double aspan = stop - start;

        if (const bool ccw = stop > start;
            stop >= 0 && start >= 0) {
            if (!ccw)
                aspan += 360;
        } else {
            if (aspan < -180 || (qFuzzyCompare(aspan, -180) && !ccw))
                aspan += 360;
            else if (aspan > 180 || (qFuzzyCompare(aspan, 180) && ccw))
                aspan -= 360;
        }

        QPainterPath path;
        path.moveTo(QLineF::fromPolar(radius, -start).translated(center).p2());
        QPointF rad(radius, radius);
        path.arcTo(QRectF(center - rad, center + rad), -start, -aspan);
        QMatrix m;
        m.scale(1000, 1000);

        QPainterPath path2;
        for (auto& poly : path.toSubpathPolygons(m))
            path2.addPolygon(poly);

        QMatrix m2;
        m2.scale(0.001, 0.001);
        auto p(path2.toSubpathPolygons(m2).first());

        return p;
    };

    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(sp->file, this, arc(centerPoint, radius, startAngle, endAngle), {});
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        GraphicObject go(sp->file, this, arc(centerPoint, radius, startAngle, endAngle), {});
        attachToLayer(std::move(go));
    }
}

void ARC::parse(CodeData& code)
{
    do {
        data << code;
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
}
