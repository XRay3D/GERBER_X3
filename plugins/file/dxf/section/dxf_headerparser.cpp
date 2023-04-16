// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "dxf_headerparser.h"
#include "dxf_file.h"
#include <QTreeWidget>

namespace Dxf {
SectionHEADER::SectionHEADER(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , header(file->header()) {
    parse();
}

void SectionHEADER::parse() {
    QString key;
    do {
        CodeData code(nextCode());
        if(code == "ENDSEC")
            continue;
        if(code.type() == CodeData::String && code.string().startsWith('$'))
            key = code.string();
        else if(!key.isEmpty())
            header[key][code.code()] = code.value();
    } while(hasNext());
}

} // namespace Dxf
