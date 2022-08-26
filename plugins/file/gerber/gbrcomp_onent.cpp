// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbrcomp_onent.h"
#include "gbr_types.h"

namespace Gerber {

bool Component::setMountType(const QString& key) {
    int val = staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data());
    mount_ = static_cast<MountType>(val);
    return val > -1 ? true : false;
}

int Component::value1(const QString& key) { return staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().mid(0, 1).data()); }

int Component::value2(const QString& key) { return staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().mid(1).data()); }

bool Component::setData(int key, const QStringList& data) {
    bool fl = false;
    switch (key) {
    case Component::Rot:
        rotation_ = data.last().toDouble(&fl);
        return fl;
    case Component::Mfr:
        manufacturer_.name = data.last();
        return true;
    case Component::MPN:
        manufacturer_.partNumber = data.last();
        return true;
    case Component::Val:
        value_ = data.last();
        return true;
    case Component::Mnt:
        return setMountType(data.last());
    case Component::Ftp:
        footprintName_ = data.last();
        return true;
    case Component::PgN:
        package_.name = data.last();
        return true;
    case Component::Hgt:
        height_ = data.last().toDouble(&fl);
        return fl;
    case Component::LbN:
        library_.name = data.last();
        return true;
    case Component::LbD:
        library_.description = data.last();
        return true;
    case Component::Sup:
        return false;
    default:
        return false;
    }
    return fl;
}

QString Component::toolTip() const {
    QString tt;
    tt += QString(GbrObj::tr("Rotation: %1\n")).arg(rotation_);
    tt += QString(GbrObj::tr("Value: %1\n")).arg(value_);
    tt += QString(GbrObj::tr("Footprint: %1\n")).arg(footprintName_);
    return tt;
}

QDataStream& operator<<(QDataStream& stream, const Component& c) {
    stream << c.rotation_;
    stream << c.height_;
    stream << c.mount_;
    stream << c.footprintName_;
    stream << c.refdes_;
    stream << c.value_;
    stream << c.referencePoint_;
    stream << c.footprint_;
    stream << c.library_;
    stream << c.manufacturer_;
    stream << c.package_;
    stream << c.suppliers_;
    stream << c.pins_;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Component& c) {
    stream >> c.rotation_;
    stream >> c.height_;
    stream >> c.mount_;
    stream >> c.footprintName_;
    stream >> c.refdes_;
    stream >> c.value_;
    stream >> c.referencePoint_;
    stream >> c.footprint_;
    stream >> c.library_;
    stream >> c.manufacturer_;
    stream >> c.package_;
    stream >> c.suppliers_;
    stream >> c.pins_;
    return stream;
}

} // namespace Gerber
