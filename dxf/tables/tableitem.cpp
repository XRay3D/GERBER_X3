#include "tableitem.h"
#include <QMetaEnum>
namespace Dxf {
TableItem::TableItem(SectionParser* sp)
    : sp(sp)
{
}

int TableItem::toType(const QString& key)
{
    return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().toUpper().data());
}
}
