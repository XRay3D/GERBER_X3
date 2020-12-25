#include "dxf_point.h"

namespace Dxf {

Point::Point(SectionParser* sp)
    : Entity(sp)
{
}

void Point::draw(const Dxf::InsertEntity* const i) const
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

void Point::parse(Dxf::CodeData& code)
{
    do {
        data.push_back(code);
        switch (static_cast<VarType>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case PointX:
            point.rx() = code;
            break;
        case PointY:
            point.ry() = code;
            break;
        case PointZ:
            break;
        case ExtrusionDirectionX:
        case ExtrusionDirectionY:
        case ExtrusionDirectionZ:
            break;
        case AngleOfTheXZxisForTheUCS:
            break;
        default:
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

GraphicObject Point::toGo() const
{
    QPolygonF p;
    p.append(point);

    return { sp->file, this, p, {} };
}

}
