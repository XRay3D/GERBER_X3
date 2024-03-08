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
#include "dxf_sectionparser.h"
#include "dxf_file.h"
#include <QDebug>
#include <QMetaEnum>

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"

namespace Dxf {

SectionParser::SectionParser(Codes::iterator from, Codes::iterator to, Codes::iterator it, File* file)
    : type(toType(*(from + 1)))
    , from(from)
    , to(to)
    , it(it)
    , file(file) {
}
SectionParser::SectionParser(Codes::iterator from, Codes::iterator to, File* file)
    : type(toType(*(from + 1)))
    , from(from)
    , to(to)
    , it(from)
    , file(file) {
}

SectionParser::SectionType SectionParser::toType(const QString& key) {
    return static_cast<SectionType>(staticMetaObject
                                        .enumerator(0)
                                        .keysToValue(key.toUtf8().data()));
}

const CodeData& SectionParser::nextCode() const {
    auto& val = *it;
    ++it;
    return val;
    return *(++it);
}

const CodeData& SectionParser::prevCode() const {
    --it;
    auto& val = *it;
    return val;
    return *(--it);
}

bool SectionParser::hasNext() const { return std::distance(it, to) > 0; }

bool SectionParser::hasPrev() const { return std::distance(from, it) > 0; }

QDebug operator<<(QDebug debug, const SectionParser& c) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "\rSEC(" << c.type << ", " << std::distance(c.from, c.to) << ')';
    return debug;
}

} // namespace Dxf

#include "moc_dxf_sectionparser.cpp"
