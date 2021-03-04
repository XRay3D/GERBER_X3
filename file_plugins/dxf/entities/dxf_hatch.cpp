// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_hatch.h"
#include "dxf_file.h"

#include <QPolygonF>

namespace Dxf {
Hatch::Hatch(SectionParser* sp)
    : Entity(sp)
{
}

Hatch::~Hatch()
{
    for (auto edge : edges)
        qDeleteAll(edge);
}

//void Hatch::draw(const InsertEntity* const i) const
//{
//    if (i) {
//        for (int r = 0; r < i->rowCount; ++r) {
//            for (int c = 0; c < i->colCount; ++c) {
//                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//                GraphicObject go(toGo());
//                i->transform(go, tr);
//                i->attachToLayer(std::move(go));
//            }
//        }
//    } else {
//        attachToLayer(toGo());
//    }
//}

void Hatch::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarker: // 100
            break;
        case ElevationPointX: // 10
            break;
        case ElevationPointY: // 20
            break;
        case ElevationPointZ: // 30
            break;
        case ExtrDirectionX: // 210
            break;
        case ExtrDirectionY: // 220
            break;
        case ExtrDirectionZ: // 230
            break;
        case HatchPatternName: // 2
            break;
        case SolidFillFlag: // 70
            break;
        case PatternFillColor: // 63
            break;
        case AssociativityFlag: // 71
            break;
        case NumberOfBoundaryPaths: // 91
            break;
        case HatchStyle: // 75
            break;
        case HatchPatternType: // 76
            break;
        case HatchPatternAngle: // 52
            break;
        case HatchPatternScaleOrSpacing: // 41
            break;
        case BoundaryAnnotationFlag: // 73
            break;
        case HatchPatternDoubleFlag: // 77
            break;
        case NumberOfPatternDefinitionLines: // 78
            break;
        case PixelSize: // 47
            break;
        case NumberOfSeedPoints: // 98
            break;
        case OffsetVector: // 11
            break;
        case NumberOfDegenerateBoundaryPaths: // 99
            break;
            //        case SeedPointX: // 10
            //            break;
            //        case SeedPointY: // 20
            //            break;
        case IndicatesSolidHatchOrGradient: // 450
            break;
        case Zero: // 451
            break;
        case RecordsColors: // 452
            break;
        case NumberOfColors: // 453
            break;
        case RotationAangleInRadiansForGradients: // 460
            break;
        case GradientDefinition: // 461
            break;
        case ColorTintValueUsedByDialogCode: // 462
            break;
        case ReservedForFutureUse: // 463
            break;
        case String: // 470
            break;
            // посипроение контура
        case PathTypeFlag: // 92
            pathTypeFlags.emplace_back(int(code)); // PathTypeFlags
            edges.resize(pathTypeFlags.size());
            break;
        case NumberOfEdges: // 93
            if (!edges.size())
                edges.resize(1);
            edges[edges.size() - 1].reserve(int(code));
            break;
        case EdgeType: // 72
            edgeType = code;
            switch (edgeType) {
            case Line: { // 1
                auto line = new LineEdge(edgeType);
                edges[edges.size() - 1].push_back(line);
                for (int i = 0; i < 4; ++i) {
                    code = sp->nextCode();
                    switch (code.code()) {
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
                //break;
            case EllipticArc: // 3
                //break;
            case Spline: // 4
                //break;
                throw DxfObj::tr("Unimplemented edge type in HATCH: %1").arg(edgeType);
            default:
                throw DxfObj::tr("Unknown edge type in HATCH: %1").arg(edgeType);
            }
            break;
        case NumberOfSourceBoundaryObjects: // 97
            referencesToSourceBoundaryObject.reserve(int(code));
            break;
        case ReferenceToSourceBoundaryObjects: // 330
            referencesToSourceBoundaryObject.emplace_back(code.string());
            break;
        default:
            Entity::parse(code);
            break;
        }
        // Entity::parse(code);
        //        DC	5	S//
        //        DC	8	S//
        //        DC	62	I//
        //        DC	92	I
        //        DC	93	I
        //        DC	72	I
        //        DC	21	D
        //        DC	97	I
        //        DC	330	S//
        //        DC	0	S

        //        DC	0		S
        //        DC	10		D
        //        DC	100		S
        //        DC	11		D
        //        DC	2		S
        //        DC	20		D
        //        DC	21		D
        //        DC	210		D
        //        DC	220		D
        //        DC	230		D
        //        DC	330		S
        //        DC	5		S
        //        DC	62		I
        //        DC	70		I
        //        DC	71		I
        //        DC	72		I
        //        DC	75		I
        //        DC	76		I
        //        DC	8		S
        //        DC	91		I
        //        DC	92		I
        //        DC	93		I
        //        DC	97		I
        //        DC	98		I
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type Hatch::type() const { return Type::HATCH; }

GraphicObject Hatch::toGo() const
{
    Paths paths(edges.size());
    for (size_t i = 0; i < edges.size(); ++i)
        for (auto edge : edges[i])
            paths[i].append(Path(edge->toPolygon()));
    Clipper clipper;
    clipper.AddPaths(paths, ptSubject);
    clipper.Execute(ctUnion, paths, pftEvenOdd);
    //dbgPaths(paths, referencesToSourceBoundaryObject.front(), true);
    return { this, {} /*edges.size() == 1 ? paths[0] : Path()*/, paths };
}

void Hatch::write(QDataStream& stream) const
{
    //    stream << edges;

    stream << referencesToSourceBoundaryObject; // Ссылка на исходные объекты контура (несколько записей)

    stream << centerPoint;
    stream << pathTypeFlags;
    stream << edgeType;
    stream << thickness;
    stream << radius;
}

void Hatch::read(QDataStream& stream)
{
    //    stream >> edges;

    stream >> referencesToSourceBoundaryObject; // Ссылка на исходные объекты контура (несколько записей)

    stream >> centerPoint;
    stream >> pathTypeFlags;
    stream >> edgeType;
    stream >> thickness;
    stream >> radius;
}
}
