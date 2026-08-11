/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "ex_types.h"

#include "abstract_file.h"

namespace Excellon {

class[[= Serial::name("Excellon")]] File : public AbstractFile, public QList<Hole> {
    Tools tools_;
    friend class Parser;
    Format format_;

public:
    explicit File();
    ~File() override;

    uint32_t type() const override { return EXCELLON; }

    double tool(unsigned t) const;
    Tools tools() const;

    Format format() const;
    void setFormat(const Format& value);

    // AbstractFile interface
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    // Отверстия приходят из JSON без обратных связей: указатели на файл и на
    // формат движок не пишет (см. ex_types.h), а createGi ниже по цепочке уже
    // спрашивает у них диаметр текущего инструмента.
    void postLoad() {
        format_.file = this;
        for(Hole& hole: *this) {
            hole.file = this;
            hole.state.format = &format_;
        }
        AbstractFile::postLoad();
    }

    void createGi() override;
    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;
    std::vector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;

    QIcon icon() const override { return QIcon::fromTheme(u"drill-path"_s); }

protected:
    Geo::Polygons merge() const override;
};

} // namespace Excellon
