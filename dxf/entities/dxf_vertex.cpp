#include "dxf_vertex.h"

namespace Dxf {

VERTEX_::VERTEX_(SectionParser* sp)
    : Entity(sp)
{
}

void VERTEX_::draw(const Dxf::INSERT_ET* const i) const
{
}

void VERTEX_::parse(Dxf::CodeData& code)
{
    do {
        data << code;
        switch (static_cast<VarType>(code.code())) {
        case SubclassMarker: // Маркер подкласса (AcDbVertex)
            break;
        case PointX: // Точка местоположения (ОСК в 2D-среде, МСК в 3D-среде)
            x = code;
            break;
        case PointY: // Файл DXF: значение X; приложение: 3D-точка
            y = code;
            break;
        case PointZ: // Файл DXF: значения Y и Z для точки местоположения (ОСК в 2D-среде, МСК в 3D-среде)
            break;
        case StartingWidth: // Начальная ширина (необязательно; значение по умолчанию — 0)
            break;
        case EndingWidth: // Конечная ширина (необязательно; значение по умолчанию — 0)
            break;
        case Bulge: // Прогиб (необязательно; значение по умолчанию — 0). Прогиб — это касательная одной четвертой центрального угла к дуговому сегменту. Если дуга идет в направлении по часовой стрелке от начальной точки к конечной, то значение касательной будет отрицательным. Если значение прогиба равно 0, то сегмент прямой, а если 1, то полукруглый.
            bulge = code;
            break;
        case VertexFlags: // Флаги вершин:
            vertexFlags = code;
        case CurveFitTangentDirection: // Направление касательной с дуговым сглаживанием
            curveFitTangentDirection = code;
        case PolyfaceMeshVertexIndex1:
        case PolyfaceMeshVertexIndex2:
        case PolyfaceMeshVertexIndex3:
        case PolyfaceMeshVertexIndex4:
        case VertexIdentifier:
            break;
        default:
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

}
