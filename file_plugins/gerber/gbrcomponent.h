/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once
#include "datastream.h"
#include "gbrattributes.h"
#include <QMap>
#include <QObject>
#include <QPolygonF>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QVector>

namespace Gerber {

class ComponentItem;

struct Library {
    QString name;        /* <field> Library name. */
    QString description; /* <field> Library description. */
    friend QDataStream& operator<<(QDataStream& stream, const Library& l) {
        stream << l.name << l.description;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Library& l) {
        stream >> l.name >> l.description;
        return stream;
    }
};
struct Manufacturer {
    QString name;       /* <field> Manufacturer. */
    QString partNumber; /* <field> Manufacturer part number. */
    friend QDataStream& operator<<(QDataStream& stream, const Manufacturer& m) {
        stream << m.name << m.partNumber;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Manufacturer& m) {
        stream >> m.name >> m.partNumber;
        return stream;
    }
};
struct Package {
    QString name;        /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
    QString description; /* <field> Package description. */
    friend QDataStream& operator<<(QDataStream& stream, const Package& p) {
        stream << p.name << p.description;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Package& p) {
        stream >> p.name >> p.description;
        return stream;
    }
};
struct Pin {
    /*
     *  <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name.
     *  <SPN> is a field with a supplier part name
     */
    QString number;
    QString description;
    QPointF pos;
    friend QDataStream& operator<<(QDataStream& stream, const Pin& p) {
        stream << p.number << p.description << p.pos;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Pin& p) {
        stream >> p.number >> p.description >> p.pos;
        return stream;
    }
};
struct Supplier {
    QString name;        /* <field> Library name. */
    QString description; /* <field> Library description. */
    friend QDataStream& operator<<(QDataStream& stream, const Supplier& s) {
        stream << s.name << s.description;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Supplier& s) {
        stream >> s.name >> s.description;
        return stream;
    }
};

class Component {
    Q_GADGET
    //friend ComponentItem;
    friend QDataStream& operator<<(QDataStream& stream, const Component& c);
    friend QDataStream& operator>>(QDataStream& stream, Component& c);

public:
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

    ComponentItem* componentitem() const { return m_componentitem; }
    void setComponentitem(ComponentItem* componentitem) const { m_componentitem = componentitem; }

    Library library() const { return m_library; }
    void setLibrary(const Library& library) { m_library = library; }

    Manufacturer manufacturer() const { return m_manufacturer; }
    void setManufacturer(const Manufacturer& manufacturer) { m_manufacturer = manufacturer; }

    MountType mount() const { return m_mount; }
    void setMount(const MountType& mount) { m_mount = mount; }

    Package package() const { return m_package; }
    void setPackage(const Package& package) { m_package = package; }

    mvector<Pin> pins() const { return m_pins; }
    mvector<Pin>& pins() { return m_pins; }
    void addPin(Pin&& pins) { m_pins.emplace_back(pins); }

    mvector<Supplier> suppliers() const { return m_suppliers; }
    void setSuppliers(const mvector<Supplier>& suppliers) { m_suppliers = suppliers; }

    QPointF referencePoint() const { return m_referencePoint; }
    void setReferencePoint(const QPointF& referencePoint) { m_referencePoint = referencePoint; }

    mvector<QPolygonF> footprint() const { return m_footprint; }
    void addFootprint(const QPolygonF& footprint) { m_footprint.emplace_back(footprint); }

    QString footprintName() const { return m_footprintName; }
    void setFootprintName(const QString& footprintName) { m_footprintName = footprintName; }

    const QString& refdes() const { return m_refdes; }
    void setRefdes(const QString& refdes) { m_isNull = true, m_refdes = refdes; }

    QString value() const { return m_value; }
    void setValue(const QString& value) { m_value = value; }

    double height() const { return m_height; }
    void setHeight(double height) { m_height = height; }

    double rotation() const { return m_rotation; }
    void setRotation(double rotation) { m_rotation = rotation; }

    bool isNull() const { return m_isNull; }

private:
    double m_rotation = 0.0; /* <decimal> The rotation angle of the component.*/
    double m_height = 0.0;   /* <decimal> Height, in the unit of the file. */
    mutable ComponentItem* m_componentitem = nullptr;
    Library m_library;
    Manufacturer m_manufacturer;
    MountType m_mount = Other; /* (TH|SMD|BGA|Other) Mount type. */
    Package m_package;
    mvector<Pin> m_pins;
    mvector<Supplier> m_suppliers;
    QPointF m_referencePoint;
    mvector<QPolygonF> m_footprint;
    QString m_footprintName; /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
    QString m_refdes;
    QString m_value; /* <field> E.g. 220nF. */
    bool m_isNull = true;
};
} // namespace Gerber
