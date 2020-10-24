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
#pragma once
#include <QDataStream>
#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator>>(QDataStream& s, E& e)
{
    qint32 i;
    s >> i;
    e = static_cast<E>(i);
    return s;
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator<<(QDataStream& s, E e)
{
    s << static_cast<qint32>(e);
    return s;
}
#endif


