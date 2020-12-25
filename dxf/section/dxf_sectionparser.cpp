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
    , file(file)
{
    qDebug() << type;
}
SectionParser::SectionParser(Codes::iterator from, Codes::iterator to, File* file)
    : type(toType(*(from + 1)))
    , from(from)
    , to(to)
    , it(from)
    , file(file)
{
    qDebug() << type;
}

SectionParser::SectionType SectionParser::toType(const QString& key)
{
    return static_cast<SectionType>(staticMetaObject
                                        .enumerator(0)
                                        .keysToValue(key.toLocal8Bit().data()));
}

const CodeData& SectionParser::nextCode() const
{
    auto& val = *it;
    ++it;
    return val;
}

const CodeData& SectionParser::prevCode() const
{
    --it;
    auto& val = *it;
    return val;
}

bool SectionParser::hasNext() const
{
    //qDebug() << "hasNext" << std::distance(it, to) << std::distance(from, to);
    return std::distance(it, to) > 0;
}

bool SectionParser::hasPrev() const
{
    //qDebug() << "hasPrev" << std::distance(from, it) << std::distance(from, to);
    return std::distance(from, it) > 0;
}

QDebug operator<<(QDebug debug, const SectionParser& c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "\rSEC(" << c.type << ", " << std::distance(c.from, c.to) << ')';
    return debug;
}

}
