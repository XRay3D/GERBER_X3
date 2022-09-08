// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_entities.h"
#include "dxf_file.h"
#include "entities/dxf_allentities.h"

#include <QGraphicsView>
#include <QTimer>

namespace Dxf {
SectionENTITIES::SectionENTITIES(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , sp(this)
    , blocks(file->blocks()) {
}

SectionENTITIES::SectionENTITIES(Blocks& blocks, CodeData& code, SectionParser* sp)
    : SectionParser(sp->from, sp->to, sp->it, sp->file)
    , sp(sp)
    , blocks(blocks) {
    do {
        file->entities_.emplace_back(entityParse(code));
        file->entities_.back()->parse(code);
        file->entities_.back()->id = file->entities_.size() - 1;
        entities.push_back(file->entities_.back().get());
    } while (code != "ENDBLK");
}

SectionENTITIES::~SectionENTITIES() {
}

void SectionENTITIES::parse() {
    CodeData code = nextCode();
    code = nextCode();
    code = nextCode();
    do {
        file->entities_.emplace_back(entityParse(code));
        file->entities_.back()->parse(code);
        file->entities_.back()->id = file->entities_.size() - 1;
    } while (hasNext());
    for (auto& e : qAsConst(file->entities_))
        e->draw();
}

std::shared_ptr<Entity> SectionENTITIES::entityParse(CodeData& code) {
    key = Entity::toType(code);

    return createEntity(key, blocks, sp);

    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        break; // return std::make_shared<ACADProxyEntity>(sp);
    case Entity::ARC:
        return std::make_shared<Arc>(sp);
    case Entity::ATTDEF:
        return std::make_shared<AttDef>(sp);
    case Entity::ATTRIB:
        return std::make_shared<Attrib>(sp);
    case Entity::BODY:
        return std::make_shared<Body>(sp);
    case Entity::CIRCLE:
        return std::make_shared<Circle>(sp);
    case Entity::DIMENSION:
        return std::make_shared<Dummy /*Dimension*/>(sp);
    case Entity::ELLIPSE:
        return std::make_shared<Ellipse>(sp);
    case Entity::HATCH:
        return std::make_shared<Hatch>(sp);
    case Entity::HELIX:
        return std::make_shared<Helix>(sp);
    case Entity::IMAGE:
        return std::make_shared<Image>(sp);
    case Entity::INSERT:
        return std::make_shared<InsertEntity>(blocks, sp);
    case Entity::LEADER:
        return std::make_shared<Leader>(sp);
    case Entity::LIGHT:
        return std::make_shared<Light>(sp);
    case Entity::LINE:
        return std::make_shared<Line>(sp);
    case Entity::LWPOLYLINE:
        return std::make_shared<LwPolyline>(sp);
    case Entity::MESH:
        return std::make_shared<Mesh>(sp);
    case Entity::MLEADER:
        return std::make_shared<MLeader>(sp);
    case Entity::MLEADERSTYLE:
        return std::make_shared<MLeaderStyle>(sp);
    case Entity::MLINE:
        return std::make_shared<MLine>(sp);
    case Entity::MTEXT:
        return std::make_shared<MText>(sp);
    case Entity::OLE2FRAME:
        return std::make_shared<Ole2Frame>(sp);
    case Entity::OLEFRAME:
        return std::make_shared<OleFrame>(sp);
    case Entity::POINT:
        return std::make_shared<Point>(sp);
    case Entity::POLYLINE:
        return std::make_shared<PolyLine>(sp);
    case Entity::RAY:
        return std::make_shared<Ray>(sp);
    case Entity::REGION:
        return std::make_shared<Region>(sp);
    case Entity::SECTION:
        return std::make_shared<Section>(sp);
    case Entity::SEQEND:
        return std::make_shared<SeqEnd>(sp);
    case Entity::SHAPE:
        return std::make_shared<Shape>(sp);
    case Entity::SOLID:
        return std::make_shared<Solid>(sp);
    case Entity::SPLINE:
        return std::make_shared<Spline>(sp);
    case Entity::SUN:
        return std::make_shared<Sun>(sp);
    case Entity::SURFACE:
        return std::make_shared<Surface>(sp);
    case Entity::TABLE:
        return std::make_shared<Table>(sp);
    case Entity::TEXT:
        return std::make_shared<Text>(sp);
    case Entity::TOLERANCE:
        return std::make_shared<Tolerance>(sp);
    case Entity::TRACE:
        return std::make_shared<Trace>(sp);
    case Entity::UNDERLAY:
        return std::make_shared<Underlay>(sp);
    case Entity::VERTEX:
        return std::make_shared<Vertex>(sp);
    case Entity::VIEWPORT:
        return std::make_shared<Viewport>(sp);
    case Entity::WIPEOUT:
        return std::make_shared<Dummy /*Wipeout*/>(sp);
    case Entity::XLINE:
        return std::make_shared<XLine>(sp);
    default:
        throw DxfObj::tr("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
    }
    throw DxfObj::tr("Not implemented: %1, %2").arg(key).arg(code.operator QString());
}

} // namespace Dxf
