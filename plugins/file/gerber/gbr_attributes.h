/********************************************************************************
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
#include <QDebug>
#include <QMetaEnum>
#include <QMetaObject>
#include <QObject>
#include <QStringList>
#include <map>
#include <memory>

/*

Каждый атрибут состоит из имени атрибута и необязательного значения атрибута:
<Attribute> = <AttributeName>[,<AttributeValue>]*

Имена атрибутов соответствуют синтаксису имен в разделе 3.6.5.
Значение атрибута состоит из одного или нескольких полей, разделенных запятыми, см. Раздел 3.6.6.
<AttributeValue> = <Field>{,<Field>}

По элементам, к которым они прикреплены, есть три типа атрибутов:
Тип вложения        Элемент, к которому они прикрепляют метаинформацию
Атрибуты файла      Прикрепите метаинформацию к файлу целиком.
Атрибуты диафрагмы  Прикрепите метаинформацию к апертуре или региону. Объекты, созданные апертурой, наследуют метаинформацию апертуры.
Атрибуты объекта    Прикрепить метаинформацию к объекту напрямую


*/

namespace Gerber {

namespace Attr { // Attributes

    struct AbstrFileFunc;
    struct AbstrAperFunc;

    struct Command {
        static int value(const QString& key) {
            return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data());
        }
        enum e {
            TF, // Attribute file. Set a file attribute.    5.2
            TA, // Attribute aperture. Add an aperture attribute to the dictionary or modify it.    5.3
            TO, // Attribute object. Add an object attribute to the dictionary or modify it.    5.4
            TD, // Attribute delete. Delete one or all attributes in the dictionary.    5.5
        };
        Q_ENUM(e)
        Q_GADGET
    };

    struct File {
        Q_GADGET

    public:
        /* Атрибуты файла устанавливаются с помощью команды TF в верхнем регистре с использованием следующего синтаксиса
         * <TF command> = %TF<AttributeName>[,<AttributeValue>]*%
         * <AttributeValue> = <Field>{,<Field>}
         */
        enum class StdAttr {
            Part,               //Определяет часть, которую представляет файл, например одна печатная плата 5.6.2
            FileFunction,       //Определяет функцию файла на плате, например верхний медный слой 5.6.3
            FilePolarity,       //Положительное или отрицательное. Это определяет, представляет ли изображение присутствие или отсутствие материала. 5.6.4
            SameCoordinates,    //Все файлы в наборе производственных данных с этим атрибутом используют одни и те же координаты. Другими словами, они совпадают. 5.6.5
            CreationDate,       //Определяет дату и время создания файла. 5.6.6
            GenerationSoftware, //Определяет программное обеспечение, создающее файл. 5.6.7
            ProjectId,          //Определяет проект и редакции. 5.6.8
            MD5,                //Устанавливает подпись или контрольную сумму файла MD5. 5.6.9
        };
        Q_ENUM(StdAttr) // 0
        static StdAttr toStdAttr(const QString& key);

        enum class ePart {
            /* Значение атрибута файла .Part определяет, какая часть описывается.
             * Атрибут - если он присутствует - должен быть определен в заголовке.
             */
            Single,           // Одиночная печатная плата
            Array,            // A.k.a. панель заказчика, панель монтажная, панель транспортировочная, бисквит
            FabricationPanel, // A.k.a. рабочая панель, производственная панель
            Coupon,           // Тестовый купон
            Other             // Other,<mandatory field>         Ни один из вышеперечисленных. Обязательное поле неформально указывает на деталь
        };
        Q_ENUM(ePart) // 1
        static ePart toPart(const QString& key);

        enum eFilePolarity {
            Positive, // Изображение представляет наличие материала (рекомендуется)
            Negative, // Изображение представляет собой отсутствие материала
        };
        Q_ENUM(eFilePolarity) // 2
        static eFilePolarity toFilePolarityValue(const QString& key);

        enum Function {
            // Drawing
            ArrayDrawing,
            AssemblyDrawing,
            FabricationDrawing,
            OtherDrawing,
            // Mask
            Carbonmask,
            Goldmask,
            Heatsinkmask,
            Peelablemask,
            Silvermask,
            Soldermask,
            Tinmask,

            Component,
            Copper,
            Depthrout,
            Drillmap,
            Glue,
            Legend,
            // Plated
            NonPlated,
            Plated,
            Other,
            Pads,
            Paste,
            Profile,
            Vcut,
            Vcutmap,
            Viafill,
        };
        Q_ENUM(Function)
        static Function toFunction(const QString& key);

        void parse(const QStringList& list);

        QString creationDate;
        std::shared_ptr<AbstrFileFunc> function_;
        eFilePolarity filePolarity;
        QStringList generationSoftware;
        QString md5;
        QStringList part;
        QStringList projectId;
        QStringList sameCoordinates;
        std::map<QString, QStringList> custom;
    };

    struct Aperture {
        Q_GADGET
    public:
        static int value(const QString& key);
        enum StdAttr {
            // AperFunction objects created with the apertures, e.g. SMD pad 5.6.10
            AperFunction,
            // Tolerance of drill holes 5.6.11
            DrillTolerance,
            // If a flash represents text allows to define string, font, … 5.6.12
            FlashText,
        };
        Q_ENUM(StdAttr)
        StdAttr toStdAttr(const QString& key);

        enum Function {
            ViaDrill,
            BackDrill,
            ComponentDrill,
            MechanicalDrill,
            CastellatedDrill,
            OtherDrill,
            ComponentPad,
            SMDPad,
            BGAPad,
            ConnectorPad,
            HeatsinkPad,
            ViaPad,
            TestPad,
            CastellatedPad,
            FiducialPad,
            ThermalReliefPad,
            WasherPad,
            AntiPad,
            OtherPad,
            Conductor,
            EtchedComponent,
            NonConductor,
            CopperBalancing,
            Border,
            OtherCopper,
            // Component
            ComponentMain,
            ComponentOutline,
            ComponentPin,
            //
            Profile,
            NonMaterial,
            Material,
            Other,
        };
        Q_ENUM(Function)
        Function toFunction(const QString& key);

        void parse(const QStringList& list);
        std::shared_ptr<AbstrAperFunc> function_;
        QStringList drillTolerance_;
        QStringList flashText_;
    };

    //    struct AperFunction {
    //        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
    //        enum e {
    //            ComponentMain, /* This aperture is flashed at the centroid of a component.
    //        The flash carries the object attributes with the main
    //        characteristics of the component.
    //        The following aperture must be used:
    //        %ADD10C,0.300*% (mm)
    //        %ADD10C,0.012*% (in)*/
    //            ComponentOutline, /*(Body|Lead2Lead|Footprint|Courtyard)
    //        This attribute is used to draw the outline of the
    //        component. An outline is a sequence of connected
    //        draws and arcs. They are said to connect only if they are
    //        defined consecutively, with the second starting where
    //        the first one ends. Thus, the order in which they are
    //        defined is significant. A contour is closed: the end point
    //        of the last draw/arc must coincide with the start point of
    //        the first. Outlines cannot instance_-intersect.
    //        Four different types of outlines are defined. See drawing,
    //        courtesy Thiadmer Riemersma:
    //        Outlines of different types on the same component are
    //        allowed.
    //        The following aperture must be used:
    //        %ADD11C,0.100*% (mm)
    //        %ADD11C,0.004*% (in)*/
    //            ComponentPin,
    //            /*An aperture whose flash point indicates the location of
    //        the component pins (leads). The .P object attribute must
    //        be attached to each flash to identify the reference
    //        descriptor and pin.
    //        For the key pin, typically pin "1" or "A1", the following
    //        diamond shape aperture must be used:
    //        %ADD12P,0.360X4X0.0*% (mm)
    //        %ADD12P,0.017X4X0.0*% (in)
    //        The key pin is then visible in the image.
    //        For all other pins the following zero size aperture must
    //        be used:
    //        %ADD13C,0*%...(both mm and in)
    //        These pins are not visible which avoids cluttering the
    //        image.*/
    //        };
    //        Q_ENUM(e)
    //        Q_GADGET
    //    };
    //    struct ComponentOutline {
    //        static int value(const QString& key) { return staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().data()); }
    //        enum e {
    //            Body,
    //            Lead2Lead,
    //            Footprint,
    //            Courtyard,
    //        };
    //        Q_ENUM(e)
    //        Q_GADGET
    //    };

} // namespace Attr

} // namespace Gerber
