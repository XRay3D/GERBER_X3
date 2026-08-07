/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
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
    parse();
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
    } while(code != u"ENDBLK"_s);
}

SectionENTITIES::~SectionENTITIES() {
}

void SectionENTITIES::parse() {
    CodeData code = nextCode();
    code          = nextCode();
    code          = nextCode();
    // К этому моменту file->entities_ уже может содержать сущности, разобранные
    // при чтении секции BLOCKS (они хранятся там же, чтобы id оставался индексом
    // в общем векторе). Их рисовать здесь нельзя: они будут отрисованы через
    // InsertEntity::draw() с нужным смещением/масштабом/поворотом, когда до них
    // дойдёт очередь при разборе соответствующего INSERT. Иначе геометрия блока
    // рисуется ещё и напрямую, в исходных координатах блока с масштабом 1:1.
    const size_t first = file->entities_.size();
    do {
        file->entities_.emplace_back(entityParse(code));
        file->entities_.back()->parse(code);
        file->entities_.back()->id = file->entities_.size() - 1;
    } while(hasNext());
    for(size_t i = first; i < file->entities_.size(); ++i) {
        Entity* entity = file->entities_[i].get();
        // 3D-геометрия не рисуется в слой напрямую: она копится в Model3D файла
        // и превращается в проекционные слои-силуэты (File::createProjectionLayers).
        if(auto pl = dynamic_cast<PolyLine*>(entity); pl && pl->isMesh())
            file->mesh_.addPolyLine(*pl);
        else if(auto face = dynamic_cast<Face3D*>(entity); face && face->is3D())
            file->mesh_.addFace3D(*face);
        else
            entity->draw();
    }
}

std::shared_ptr<Entity> SectionENTITIES::entityParse(CodeData& code) {
    key = Entity::toType(code);
    return createEntity(key, blocks, sp);
}

} // namespace Dxf
