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
    case EntityName: // -1
        break;
    case EntityType: // 0
        break;
    case HandleAllExcept: // 5
        break;
    case HandleTableOnly: // 105
        break;
        //    case StartOfApplicationDefinedGroup: // 102
        //        break;
        //    case EndOfGroup: // 102
        //        break;
    case IndicatesTheStartOfTheAutocadPersistentReactorsGroup: // 102
        break;
    case HandleToOwnerDictionary: // 330
        break;
        //    case IndicatesEndOfGroup: // 102
        //        break;
        //    case StartOfAnExtensionDictionaryGrou: // 102
        //        break;
    case Hard_OwnerID_HandleToOwnerDictionary: // 360
        break;
        //    case DictionaryEndOfGroup: // 102
        //        break;
        //    case SoftPointerID_HandleToOwnerObject: // 330
        //        break;
    case SubclassMarker: // 100
        break;
    default:
        break;
    }
}

AbstractTable::Type AbstractTable::toType(const QString& key) {
    return static_cast<Type>(staticMetaObject
            .enumerator(0)
            .keyToValue(key.toUtf8().toUpper().data()));
}

} // namespace Dxf

#include "moc_dxf_abstracttable.cpp"
