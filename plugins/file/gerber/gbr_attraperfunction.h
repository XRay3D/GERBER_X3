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

struct AbstrAperFunc {
    Q_GADGET
public:
    virtual ~AbstrAperFunc() = default;
    AbstrAperFunc(Aperture::Function function)
        : function(function) {
    }
    const Aperture::Function function;
};

struct ViaDrill : AbstrAperFunc {
    Q_GADGET
    /*[,<IPC-4761>]<IPC-4761>=(Ia|Ib|IIa|IIb| IIIa|IIIb|IVa|IVb|V|VI| VII|None)*/
    /*[,<IPC-4761>]
    <IPC-4761>=(Ia|Ib|IIa|IIb| IIIa|IIIb|IVa|IVb|V|VI| VII|None)
    Сквозное отверстие. Это зарезервировано для отверстий,
    единственная функция которых - соединять разные слои.
    Его нельзя использовать для отверстий для выводов компонентов.
    Никакие штифты не вставляются через отверстия.
    В необязательном поле указывается защита переходного отверстия
    в соответствии с IPC-4761.*/
public:
    ViaDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct BackDrill : AbstrAperFunc {
    Q_GADGET
    /*[,PressFit]*/
    /*[,PressFit]
   Отверстие для удаления обшивки с подпяточного пролета
   путем просверливания этого подпролетного участка большего диаметра.*/
public:
    BackDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ComponentDrill : AbstrAperFunc {
    Q_GADGET
    /*[,PressFit]*/
    /*[,PressFit]
    Отверстие, которое используется для крепления и / или электрического соединения
     ыводов компонентов, включая контакты и провода, на печатную плату.
    (Согласно IPC-T-50).
    Дополнительная этикетка PressFit указывает отверстия для проводов с прессовой посадкой.
    Провода с прессовой посадкой вдавливаются в металлические сквозные отверстия
    подходящего размера для обеспечения электрического контакта.
    Этикетка может быть нанесена только на отверстия PTH.
    См. Также ComponentPad.*/
public:
    ComponentDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct MechanicalDrill : AbstrAperFunc {
    Q_GADGET
    /*[,(Tooling|Breakout|Other)]*/
    /*[,(Tooling|Breakout|Other)]
    Отверстие с механической функцией (регистрация, винт и т. Д.) Спецификатор не является обязательным. Если он присутствует, он может принимать одно из следующих значений:
    • Tooling: отверстия для инструментов для временного прикрепления
    платы или панели к тестовым приспособлениям во время сборки и тестирования.
    Также называются монтажными отверстиями.
    • BreakOut: отверстия без покрытия, образующие отрывной язычок,
    используемый при разводке с разрывом.*/
public:
    MechanicalDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct CastellatedDrill : AbstrAperFunc {
    Q_GADGET
    /**/
    /*
   Металлизированные отверстия, прорезанные краем платы;
   используется для соединения печатных плат.*/
public:
    CastellatedDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct OtherDrill : AbstrAperFunc {
    Q_GADGET
    /*,<mandatory field>*/
    /*,<mandatory field>
   Отверстие, но ничего из вышеперечисленного.
   Обязательное поле неформально описывает тип.*/
public:
    OtherDrill(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ComponentPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Контактная площадка, связанная с отверстием для компонента. Контактные площадки вокруг ComponentDrill на всех слоях принимают значение ComponentPad, хотя на самом деле на внутренних слоях они имеют только функцию перехода. Другими словами, значение атрибута площадки следует за значением атрибута инструмента сверла.
   Только для компонентов со сквозным отверстием; SMD и BGA имеют свой собственный выделенный тип.
   См. Также ComponentDrill.*/
public:
    ComponentPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct SMDPad : AbstrAperFunc {
    Q_GADGET
    /*,(CuDef|SMDef)*/
    /*,(CuDef|SMDef)
   Контактная площадка, принадлежащая посадочному месту SMD-компонента. Они приклеены или иным образом электрически подключены к печатной плате. Назначение этих контактных площадок обычно состоит в том, чтобы подключить компонентную схему к печатной плате, но для определенных компонентов некоторые контактные площадки могут не подключаться к компоненту внутри корпуса. За исключением контактных площадок BGA, имеющих собственный тип. Эта функция действует только для обычных электрических прокладок, термопрокладки имеют свою функцию; см. HeatsinkPad.
   Спецификатор (CuDef | SMDef) является обязательным. CuDef означает определенную медь; это, безусловно, самая распространенная площадка для поверхностного монтажа; медная площадка полностью свободна от паяльной маски; область, покрываемая паяльной пастой, определяется медной площадкой. SMDef - обозначение паяльной маски; паяльная маска перекрывает медную площадку; область, покрываемая паяльной пастой, определяется отверстием паяльной маски, а не медной площадкой. (CuDef иногда довольно неудобно называют без определения паяльной маски.)
   Применимо только для внешних слоев.
   Когда контактная площадка SMD содержит сквозное отверстие, контактная площадка, к которой припаяна SMD, на внешнем слое с SMD - это SMDPad, все остальные контактные площадки в стеке - ViaPad. Если контактная площадка SMD содержит встроенную контактную площадку, как и должно быть, то эта встроенная контактная площадка, конечно же, является ViaPad.*/

public:
    SMDPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct BGAPad : AbstrAperFunc {
    Q_GADGET
    /*,(CuDef|SMDef)*/
    /*,(CuDef|SMDef)
   Контактная площадка, принадлежащая посадочному месту компонента BGA. Они припаяны или иным образом электрически подключены к печатной плате. Назначение этих контактных площадок обычно состоит в том, чтобы подключить компонентную схему к печатной плате, но для определенных компонентов некоторые контактные площадки могут не подключаться к компоненту внутри корпуса.
   Спецификатор (CuDef | SMDef) является обязательным. CuDef означает определенную медь, SMDef - определенную паяльную маску; см. SMDPad.
   Применимо только для внешних слоев.
   Когда контактная площадка BGA имеет сквозное отверстие, контактная площадка, к которой припаяна BGA, является BGAPad, все остальные контактные площадки в стеке - ViaPad. Если контактная площадка BGA содержит встроенную контактную площадку, как и должно быть, то эта встроенная контактная площадка, конечно же, является ViaPad.*/

public:
    BGAPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ConnectorPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Крайняя контактная площадка.
   Применимо только для внешних слоев.*/
public:
    ConnectorPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct HeatsinkPad : AbstrAperFunc {
    Q_GADGET
    /*Радиатор или термопрокладка, обычно для SMD*/
    /*Радиатор или термопрокладка, обычно для SMD*/
public:
    HeatsinkPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ViaPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Переходная площадка.
    В нем предусмотрено кольцо для крепления обшивки в стволе.
    Это зарезервировано для контактных площадок, у которых нет другой функции,
    кроме установления соединения между слоями:
    компонентные контактные площадки часто также имеют функцию перехода;
    однако их основная функция - это компонентная площадка, и они должны иметь эту функцию;
    аналогично для тестовых площадок, переходное отверстие в BGA и т. д.*/
public:
    ViaPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct TestPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Тестовая площадка. Применимо только для внешних слоев.
    Иногда тестовая площадка просверливается и также имеет функцию перехода для экономии места. Такая площадка должна быть указана как тестовая. (Из конструкции ясно, что он имеет функцию перехода, но изготовитель должен знать, что это тестовая площадка, и это не очевидно. Точно так же контактная площадка компонента может также функционировать как переходное отверстие, но остается контактной площадкой компонента. )*/

public:
    TestPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct CastellatedPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Подушечки на металлизированных отверстиях, прорезанных краем доски; используется для соединения печатных плат.*/

public:
    CastellatedPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct FiducialPad : AbstrAperFunc {
    Q_GADGET
    /*,(Local|Global|Panel)*/
    /*,(Local|Global|Panel)
   Реперная площадка.
   Спецификатор (Local | GlobalPanel) является обязательным. Контактные площадки для локальных реперных знаков используются для определения положения отдельного компонента, глобальные для определения местоположения отдельной печатной платы и панели для определения местоположения панели.*/

public:
    FiducialPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ReliefPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Тепловая разгрузочная площадка, соединенная с окружающей медью, ограничивая тепловой поток.*/

public:
    ReliefPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct WasherPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Прокладка вокруг отверстия без покрытия без электрической функции. Несколько приложений, например прокладка, укрепляющая печатную плату в месте крепления болтом - отсюда и название шайба.*/

public:
    WasherPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct AntiPad : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Колодка с очищающей полярностью (LPC), создающая зазор в плоскости. Это освобождает место для прохода сверла без соединения с самолетом. Обратите внимание, что установка самого атрибута AntiPad не влияет на изображение и, следовательно, не превращает панель в LPC как побочный эффект - это должно быть сделано явно с помощью команды% LPC *%.*/

public:
    AntiPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct OtherPad : AbstrAperFunc {
    Q_GADGET
    /*,<mandatory field>*/
    /*,<mandatory field>
    Прокладка, не указанная выше. Обязательное поле неформально описывает тип.*/

public:
    OtherPad(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct Conductor : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Медь, функция которой заключается в соединении контактных площадок или обеспечении экранирования, как правило, дорожек и медных заливок, таких как плоскости питания и заземления. Обратите внимание, что токопроводящие медные заливки должны нести этот атрибут, независимо от того, правильно ли они сделаны указанием региона или покраской - см. Регионы.
   (Обратите внимание, что рисование - плохая практика, но если вам нужно рисовать, по крайней мере, добавьте атрибут, чтобы было ясно, что означает клубок рисунков.)*/

public:
    Conductor(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct EtchedComponent : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Протравленные компоненты - это встроенные катушки индуктивности, трансформаторы и конденсаторы, протравленные в медь печатной платы. На следующем рисунке показаны два протравленных индуктора:
   Для списка соединений САПР это такие же компоненты, как и другие: имена цепей с обеих сторон различаются. (Тем не менее, для электрического испытания голой платы они могут проводить медь и соединять сеть с обеих сторон.)*/

public:
    EtchedComponent(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct NonConductor : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Медь, не выполняющая роль проводника, не имеющая электрической функции; обычно текст на печатной плате, такой как номер детали и версия. Не то чтобы этот атрибут можно было применить только к меди, а не к элементам чертежа в медном слое; см. также Нематериал и Профиль.*/

public:
    NonConductor(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct CopperBalancing : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Медный узор добавлен для балансировки медного покрытия при нанесении покрытия.*/

public:
    CopperBalancing(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct Border : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Медный бордюр производственной панели.*/
public:
    Border(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct OtherCopper : AbstrAperFunc {
    Q_GADGET
    /*,<mandatory field>*/
    /*,<mandatory field>
   Указывает на другую функцию. Обязательное поле неформально описывает тип.*/

public:
    OtherCopper(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ComponentMain : AbstrAperFunc {
    Q_GADGET
    /**/
    /*

    Эта апертура высвечивается в центре тяжести компонента. Флэш-память содержит атрибуты объекта с основными характеристиками компонента.
    Необходимо использовать следующую апертуру:
    %ADDnnC,0.300*% (mm)
    %ADDnnC,0.012*% (in)
    nn - номер апертуры, целое число ≥ 10.
    */
public:
    ComponentMain(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ComponentOutline : AbstrAperFunc {
    Q_GADGET
    /*,<type><type>=(Body|Lead2Lead| Footprint|Courtyard)*/
    /*,<type>
               <type>=(Body|Lead2Lead| Footprint|Courtyard)
   Этот атрибут используется для рисования контура компонента. Контур - это последовательность соединенных линий и дуг. Говорят, что они подключаются только в том случае, если они определены последовательно, причем второй начинается там, где заканчивается первый. Таким образом, важен порядок, в котором они определены. Контур замкнут: конечная точка последнего рисунка / дуги должна совпадать с начальной точкой первого. Контуры не могут пересекаться друг с другом.
   Определены четыре различных типа контуров. См. Рисунок, любезно предоставленный Тиадмером Римерсмой:
   Допускаются контуры разных типов на одном и том же компоненте. Необходимо использовать следующую диафрагму:
   %ADDnnC,0.100*% (mm)
   %ADDnnC,0.004*% (in)*/
public:
    ComponentOutline(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ComponentPin : AbstrAperFunc {
    Q_GADGET
    /**/
    /*
   Координаты в команде flash (D03) указывают расположение выводов компонентов (выводов).
   Атрибут объекта .P должен быть прикреплен к каждой вспышке, чтобы идентифицировать ссылочный дескриптор и контакт.
   Для штифта ключа, обычно штифта «1» или «A1», необходимо использовать следующую диафрагму в форме ромба:
   %ADDnnP,0.360X4X0.0*% (mm)
   %ADDnnP,0.017X4X0.0*% (in)
   булавка будет видна на изображении.
   Для всех остальных выводов необходимо использовать следующую апертуру нулевого размера:
   %ADDnnC,0*%...(both mm and in)
   Эти контакты не видны, что позволяет избежать загромождения изображения.*/

public:
    ComponentPin(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct ProfileA : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Обозначает чертежи и дуги, которые точно определяют профиль или контур печатной платы. Это содержимое файла профиля, но оно также может присутствовать в других слоях. См. 6.1.5*/

public:
    ProfileA(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct NonMaterial : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Значение атрибута NonMaterial определяет объекты, которые представляют не физический материал, а элементы чертежа. NonMaterial имеет значение только для файлов, которые определяют структуру физических слоев печатной платы, таких как медные слои или паяльная маска. К сожалению, такие файлы иногда содержат не только данные, представляющие материал, но также элементы чертежа, такие как рамка и основная надпись. (Примечание: элементы чертежа не следует смешивать с данными шаблона, все, см. 6.1.6.2. Используйте файлы чертежей для чертежей.).*/

public:
    NonMaterial(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct Material : AbstrAperFunc {
    Q_GADGET
    /**/
    /*Определяет правильную часть файла данных.
    Паяльные маски традиционно отрицательны. Изображение представляет отверстия паяльной маски. Апертуры принимают значение «Материал» - они определяют материал паяльной маски, но в отрицательном смысле.
    Для слоев меди и сверла Материал разделен на более конкретные функции, такие как площадка SMD. Используйте специальные функции, когда они доступны, а не «Материал».*/

public:
    Material(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};
struct OtherA : AbstrAperFunc {
    Q_GADGET
    /*,<mandatory field>*/
    /*,<mandatory field>
   Значение «Другое» следует использовать, если ни одно из вышеперечисленных значений не подходит. Помещая значение «Другое», а не грубо опуская атрибут, становится явным, что значение не соответствует ни одному из вышеперечисленных - пропущенный атрибут может быть одним из вышеперечисленных. Конечно, не злоупотребляйте существующими значениями, превращая атрибут с неопределенно похожей функцией в это значение, которое не подходит идеально - сохраните идентификацию в чистоте, используя «Другое».
   Обязательное поле неформально описывает функцию диафрагмы.*/

public:
    OtherA(Aperture::Function function, const QStringList& /*list*/)
        : AbstrAperFunc(function) {
    }
};

} // namespace Gerber::Attr
