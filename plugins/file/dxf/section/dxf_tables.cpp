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
#include "dxf_tables.h"
#include "tables/dxf_abstracttable.h"

namespace Dxf {

SectionTABLES::SectionTABLES(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , layers(file->layers()) {
    parse();
}

SectionTABLES::~SectionTABLES() {
    for (const auto& [k, v] : tables) {
        (void)k;
        qDeleteAll(v);
    }
}

void SectionTABLES::parse() {
    do {
        CodeData code(nextCode());
        if (code == "TABLE") {
            code = nextCode();
            do {
                auto type = AbstractTable::toType(code);
                switch (type) {
                case AbstractTable::APPID:
                    // tables[type].append(new AppId(this));
                    break;
                case AbstractTable::BLOCK_RECORD:
                    // tables[type].append(new BlockRecord(this));
                    break;
                case AbstractTable::DIMSTYLE:
                    // tables[type].append(new DimStyle(this));
                    break;
                case AbstractTable::LAYER:
                    tables[type].append(new Layer(this));
                    break;
                case AbstractTable::LTYPE:
                    // tables[type].append(new LType(this));
                    break;
                case AbstractTable::STYLE:
                    tables[type].append(new Style(this));
                    break;
                case AbstractTable::UCS:
                    // tables[type].append(new UCS(this));
                    break;
                case AbstractTable::VIEW:
                    // tables[type].append(new View(this));
                    break;
                case AbstractTable::VPORT:
                    // tables[type].append(new VPort(this));
                    break;
                }
                if (tables.contains(type)) {
                    tables[type].last()->parse(code);
                    if (type == AbstractTable::LAYER) {
                        auto layer = static_cast<Layer*>(tables[type].last());
                        layers[layer->name()] = layer;
                    }
                } else {
                    // skip unimplemented tables
                    do {
                        code = nextCode();
                    } while (code.code() != 0);
                }
            } while (code != "ENDTAB");
        }
    } while (hasNext());
}

} // namespace Dxf
