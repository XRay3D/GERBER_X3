#pragma once



#include "gbrattributes.h"
#include <QMap>
#include <QObject>
#include <QPolygonF>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QVector>
#include <datastream.h>

namespace Gerber {

class Component {
    Q_GADGET
public:
    Component();
    enum MountType {
        TH,
        SMD,
        Fiducial,
        Other
    };
    Q_ENUM(MountType)
    bool setMountType(const QString& key);

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
    static int value1(const QString& key);

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
    static int value2(const QString& key);

    bool setData(int key, const QStringList& data);
    QString toolTip() const;

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
        friend QDataStream& operator<<(QDataStream& stream, const Library& l)
        {
            stream << l.name;
            stream << l.description;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Library& l)
        {
            stream >> l.name;
            stream >> l.description;
            return stream;
        }
    } library;
    struct Manufacturer {
        QString name; /* <field> Manufacturer. */
        QString partNumber; /* <field> Manufacturer part number. */
        friend QDataStream& operator<<(QDataStream& stream, const Manufacturer& m)
        {
            stream << m.name;
            stream << m.partNumber;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Manufacturer& m)
        {
            stream >> m.name;
            stream >> m.partNumber;
            return stream;
        }
    } manufacturer;
    struct Package {
        QString name; /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
        QString description; /* <field> Package description. */
        friend QDataStream& operator<<(QDataStream& stream, const Package& p)
        {
            stream << p.name;
            stream << p.description;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Package& p)
        {
            stream >> p.name;
            stream >> p.description;
            return stream;
        }
    } package;
    struct Supplier {
        QString name; /* <field> Library name. */
        QString description; /* <field> Library description. */
        friend QDataStream& operator<<(QDataStream& stream, const Supplier& s)
        {
            stream << s.name;
            stream << s.description;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Supplier& s)
        {
            stream >> s.name;
            stream >> s.description;
            return stream;
        }
    };
    QList<Supplier> suppliers; /* <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field with a supplier part name*/
    struct Pin {
        QString number;
        QString description;
        QPointF pos;
        friend QDataStream& operator<<(QDataStream& stream, const Pin& p)
        {
            stream << p.number;
            stream << p.description;
            stream << p.pos;
            return stream;
        }
        friend QDataStream& operator>>(QDataStream& stream, Pin& p)
        {
            stream >> p.number;
            stream >> p.description;
            stream >> p.pos;
            return stream;
        }
    };

    QList<Pin> pins;

    friend QDataStream& operator<<(QDataStream& stream, const Component& c)
    {
        stream << c.rotation;
        stream << c.height;
        stream << c.mount;
        stream << c.footprintName;
        stream << c.refdes;
        stream << c.value;
        stream << c.referencePoint;
        stream << c.footprint;
        stream << c.library;
        stream << c.manufacturer;
        stream << c.package;
        stream << c.suppliers;
        stream << c.pins;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Component& c)
    {
        stream >> c.rotation;
        stream >> c.height;
        stream >> c.mount;
        stream >> c.footprintName;
        stream >> c.refdes;
        stream >> c.value;
        stream >> c.referencePoint;
        stream >> c.footprint;
        stream >> c.library;
        stream >> c.manufacturer;
        stream >> c.package;
        stream >> c.suppliers;
        stream >> c.pins;
        return stream;
    }
};
}

