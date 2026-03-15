/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_abstracttable.h"
#include <QMetaEnum>

namespace Dxf {

AbstractTable::AbstractTable(SectionParser* sp)
    : sp{sp} {
}

void AbstractTable::parse(CodeData& code) {
    switch(code.code()) {
    case EntityName     : break; // -1
    case EntityType     : break; // 0
    case HandleAllExcept: break; // 5
    case HandleTableOnly:
        break; // 105
        // case StartOfApplicationDefinedGroup: // 102
        // break;
        // case EndOfGroup: // 102
        // break;
    case IndicatesTheStartOfTheAutocadPersistentReactorsGroup: break; // 102
    case HandleToOwnerDictionary:
        break; // 330
        // case IndicatesEndOfGroup: // 102
        // break;
        // case StartOfAnExtensionDictionaryGrou: // 102
        // break;
    case Hard_OwnerID_HandleToOwnerDictionary:
        break; // 360
        // case DictionaryEndOfGroup: // 102
        // break;
        // case SoftPointerID_HandleToOwnerObject: // 330
        // break;
    case SubclassMarker: break; // 100
    default            : break;
    }
}

AbstractTable::Type AbstractTable::toType(const QString& key) {
    return static_cast<Type>(staticMetaObject
            .enumerator(0)
            .keyToValue(key.toUtf8().toUpper().data()));
}

} // namespace Dxf

#include "moc_dxf_abstracttable.cpp"
