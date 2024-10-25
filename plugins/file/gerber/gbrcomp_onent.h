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
#pragma once

#include "arc_solver.h"
#include "datastream.h"
#include "gbr_attributes.h"
#include "mvector.h"
#include <QMap>
#include <QObject>
#include <QPolygonF>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QVector>

namespace Gerber::Comp {

class Item;

struct Library {
    QString name;        /* <field> Library name. */
    QString description; /* <field> Library description. */
    SERIALIZE_POD(Library)
};
struct Manufacturer {
    QString name;       /* <field> Manufacturer. */
    QString partNumber; /* <field> Manufacturer part number. */
    SERIALIZE_POD(Manufacturer)
};
struct Package {
    QString name;        /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
    QString description; /* <field> Package description. */
    SERIALIZE_POD(Package)
};
struct Pin {
    /*
     *  <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name.
     *  <SPN> is a field with a supplier part name
     */
    QString number;
    QString description;
    QPointF pos;
    SERIALIZE_POD(Pin)
};
struct Supplier {
    QString name;        /* <field> Library name. */
    QString description; /* <field> Library description. */
    SERIALIZE_POD(Supplier)
};

class Component {
    Q_GADGET
    // friend Item;
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
        The component reference designator linked to an object, e.g CL2. 5.6.15
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

    Item* componentitem() const { return componentitem_; }
    void setitem(Item* componentitem) const { componentitem_ = componentitem; }

    Library library() const { return library_; }
    void setLibrary(const Library& library) { library_ = library; }

    Manufacturer manufacturer() const { return manufacturer_; }
    void setManufacturer(const Manufacturer& manufacturer) { manufacturer_ = manufacturer; }

    MountType mount() const { return mount_; }
    void setMount(const MountType& mount) { mount_ = mount; }

    Package package() const { return package_; }
    void setPackage(const Package& package) { package_ = package; }

    mvector<Pin> pins() const { return pins_; }
    mvector<Pin>& pins() { return pins_; }
    void addPin(Pin&& pins) { pins_.emplace_back(pins); }

    mvector<Supplier> suppliers() const { return suppliers_; }
    void setSuppliers(const mvector<Supplier>& suppliers) { suppliers_ = suppliers; }

    QPointF referencePoint() const { return referencePoint_; }
    void setReferencePoint(const QPointF& referencePoint) { referencePoint_ = referencePoint; }

    mvector<Poly> footprint() const { return footprint_; }
    void addFootprint(const Poly& footprint) { footprint_.emplace_back(footprint); }

    QString footprintName() const { return footprintName_; }
    void setFootprintName(const QString& footprintName) { footprintName_ = footprintName; }

    const QString& refdes() const { return refdes_; }
    void setRefdes(const QString& refdes) { isNull_ = true, refdes_ = refdes; }

    QString value() const { return value_; }
    void setValue(const QString& value) { value_ = value; }

    double height() const { return height_; }
    void setHeight(double height) { height_ = height; }

    double rotation() const { return rotation_; }
    void setRotation(double rotation) { rotation_ = rotation; }

    bool isNull() const { return isNull_; }

private:
    double rotation_ = 0.0; /* <decimal> The rotation angle of the component.*/
    double height_ = 0.0;   /* <decimal> Height, in the unit of the file. */
    mutable Item* componentitem_ = nullptr;
    Library library_;
    Manufacturer manufacturer_;
    MountType mount_ = Other; /* (TH|SMD|BGA|Other) Mount type. */
    Package package_;
    mvector<Pin> pins_;
    mvector<Supplier> suppliers_;
    QPointF referencePoint_;
    mvector<Poly> footprint_;
    QString footprintName_; /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
    QString refdes_;
    QString value_; /* <field> E.g. 220nF. */
    bool isNull_ = true;
};

} // namespace Gerber::Comp
