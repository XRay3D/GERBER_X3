#include "dxf_line.h"

namespace Dxf {

LINE::LINE(SectionParser* sp)
    : Entity(sp)
{
}

void LINE::draw(const INSERT_ET* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(sp->file, this, { startPoint, endPoint }, {});
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        GraphicObject go(sp->file, this, { startPoint, endPoint }, {});
        attachToLayer(std::move(go));
    }
}

void LINE::parse(CodeData& code)
{
    do {
        data << code;
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

}
