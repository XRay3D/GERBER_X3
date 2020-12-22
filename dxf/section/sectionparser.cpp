#include "sectionparser.h"
#include "dxffile.h"
#include <QDebug>
#include <QMetaEnum>

namespace Dxf {

SectionParser::SectionParser(QVector<CodeData>&& data, File* file)
    : data(std::move(data))
    , file(file)
    , type(static_cast<sec>(key(this->data[1])))
{
}

int SectionParser::key(const QString& key)
{
    return staticMetaObject.enumerator(0).keysToValue(key.toLocal8Bit().data());
}
}
