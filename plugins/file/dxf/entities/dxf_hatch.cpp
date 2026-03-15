/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_hatch.h"
#include "dxf_file.h"

#include <QPolygonF>

namespace Dxf {
Hatch::Hatch(SectionParser* sp)
    : Entity{sp} {
}

Hatch::~Hatch() {
    for(auto edge: edges)
        qDeleteAll(edge);
}

void Hatch::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case SubclassMarker                : break; // 100
        case ElevationPointX               : break; // 10
        case ElevationPointY               : break; // 20
        case ElevationPointZ               : break; // 30
        case ExtrDirectionX                : break; // 210
        case ExtrDirectionY                : break; // 220
        case ExtrDirectionZ                : break; // 230
        case HatchPatternName              : break; // 2
        case SolidFillFlag                 : break; // 70
        case PatternFillColor              : break; // 63
        case AssociativityFlag             : break; // 71
        case NumberOfBoundaryPaths         : break; // 91
        case HatchStyle                    : break; // 75
        case HatchPatternType              : break; // 76
        case HatchPatternAngle             : break; // 52
        case HatchPatternScaleOrSpacing    : break; // 41
        case BoundaryAnnotationFlag        : break; // 73
        case HatchPatternDoubleFlag        : break; // 77
        case NumberOfPatternDefinitionLines: break; // 78
        case PixelSize                     : break; // 47
        case NumberOfSeedPoints            : break; // 98
        case OffsetVector                  : break;                   // 11
        case NumberOfDegenerateBoundaryPaths:
            break; // 99
            // case SeedPointX: break;// 10
            // case SeedPointY: break;// 20
        case IndicatesSolidHatchOrGradient      : break; // 450
        case Zero                               : break; // 451
        case RecordsColors                      : break; // 452
        case NumberOfColors                     : break; // 453
        case RotationAangleInRadiansForGradients: break; // 460
        case GradientDefinition                 : break; // 461
        case ColorTintValueUsedByDialogCode     : break; // 462
        case ReservedForFutureUse               : break;                // 463
        case String:
            break; // 470
            // посипроение контура
        case PathTypeFlag:                         // 92
            pathTypeFlags.emplace_back(int(code)); // PathTypeFlags
            edges.resize(pathTypeFlags.size());
            break;
        case NumberOfEdges: // 93
            if(!edges.size())
                edges.resize(1);
            edges[edges.size() - 1].reserve(int(code));
            break;
        case EdgeType: // 72
            edgeType = code;
            switch(edgeType) {
            case Line: { // 1
                auto line = new LineEdge{edgeType};
                edges[edges.size() - 1].push_back(line);
                for(int i{}; i < 4; ++i) {
                    code = sp->nextCode();
                    switch(code.code()) {
                    case PrimaryX:
                        line->p1.setX(code);
                        continue;
                    case PrimaryY:
                        line->p1.setY(code);
                        continue;
                    case Other1X:
                        line->p2.setX(code);
                        continue;
                    case Other1Y:
                        line->p2.setY(code);
                        continue;
                    }
                }
            } break;
            case CircularArc: // 2
                              // break;
            case EllipticArc: // 3
                              // break;
            case Spline:      // 4
                // break;
                throw DxfObj::tr("Unimplemented edge type in HATCH: %1").arg(edgeType);
            default: throw DxfObj::tr("Unknown edge type in HATCH: %1").arg(edgeType);
            }
            break;
        case NumberOfSourceBoundaryObjects   : referencesToSourceBoundaryObject.reserve(int(code)); break;          // 97
        case ReferenceToSourceBoundaryObjects: referencesToSourceBoundaryObject.emplace_back(code.string()); break; // 330
        default                              : Entity::parse(code); break;
        }
        // Entity::parse(code);
        // DC 5 S//
        // DC 8 S//
        // DC 62 I//
        // DC 92 I
        // DC 93 I
        // DC 72 I
        // DC 21 D
        // DC 97 I
        // DC 330 S//
        // DC 0 S

        // DC 0  S
        // DC 10  D
        // DC 100  S
        // DC 11  D
        // DC 2  S
        // DC 20  D
        // DC 21  D
        // DC 210  D
        // DC 220  D
        // DC 230  D
        // DC 330  S
        // DC 5  S
        // DC 62  I
        // DC 70  I
        // DC 71  I
        // DC 72  I
        // DC 75  I
        // DC 76  I
        // DC 8  S
        // DC 91  I
        // DC 92  I
        // DC 93  I
        // DC 97  I
        // DC 98  I
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Hatch::type() const { return Type::HATCH; }

DxfGo Hatch::toGo() const {
    qInfo("Hatch");
    Paths paths(edges.size());
    for(size_t i{}; i < edges.size(); ++i)
        for(auto edge: edges[i])
            paths[i] += ~edge->toPolygon();
    Clipper clipper;
    clipper.AddOpenSubject(paths); // FIXME AddSubject???
    clipper.Execute(ClipType::Union, FillRule::EvenOdd, paths);
    // dbgPaths(paths, referencesToSourceBoundaryObject.front(), true);
    return {id, {} /*edges.size() == 1 ? paths[0] : Path()*/, paths};
}

void Hatch::write(QDataStream& stream) const {
    // stream << edges;

    stream << referencesToSourceBoundaryObject; // Ссылка на исходные объекты контура (несколько записей)

    stream << centerPoint;
    stream << pathTypeFlags;
    stream << edgeType;
    stream << thickness;
    stream << radius;
}

void Hatch::read(QDataStream& stream) {
    // stream >> edges;

    stream >> referencesToSourceBoundaryObject; // Ссылка на исходные объекты контура (несколько записей)

    stream >> centerPoint;
    stream >> pathTypeFlags;
    stream >> edgeType;
    stream >> thickness;
    stream >> radius;
}

} // namespace Dxf
