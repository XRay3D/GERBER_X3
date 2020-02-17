#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <QDataStream>
#include <type_traits>

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

#endif // DATASTREAM_H
