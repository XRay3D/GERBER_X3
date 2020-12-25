#include "dxf_line.h"

#include <QPainterPath>

namespace Dxf {

Line::Line(SectionParser* sp)
    : Entity(sp)
{
}

void Line::draw(const InsertEntity* const i) const
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

void Line::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (static_cast<VarType>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case StartPointX:
            startPoint.rx() = code;
            break;
        case StartPointY:
            startPoint.ry() = code;
            break;
        case StartPointZ:
            break;
        case EndPointX:
            endPoint.rx() = code;
            break;
        case EndPointY:
            endPoint.ry() = code;
            break;
        case EndPointZ:
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

GraphicObject Line::toGo() const
{
    QPolygonF p;
    if (p.isEmpty()) {
        p.append(startPoint);
        p.append(endPoint);
    }

    Paths paths;
    //    ClipperOffset offset;
    //    offset.AddPath(p, jtRound, etOpenRound);
    //    offset.Execute(paths, thickness * uScale);

    return { sp->file, this, p, paths };
}

}
