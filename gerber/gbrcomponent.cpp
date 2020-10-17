// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrcomponent.h"



Gerber::Component::Component() {}

bool Gerber::Component::setMountType(const QString &key)
{
    int val = staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data());
    mount = static_cast<MountType>(val);
    return val > -1 ? true : false;
}

int Gerber::Component::value1(const QString &key) { return staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().mid(0, 1).data()); }

int Gerber::Component::value2(const QString &key) { return staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().mid(1).data()); }

bool Gerber::Component::setData(int key, const QStringList &data)
{
    bool fl = false;
    switch (key) {
    case Component::Rot:
        rotation = data.last().toDouble(&fl);
        break;
    case Component::Mfr:
        manufacturer.name = data.last();
        break;
    case Component::MPN:
        manufacturer.partNumber = data.last();
        break;
    case Component::Val:
        value = data.last();
        break;
    case Component::Mnt:
        return setMountType(data.last());
    case Component::Ftp:
        footprintName = data.last();
        break;
    case Component::PgN:
        package.name = data.last();
        break;
    case Component::Hgt:
        height = data.last().toDouble(&fl);
        break;
    case Component::LbN:
        library.name = data.last();
        break;
    case Component::LbD:
        library.description = data.last();
        break;
    case Component::Sup:
        break;
    default:;
    }
    return fl;
}

QString Gerber::Component::toolTip() const
{
    QString tt;
    tt += QString("Rotation: %1\n").arg(rotation);
    tt += QString("Value: %1\n").arg(value);
    tt += QString("Footprint: %1\n").arg(footprintName);
    return tt;
}


