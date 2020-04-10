#pragma once
#ifndef GBRCOMPONENT_H
#define GBRCOMPONENT_H

#include "gbrattributes.h"
#include <QMap>
#include <QObject>
#include <QPolygonF>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QVector>

namespace Gerber {

class Component {
    Q_GADGET
public:
    Component() {}
    enum MountType {
        TH,
        SMD,
        Fiducial,
        Other
    };
    Q_ENUM(MountType)
    bool setMountType(const QString& key)
    {
        int val = staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data());
        mount = static_cast<MountType>(val);
        return val > -1 ? true : false;
    }

    enum e1 {
        N, /*
        The CAD net name of a conducting object, e.g. Clk13. 5.6.13
        Graphics Object*/
        P, /*
        The pin number (or name) and reference descriptor of a component pad on an outer layer, e.g. IC3,7. 5.6.14
        Graphics Object*/
        C,
        /* <refdes>
        <refdes>=<field>
        This is an already existing attribute. See section 5.6.15
        in the main specification for more information. It
        identifies the component reference descriptor.
        .CRot,<decimal> The rotation angle of the component.
        The rotation angle is consistent with the one for graphics
        objects. Positive rotation is counterclockwise viewed
        from the top side, even if the component is on the
        bottom side. The zero-rotation orientation of a top side
        component as in IPC-7351. The base orientation of a
        bottom side component is the one on the top side,
        mirrored around the X axis.
        The rotation is around the flash point.
        The component reference designator linked to an object, e.g C2. 5.6.15
        Graphics Object*/
    };
    Q_ENUM(e1)
    static int value1(const QString& key) { return staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().mid(0, 1).data()); }

    enum eC {
        Rot, /* <decimal> The rotation angle of the component.
        The rotation angle is consistent with the one for graphics
        objects. Positive rotation is counterclockwise viewed
        from the top side, even if the component is on the
        bottom side. The zero-rotation orientation of a top side
        component as in IPC-7351. The base orientation of a
        bottom side component is the one on the top side,
        mirrored around the X axis.
        The rotation is around the flash point.*/
        Mfr, /* <field> Manufacturer. */
        MPN, /* <field> Manufacturer part number. */
        Val, /* <field> E.g. 220nF. */
        Mnt, /* (TH|SMD|BGA|Other) Mount type. */
        Ftp, /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
        PgN, /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
        PgD, /* <field> Package description. */
        Hgt, /* <decimal> Height, in the unit of the file. */
        LbN, /* <field> Library name. */
        LbD, /* <field> Library description. */
        Sup, /* <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field with a supplier part name*/
    };
    Q_ENUM(eC)
    static int value2(const QString& key) { return staticMetaObject.enumerator(2).keyToValue(key.toLocal8Bit().mid(1).data()); }

    bool setData(int key, const QStringList& data)
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
    QString toolTip() const
    {
        QString tt;
        tt += QString("Rotation: %1\n").arg(rotation);
        tt += QString("Value: %1\n").arg(value);
        tt += QString("Footprint: %1\n").arg(footprintName);
        return tt;
    }

    double rotation = 0.0; /* <decimal> The rotation angle of the component.*/
    double height = 0.0; /* <decimal> Height, in the unit of the file. */

    MountType mount = Other; /* (TH|SMD|BGA|Other) Mount type. */

    QString footprintName; /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
    QString refdes;
    QString value; /* <field> E.g. 220nF. */

    QPointF referencePoint;
    QPolygonF footprint;

    struct Library {
        QString name; /* <field> Library name. */
        QString description; /* <field> Library description. */
    } library;
    struct Manufacturer {
        QString name; /* <field> Manufacturer. */
        QString partNumber; /* <field> Manufacturer part number. */
    } manufacturer;
    struct Package {
        QString name; /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
        QString description; /* <field> Package description. */
    } package;
    struct Supplier {
        QString name; /* <field> Library name. */
        QString description; /* <field> Library description. */
    };
    QList<Supplier> suppliers; /* <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field with a supplier part name*/
    struct Pins {
        int number;
        QString description;
        QPointF pos;
    };
    QList<Pins> pins;
};
}
#endif // GBRCOMPONENT_H
