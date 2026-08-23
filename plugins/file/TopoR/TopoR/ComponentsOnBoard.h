#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Компоненты на плате (обязательный раздел).
struct ComponentsOnBoard {
    // Описание компонента на плате.
    // ! Если компонент расположен на нижней стороне платы, его посадочное место отображается зеркально относительно вертикальной оси посадочного места, описанного в библиотеке(т.е.без угла поворота). Стеки контактных площадок переворачиваются.
    struct CompInstance {
        // Описание контакта компонента на плате.
        // ! Если PadstackRef не указан, то стек контактных площадок берётся из посадочного места.
        struct Pin {
            // Номер контакта компонента.
            [[= XML::Attr]] int padNum{};
            // Ссылка на стек контактных площадок.
            // public PadstackRef PadstackRef;
            /*[[= XML::Elem]]*/ PadstackRef PadstackRef; /*("PadstackRef")*/
            bool ShouldSerializePadstackRef();
            // Точка привязки объекта.
            // public Org Org;
            /*[[= XML::Elem]]*/ Org Org; /*("Org")*/
        };
        // Описание монтажного отверстия в компоненте на плате.
        struct Mnthole {
            // Ссылка на монтажное отверстие в посадочном месте.
            // public string mntholeRef;
            [[= XML::Attr]] std::string mntholeRef;
            // Задаёт угол в градусах c точностью до тысячных долей.
            [[= XML::Attr]] double angle{};
            // Ссылка на стек контактных площадок.
            // public PadstackRef PadstackRef;
            /*[[= XML::Elem]]*/ PadstackRef PadstackRef; /*("PadstackRef")*/
                                                         // Cсылка на цепь.
                                                         // public NetRef NetRef;
            /*[[= XML::Elem]]*/ NetRef NetRef;           /*("NetRef")*/
        };
        // Описание атрибута компонента на плате.
        struct Attribute {
            // Описание ярлыка компонента на плате.
            struct Label {
                // Задаёт угол в градусах c точностью до тысячных долей.
                [[= XML::Attr]] double angle{};
                // Параметр надписей и ярлыков: зеркальность отображения.
                // public Bool mirror;
                [[= XML::Attr]] Bool mirror{};
                // public bool mirrorSpecified
                bool getMirrorSpecified() const;
                // Параметр надписей (ярлыков): способ выравнивания текста.
                // public align align;
                [[= XML::Attr]] align align{}; /*("align")*/
                // Флаг видимости.
                // public Bool visible;
                [[= XML::Attr]] Bool visible{};
                // public bool visibleSpecified
                bool getVisibleSpecified() const;
                // Ссылка на слой.
                // public LayerRef LayerRef;
                /*[[= XML::Elem]]*/ LayerRef LayerRef;         /*("LayerRef")*/
                                                               // Ссылка на стиль надписей.
                                                               // public TextStyleRef TextStyleRef;
                /*[[= XML::Elem]]*/ TextStyleRef TextStyleRef; /*("TextStyleRef")*/
                                                               // Точка привязки объекта.
                                                               // public Org Org;
                /*[[= XML::Elem]]*/ Org Org;                   /*("Org")*/
            };
            // Тип предопределённого атрибута компонента.
            // public type type;
            [[= XML::Attr]] std::optional<type> type{}; // FIXME maybe optional
            // Имя объекта или ссылка на именованный объект.
            // public string name;
            [[= XML::Attr]] std::string name;
            // Значение атрибута.
            // public string value;
            [[= XML::Attr]] std::string value;
            // Ярлыки.
            // public List<Label> Labels;
            [[= XML::Elem]] std::vector<Label> Labels; /*("Label")*/
            bool ShouldSerialize_Labels();
        };
        // Имя объекта или ссылка на именованный объект.
        // public string name;
        [[= XML::Attr]] std::string name;
        // Уникальный идентификатор компонента. Используется при синхронизации. Необязательный атрибут.
        // Если не задан, то будет создан при импорте файла.
        // public string uniqueId;
        [[= XML::Attr]] std::string uniqueId;
        // Сторона объекта.
        // !Значение Both возможно только при описании запретов размещения.
        // public side side;
        [[= XML::Attr]] side side{}; /*("side")*/
        // Задаёт угол в градусах c точностью до тысячных долей.
        // angle
        [[= XML::Attr]] double angle{};
        // Признак фиксации.
        // public Bool fixed;
        [[= XML::Attr]] Bool fixed{};
        // public bool fixedSpecified
        bool getFixedSpecified() const;
        // Ссылка на схемный компонент.
        // public ComponentRef ComponentRef;
        /*[[= XML::Elem]]*/ ComponentRef ComponentRef; /*("ComponentRef")*/
                                                       // Ссылка на посадочное место.
                                                       // public FootprintRef FootprintRef;
        /*[[= XML::Elem]]*/ FootprintRef FootprintRef; /*("FootprintRef")*/
                                                       // Точка привязки объекта.
                                                       // public Org Org;
        /*[[= XML::Elem]]*/ Org Org;                   /*("Org")*/
        // Контакты компонента на плате.
        //[XmlArrayItem/*("Pin")*/] public List<Pin> Pins;
        [[= XML::Array]] std::vector<Pin> Pins;
        bool ShouldSerialize_Pins();
        // Монтажные отверстия.
        //[XmlArrayItem/*("Mnthole")*/] public List<Mnthole> Mntholes;
        [[= XML::Array]] std::vector<Mnthole> Mntholes;
        bool ShouldSerialize_Mntholes();
        // Атрибуты компонента.
        //[XmlArrayItem/*("Attribute")*/] public List<Attribute> Attributes;
        [[= XML::Array]] std::vector<Attribute> Attributes;
        bool ShouldSerialize_Attributes();
        /************************************************************************
         * Здесь находятся функции для работы с элементами класса CompInstance. *
         * Они не являются частью формата TopoR PCB.                            *
         * **********************************************************************/
        // Для отображения имени компонента
        //
        std::string ToString();
        QTransform transform() const {
            QTransform transform;
            if(Org) transform.translate(Org.x, Org.y);
            if(angle) transform.rotate(angle);
            return transform;
        }
        /***********************************************************************/
    };
    // Описание одиночного контакта..
    struct FreePad {
        // Сторона объекта.
        // public side side;
        [[= XML::Attr]] std::string name{}; /*("side")*/
        [[= XML::Attr]] side side{};        /*("side")*/
        // Задаёт угол в градусах c точностью до тысячных долей.
        // angle
        [[= XML::Attr]] double angle{};
        // Признак фиксации.
        // public Bool fixed;
        [[= XML::Attr]] Bool fixed{};
        bool getFixedSpecified() const;
        // Ссылка на стек контактных площадок.
        // public PadstackRef PadstackRef;
        /*[[= XML::Elem]]*/ PadstackRef PadstackRef;      /*("PadstackRef")*/
                                                          // Cсылка на цепь.
                                                          // public NetRef NetRef;
        /*[[= XML::Elem]]*/ std::optional<NetRef> NetRef; /*("NetRef")*/
        // Точка привязки объекта.
        // public Org Org;
        [[= XML::ElemF]] Org Org; /*("Org")*/
        QTransform transform() const {
            QTransform transform;
            if(Org) transform.translate(Org.x, Org.y);
            if(angle) transform.rotate(angle);
            return transform;
        }
    };
    // Версия раздела.
    // public string version;
    [[= XML::Attr]] std::string version;
    // Описание компонентов на плате (инстанции компонентов)
    //[XmlArrayItem/*("CompInstance")*/] public List<CompInstance> Components;
    [[= XML::Array]] std::vector<CompInstance> Components;
    bool ShouldSerialize_Components();
    // Описание одиночных контактов.(инстанции компонентов)
    //[XmlArrayItem/*("FreePad")*/] public List<FreePad> FreePads;
    [[= XML::Array]] std::vector<FreePad> FreePads;
    bool ShouldSerialize_FreePads();
    /*****************************************************************************
     * Здесь находятся функции для работы с элементами класса ComponentsOnBoard. *
     * Они не являются частью формата TopoR PCB.                                 *
     * ***************************************************************************/
    // Добавление компонента
    // <param name="name">Имя нового компонента. Если имя неуникально, будет добавлен префикс </param>   // <param name="units">текущие единицы измерения</param>   // <param name="componentRef">ссылка на библиотеку компонентов</param>   // <param name="footprintRef">ссылка на библиотеку посадочных мест</param>   // Имя нового компонента
    std::string AddComponent(const std::string& name, units units, const std::string& componentRef, const std::string& footprintRef);
    // Удаление компонента по имени
    // <param name="name">уникальный имя компонента</param>   // true - если было произведено удаление, иначе (компонент не найден) - false
    bool RemoveComponent(const std::string& name);
    // Индекс компонента
    // <param name="name">уникальное имя компонента</param>   // индекс компонента или -1, если компонент отсутствует
    int ComponentIndexOf(const std::string& name);
    // Переименование компонента
    // <param name="oldname">старое имя компонента</param>   // <param name="newname">новое имя компонента</param>   // индекс компонента, если было произведено переименование, -1, если компонент не найден
    int RenameComponent(const std::string& oldname, const std::string& newname);
    // Генерация уникального идентификатора
    // string like "ABCDEFGH"
    std::string UniqueId();
    /*************************************************************************************/
};
} // namespace TopoR
