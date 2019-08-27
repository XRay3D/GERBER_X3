#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <QDataStream>
#include <type_traits>

template <typename E, typename = std::enable_if_t<std::is_enum<E>::value>>
inline QDataStream& operator>>(QDataStream& s, E& e)
{
    int i;
    s >> i;
    e = static_cast<E>(i);
    return s;
}

template <typename E, typename = std::enable_if_t<std::is_enum<E>::value>>
inline QDataStream& operator<<(QDataStream& s, E e)
{
    s << static_cast<int>(e);
    return s;
}

#endif // DATASTREAM_H
