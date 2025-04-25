/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbrcomp_onent.h"
#include "gbr_types.h"

namespace Gerber::Comp {

bool Component::setMountType(const QString& key) {
    int val = staticMetaObject.enumerator(0).keyToValue(key.toUtf8().data());
    mount_ = static_cast<MountType>(val);
    return val > -1 ? true : false;
}

int Component::value1(const QString& key) { return staticMetaObject.enumerator(1).keyToValue(key.toUtf8().mid(0, 1).data()); }

int Component::value2(const QString& key) { return staticMetaObject.enumerator(2).keyToValue(key.toUtf8().mid(1).data()); }

bool Component::setData(int key, const QStringList& data) {
    bool fl = false;
    switch(key) {
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
    return ::Block(stream).write(
        c.rotation_,
        c.height_,
        c.mount_,
        c.footprintName_,
        c.refdes_,
        c.value_,
        c.referencePoint_,
        c.footprint_,
        c.library_,
        c.manufacturer_,
        c.package_,
        c.suppliers_,
        c.pins_);
}

QDataStream& operator>>(QDataStream& stream, Component& c) {
    return ::Block(stream).read(
        c.rotation_,
        c.height_,
        c.mount_,
        c.footprintName_,
        c.refdes_,
        c.value_,
        c.referencePoint_,
        c.footprint_,
        c.library_,
        c.manufacturer_,
        c.package_,
        c.suppliers_,
        c.pins_);
}

} // namespace Gerber::Comp

#include "moc_gbrcomp_onent.cpp"
