// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrcomponent.h"
#include "gbrtypes.h"

namespace Gerber {

bool Component::setMountType(const QString& key)
{
    int val = staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data());
    m_mount = static_cast<MountType>(val);
    return val > -1 ? true : false;
}

int Component::value1(const QString& key) { return staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().mid(0, 1).data()); }

int Component::value2(const QString& key) { return staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().mid(1).data()); }

bool Component::setData(int key, const QStringList& data)
{
    bool fl = false;
    switch (key) {
    case Component::Rot:
        m_rotation = data.last().toDouble(&fl);
        return fl;
    case Component::Mfr:
        m_manufacturer.name = data.last();
        return true;
    case Component::MPN:
        m_manufacturer.partNumber = data.last();
        return true;
    case Component::Val:
        m_value = data.last();
        return true;
    case Component::Mnt:
        return setMountType(data.last());
    case Component::Ftp:
        m_footprintName = data.last();
        return true;
    case Component::PgN:
        m_package.name = data.last();
        return true;
    case Component::Hgt:
        m_height = data.last().toDouble(&fl);
        return fl;
    case Component::LbN:
        m_library.name = data.last();
        return true;
    case Component::LbD:
        m_library.description = data.last();
        return true;
    case Component::Sup:
        return false;
    default:
        return false;
    }
    return fl;
}

QString Component::toolTip() const
{
    QString tt;
    tt += QString(GbrObj::tr("Rotation: %1\n")).arg(m_rotation);
    tt += QString(GbrObj::tr("Value: %1\n")).arg(m_value);
    tt += QString(GbrObj::tr("Footprint: %1\n")).arg(m_footprintName);
    return tt;
}

QDataStream& operator<<(QDataStream& stream, const Component& c)
{
    stream << c.m_rotation;
    stream << c.m_height;
    stream << c.m_mount;
    stream << c.m_footprintName;
    stream << c.m_refdes;
    stream << c.m_value;
    stream << c.m_referencePoint;
    stream << c.m_footprint;
    stream << c.m_library;
    stream << c.m_manufacturer;
    stream << c.m_package;
    stream << c.m_suppliers;
    stream << c.m_pins;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Component& c)
{
    stream >> c.m_rotation;
    stream >> c.m_height;
    stream >> c.m_mount;
    stream >> c.m_footprintName;
    stream >> c.m_refdes;
    stream >> c.m_value;
    stream >> c.m_referencePoint;
    stream >> c.m_footprint;
    stream >> c.m_library;
    stream >> c.m_manufacturer;
    stream >> c.m_package;
    stream >> c.m_suppliers;
    stream >> c.m_pins;
    return stream;
}

}
