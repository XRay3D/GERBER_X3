#include "dxf_abstracttable.h"
#include <QMetaEnum>

namespace Dxf {

AbstractTable::AbstractTable(SectionParser* sp)
    : sp(sp)
{
}

AbstractTable::Type AbstractTable::toType(const QString& key)
{
    return static_cast<Type>(staticMetaObject
                                 .enumerator(0)
                                 .keyToValue(key.toLocal8Bit().toUpper().data()));
}

}
