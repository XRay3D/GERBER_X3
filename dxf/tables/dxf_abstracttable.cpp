// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
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
