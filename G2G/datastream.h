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


