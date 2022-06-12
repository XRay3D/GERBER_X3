/********************************************************************************
 * Author : Damir Bakiev *
 * Version : na *
 * Date : 11 November 2021 *
 * Website : na *
 * Copyright : Damir Bakiev 2016-2020 *
 * License:  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt *
 *******************************************************************************/
#pragma once

#include "gbr_attributes.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMetaObject>
#include <QObject>
#include <QStringList>
#include <map>

namespace Gerber::Attr {
/////////////////////////////////////////////////////
/// \brief The AbsctractData struct
///
struct AbstrFileFunc {
    Q_GADGET

public:
    virtual ~AbstrFileFunc() = default;
    enum class Side {
        Null = -1,
        Top,
        Bot,
        Inr,
    };
    Q_ENUM(Side)
    static Side toSide(const QString str) { return Side(staticMetaObject.enumerator(0).keyToValue(str.toLocal8Bit().data())); }

    enum class Layer {
        Null,
        L1,
        L2,
        L3,
        L4,
        L5,
        L6,
        L7,
        L8,
        L9,
        L10,
        L11,
        L12,
        L13,
        L14,
        L15,
        L16,
        L17,
        L18,
        L19,
        L20,
        L21,
        L22,
        L23,
        L24,
        L25,
        L26,
        L27,
        L28,
        L29,
        L30,
        L31,
        L32,
        L33,
        L34,
        L35,
        L36,
        L37,
        L38,
        L39,
        L40,
        L41,
        L42,
        L43,
        L44,
        L45,
        L46,
        L47,
        L48,
        L49,
        L50,
        L51,
        L52,
        L53,
        L54,
        L55,
        L56,
        L57,
        L58,
        L59,
        L60,
        L61,
        L62,
        L63,
        L64
    };
    Q_ENUM(Layer)
    static Layer toLayer(const QString str) { return Layer(staticMetaObject.enumerator(1).keyToValue(str.toLocal8Bit().data())); }

    AbstrFileFunc(File::Function function);
    const File::Function function;
    virtual Side side_() const { return Side::Null; }
};
/////////////////////////////////////////////////////
/// \brief The Copper struc
///
struct Copper : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /* L<p>,(Top|Inr|Bot)[,<type>] */
    // Проводник или слой меди.
    // L<p> (p - целое число> 0) указывает номер физического медного сллоя.
    // Номера идут подряд. Верхний слой всегда L1 (L0 не существует).
    // Обязательное поле (Top|Inr|Bot) определяет его как верхний, внутренний или нижний уровень,
    // эта избыточная информация помогает обрабатывать частичные данные.
    // Спецификация верхнего уровня - "Copper, L1,Top[,type]"»", для нижнего слоя 8-слойного задания - "Copper,L8,Bot[,type]"
    // На верхней стороне находятся компоненты со сквозным отверстием, если таковые имеются.
    // Необязательное поле <type> указывает тип слоя.
    // Если он присутствует, он должен принимать одно из следующих значений: Plane, Signal, Mixed или Hatched.

public:
    enum class Type {
        Null = -1,
        Plane,
        Signal,
        Mixed,
        Hatched
    };
    Q_ENUM(Type)
    static Type toType(const QString str) { return Type(staticMetaObject.enumerator(0).keyToValue(str.toLocal8Bit().data())); }

    Copper(File::Function function, const QStringList& list);
    const Layer layer;
    const Side side;
    const Type type;
};
/////////////////////////////////////////////////////
/// \brief The ArrayDrawing struct
///
struct ArrayDrawing : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*
     * Чертеж массива (бисквит, монтажный щит, отгрузочный щит, заказчик).
     * A drawing of the array (biscuit, assembly panel, shipment panel, customer panel).
     */

public:
    ArrayDrawing(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function)
    //, side(toSide(list.value(0)))
    {
    }
    // const Side side;
};
/////////////////////////////////////////////////////
/// \brief The AssemblyDrawing struct
///
struct AssemblyDrawing : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)
               Вспомогательный чертеж с расположением и позиционными обозначениями компонентов.
               Он в основном используется при сборке печатных плат. */

public:
    AssemblyDrawing(File::Function function, const QStringList& list);
    const Side side;
    virtual Side side_() const override { return side; }
};
/////////////////////////////////////////////////////
/// \brief The Component struct
///
struct Component : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*L<p>,(Top|Bot) A component layer.
                Слой с информацией о компонентах.
                L<p> Целое число p - это номер слоя меди, к которому прикреплены компоненты, описанные в этом файле.
                (Top|Bot) указывает, находятся ли компоненты наверху, вверх или внизу, внизу слоя, к которому они прикреплены.
                Этот синтаксис предназначен для встроенных компонентов.
                Для заданий без встроенных компонентов есть преднамеренное резервирование.
                Example:
                %TF.File,Component,L1,Top*%
                %TF.File,Component,L4,Bot*%*/
public:
    Component(File::Function function, const QStringList& list);
    const Layer layer;
    const Side side;
    virtual Side side_() const override { return side; }
};
/////////////////////////////////////////////////////
/// \brief The Depthrout struct
///
struct Depthrout : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)
                Area that must be routed to a given depth rather than going through the whole board. */

public:
    Depthrout(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Drillmap struct
///
struct Drillmap : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*
               A drawing with the locations of the drilled holes. It often also contains the hole sizes, tolerances and plated/non-plated info. */

public:
    Drillmap(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The FabricationDrawing struct
///
struct FabricationDrawing : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*
               A drawing with additional information for the fabrication of the bare PCB: the location of holes and slots, the board outline, sizes and tolerances, layer stack, material, finish choice, etc. */

public:
    FabricationDrawing(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Glue struct
///
struct Glue : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)
               Glue spots used to fix components to the board prior to soldering. */

public:
    Glue(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Legend struct
///
struct Legend : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)[,<index>]*/ /*
            Сверху паяльной маски напечатана легенда, показывающая, какой компонент и где находится.
            A.k.a. «Шелк» или «шелкография».
            См. Запись Soldermask для объяснения индекса. */

public:
    Legend(File::Function function, const QStringList& list);
    const Side side;
    const int index;
    virtual Side side_() const override { return side; }
};
/////////////////////////////////////////////////////
/// \brief The NonPlated struct
///
struct NonPlated : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*i,j,(NPTH|Blind|Buried) [,<label>]*/ /*
     Non-plated drill/rout data, span from copper layer i to layer j. The from/to order is not significant. The (NPTH|Blind|Buried) field is mandatory.
    The label is optional. If present it must take one of the following values: Drill, Rout or Mixed. */

public:
    enum class Type {
        Null = -1,
        NPTH,
        Blind,
        Buried
    };
    Q_ENUM(Type)
    static Type toType(const QString str) { return Type(staticMetaObject.enumerator(0).keyToValue(str.toLocal8Bit().data())); }

    enum class Label {
        Null = -1,
        Drill,
        Rout,
        Mixed
    };
    Q_ENUM(Label)
    static Label toLabel(const QString str) { return Label(staticMetaObject.enumerator(1).keyToValue(str.toLocal8Bit().data())); }

    NonPlated(File::Function function, const QStringList& list);
    const int layerFrom;
    const int layerTo;
    const Type type;
    const Label label;
};
/////////////////////////////////////////////////////
/// \brief The Other struct
///
struct Other : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*<mandatory field>
                The value ‘Other’ is to be used if none of the values above fits. By putting ‘Other’ rather than simply omitting the file function attribute it is clear the file has none of the standard functions, already useful information. Do not abuse standard values for a file with a vaguely similar function – use ‘Other’ to keep the function value clean and reliable.
                The mandatory field informally describes the file function.*/

public:
    Other(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The OtherDrawing struct
///
struct OtherDrawing : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*<mandatory field>
                Any other drawing than the 4 ones above. The mandatory field informally describes its topic. */
    // Other files
public:
    OtherDrawing(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Pads struct
///
struct Pads : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)
                A file containing only the pads (SMD, BGA, component, …). Not needed in a fabrication data set. */

public:
    Pads(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Paste struct
///
struct Paste : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)*/ /*  Места, где необходимо нанести паяльную пасту. */

public:
    Paste(File::Function function, const QStringList& list);
    const Side side;
    virtual Side side_() const override { return side; }
};
/////////////////////////////////////////////////////
/// \brief The Plated struct
///
struct Plated : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*i,j,(PTH|Blind|Buried)[,<label>]*/
    //    Данные сверления / фрезерования с покрытием от слоя меди i до слоя j.
    //    От / до заказа не имеет значения. Поле (PTH | Blind | Buried) является обязательным.
    //    Этикетка не обязательна.
    //    Если присутствует, он должен принимать одно из следующих значений: Drill, Rout или Mixed.

public:
    enum class Type {
        Null = -1,
        PTH,
        Blind,
        Buried
    };
    Q_ENUM(Type)
    static Type toType(const QString str) { return Type(staticMetaObject.enumerator(0).keyToValue(str.toLocal8Bit().data())); }

    enum class Label {
        Null = -1,
        Drill,
        Rout,
        Mixed
    };
    Q_ENUM(Label)
    static Label toLabel(const QString str) { return Label(staticMetaObject.enumerator(1).keyToValue(str.toLocal8Bit().data())); }

    Plated(File::Function function, const QStringList& list);
    const int layerFrom;
    const int layerTo;
    const Type type;
    const Label label;
};
/////////////////////////////////////////////////////
/// \brief The Profile struct
///
struct Profile : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(P|NP)*/
    /*
    Файл, содержащий профиль платы (или схему) и только профиль платы.
            Такой файл является обязательным в наборе данных для изготовления печатной платы. См. 6.1.5.
            Обязательная этикетка (P | NP) указывает, покрыта ли плата кромкой или нет. */

public:
    enum class EdgePlated {
        Null = -1,
        P,
        NP
    };
    Q_ENUM(EdgePlated)
    static EdgePlated toEdgePlated(const QString str) { return EdgePlated(staticMetaObject.enumerator(0).keyToValue(str.toLocal8Bit().data())); }

    Profile(File::Function function, const QStringList& list);
    const EdgePlated plated;
};
/////////////////////////////////////////////////////
/// \brief The Soldermask struct
///
struct Mask : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*(Top|Bot)[,<index>]*/
    /*
            Паяльная маска или припой резист.
            Обычно изображение представляет собой отверстия паяльной маски;
                тогда он имеет отрицательную полярность, см. 5.6.4.
            Необязательное поле необходимо только в том случае, если на одной стороне - верхней или нижней - более одной паяльной маски.
            Затем целое число <index> нумерует паяльные маски со стороны печатной платы наружу,
            начиная с 1 для маски непосредственно на меди.
            Обычно на одной стороне имеется только одна паяльная маска, и тогда <индекс> опускается.
            Пример с двумя верхними паяльными масками:
            Soldermask,Top,1 <- Маска на меди
            Soldermask,Top,2 <- Маска на первой маске */

public:
    enum class Type {
        Null = -1,
        Carbon,
        Gold,
        Heatsink,
        Peelable,
        Silver,
        Solder,
        Tin,
    };
    Q_ENUM(Type)
    static Type toType(File::Function function) { return Type(function - File::Carbonmask); }

    Mask(File::Function function, const QStringList& list);
    const Side side;
    const int index;
    const Type type;
    virtual Side side_() const override { return side; }
};
/////////////////////////////////////////////////////
/// \brief The Vcut struct
///
struct Vcut : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*[,(Top|Bot)]
               Contains the lines that must be v-cut. (V-cutting is also called scoring.)
               If the optional attachment (Top|Bot) is not present the scoring lines are identical on top and bottom – this is the normal case. In the exceptional case scoring is different on top and bottom two files must be supplied, one with Top and the other with Bot. */

public:
    Vcut(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Vcutmap struct
///
struct Vcutmap : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*
               A drawing with v-cut or scoring information. */

public:
    Vcutmap(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};
/////////////////////////////////////////////////////
/// \brief The Viafill struct
///
struct Viafill : AbstrFileFunc {
    Q_GADGET
    static int value(const QString str, int eNum) { return staticMetaObject.enumerator(eNum).keyToValue(str.toLocal8Bit().data()); }
    /*
               Contains the via’s that must be filled. It is however recommended to specify the filled via’s with the optional field in the .AperFunction ViaDrill. */

public:
    Viafill(File::Function function, const QStringList& /*list*/)
        : AbstrFileFunc(function) {
    }
};

} // namespace Gerber::Attr
