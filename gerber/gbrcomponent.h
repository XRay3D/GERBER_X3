#ifndef GBRCOMPONENT_H
#define GBRCOMPONENT_H

#include <QMap>
#include <QStack>
#include <QString>
#include <QVariant>
#include <QVector>

class Component {
public:
    Component();
    enum MountType {
        TH,
        SMD,
        Fiducial,
        Other
    };
    enum {
        C, // <refdes>
        CFtp, // <field> Footprint name
        CHgt, // <decimal> Height, in the unit of the file.
        CLbD, // <field> Library description
        CLbN, // <field> Library name
        CMPN, // <field> Manufacturer part number
        CMfr, // <field> Manufacturer
        CMnt, // (TH|SMD|Fiducial|Other) Mount type
        CPgD, // <field> Package description
        CPgN, // <field> Package name
        CRot, // <decimal> The rotation angle of the component.
        CSup, // <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field
        CVal, // <field> Value, e.g. 220nF
        P, // Reference descriptor and pin number - R301,1
    };
    template <class T>
    void setData(int key, const T& value)
    {
        switch (key) {
        case C:
            m_C = value; // <refdes>
            break;
        case CRot:
            m_CRot = value.toDouble(); // <decimal> The rotation angle of the component.
            break;
        case CMfr:
            m_CMfr = value; // <field> Manufacturer
            break;
        case CMPN:
            m_CMPN = value; // <field> Manufacturer part number
            break;
        case CVal:
            m_CVal = value; // <field> Value, e.g. 220nF
            break;
        case CMnt:
            //// m_CMnt = value; // (TH|SMD|Fiducial|Other) Mount type
            break;
        case CFtp:
            m_CFtp = value; // <field> Footprint name
            break;
        case CPgN:
            m_CPgN = value; // <field> Package name
            break;
        case CPgD:
            m_CPgD = value; // <field> Package description
            break;
        case CHgt:
            m_CHgt = value.toDouble(); // <decimal> Height, in the unit of the file.
            break;
        case CLbN:
            m_CLbN = value; // <field> Library name
            break;
        case CLbD:
            m_CLbD = value; // <field> Library description
            break;
        case CSup:
            m_CSup = value; // <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field
            break;
        case P:
            ///    m_P = value; // Reference descriptor and pin number - R301,1
            break;
        }
    }
    QString m_C; // <refdes>
    QString m_CFtp; // <field> Footprint name
    double m_CHgt; // <decimal> Height, in the unit of the file.
    QString m_CLbD; // <field> Library description
    QString m_CLbN; // <field> Library name
    QString m_CMPN; // <field> Manufacturer part number
    QString m_CMfr; // <field> Manufacturer
    MountType m_CMnt = Other; // (TH|SMD|Fiducial|Other) Mount type
    QString m_CPgD; // <field> Package description
    QString m_CPgN; // <field> Package name
    double m_CRot; // <decimal> The rotation angle of the component.
    QString m_CSup; // <SN>,<SPN>,{<SN>,<SPN>} <SN> is a field with the supplier name. <SPN> is a field
    QString m_CVal; // <field> Value, e.g. 220nF
    QVector<QPair<int, QString>> m_P; // Reference descriptor and pin number - R301,1

    static const QVector<QString> keys;

    QMap<int, QVariant> m_obj;
    QStack<int> m_objStack;
};

#endif // GBRCOMPONENT_H
