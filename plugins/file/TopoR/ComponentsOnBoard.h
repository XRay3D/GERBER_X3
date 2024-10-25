#pragma once

#include "Commons.h"
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Компоненты на плате (обязательный раздел).
/// </summary>
class ComponentsOnBoard : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание компонента на плате.
    /// </summary>
    /// <remarks>! Если компонент расположен на нижней стороне платы, его посадочное место отображается зеркально относительно вертикальной оси посадочного места, описанного в библиотеке(т.е.без угла поворота). Стеки контактных площадок переворачиваются.</remarks>
public:
    class CompInstance {

        /// <summary>
        /// Описание контакта компонента на плате.
        /// </summary>
        /// <remarks>! Если PadstackRef не указан, то стек контактных площадок берётся из посадочного места.</remarks>
    public:
        class CompInstance_Pin {
            /// <summary>
            /// Номер контакта компонента.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("padNum", DataType = "int")] public int _padNum;
            int _padNum = 0;

            /// <summary>
            /// Ссылка на стек контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
            PadstackRef* _PadstackRef;
            virtual ~CompInstance_Pin() {
                delete _PadstackRef;
                delete _Org;
            }

            bool ShouldSerialize_PadstackRef();
            /// <summary>
            /// Точка привязки объекта.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
            Org* _Org;
        };

        /// <summary>
        /// Описание монтажного отверстия в компоненте на плате.
        /// </summary>
    public:
        class CompInstance_Mnthole {
            /// <summary>
            /// Ссылка на монтажное отверстие в посадочном месте.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("mntholeRef")] public string _mntholeRef;
            QString _mntholeRef;

            /// <summary>
            /// Задаёт угол в градусах c точностью до тысячных долей.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
            float _angle = 0.0F;

            /// <summary>
            /// Ссылка на стек контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
            PadstackRef* _PadstackRef;

            /// <summary>
            /// Cсылка на цепь.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("NetRef")] public NetRef _NetRef;
            NetRef* _NetRef;
            virtual ~CompInstance_Mnthole() {
                delete _PadstackRef;
                delete _NetRef;
            }
        };

        /// <summary>
        /// Описание атрибута компонента на плате.
        /// </summary>
    public:
        class CompInstance_Attribute {
            /// <summary>
            /// Описание ярлыка компонента на плате.
            /// </summary>
        public:
            class CompInstance_Attribute_Label {
                /// <summary>
                /// Задаёт угол в градусах c точностью до тысячных долей.
                /// </summary>
            public:
                // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
                float _angle = 0.0F;

                /// <summary>
                /// Параметр надписей и ярлыков: зеркальность отображения.
                /// </summary>

                // ORIGINAL LINE: [XmlAttribute("mirror")] public Bool _mirror;
                Bool _mirror = static_cast<Bool>(0);
                virtual ~CompInstance_Attribute_Label() {
                    delete _LayerRef;
                    delete _TextStyleRef;
                    delete _Org;
                }

                // ORIGINAL LINE: [XmlIgnore] public bool _mirrorSpecified
                bool getMirrorSpecified() const;

                /// <summary>
                /// Параметр надписей (ярлыков): способ выравнивания текста.
                /// </summary>

                // ORIGINAL LINE: [XmlAttribute("align")] public align _align;
                align _align = static_cast<align>(0);

                /// <summary>
                /// Флаг видимости.
                /// </summary>

                // ORIGINAL LINE: [XmlAttribute("visible")] public Bool _visible;
                Bool _visible = static_cast<Bool>(0);

                // ORIGINAL LINE: [XmlIgnore] public bool _visibleSpecified
                bool getVisibleSpecified() const;

                /// <summary>
                /// Ссылка на слой.
                /// </summary>

                // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
                LayerRef* _LayerRef;

                /// <summary>
                /// Ссылка на стиль надписей.
                /// </summary>

                // ORIGINAL LINE: [XmlElement("TextStyleRef")] public TextStyleRef _TextStyleRef;
                TextStyleRef* _TextStyleRef;

                /// <summary>
                /// Точка привязки объекта.
                /// </summary>

                // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
                Org* _Org;
            };

            /// <summary>
            /// Тип предопределённого атрибута компонента.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("type")] public type _type;
            type _type = static_cast<type>(0);

            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

            /// <summary>
            /// Значение атрибута.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("value")] public string _value;
            QString _value;

            /// <summary>
            /// Ярлыки.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("Label")] public List<CompInstance_Attribute_Label> _Labels;
            std::vector<CompInstance_Attribute_Label*> _Labels;
            bool ShouldSerialize_Labels();
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Уникальный идентификатор компонента. Используется при синхронизации. Необязательный атрибут.
        /// Если не задан, то будет создан при импорте файла.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("uniqueId")] public string _uniqueId;
        QString _uniqueId;

        /// <summary>
        /// Сторона объекта.
        /// </summary>
        /// <remarks>!Значение Both возможно только при описании запретов размещения.</remarks>

        // ORIGINAL LINE: [XmlAttribute("side")] public side _side;
        side _side = static_cast<side>(0);

        /// <summary>
        /// Задаёт угол в градусах c точностью до тысячных долей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
        float _angle = 0.0F;

        /// <summary>
        /// Признак фиксации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("fixed")] public Bool _fixed;
        Bool _fixed = static_cast<Bool>(0);
        virtual ~CompInstance() {
            delete _ComponentRef;
            delete _FootprintRef;
            delete _Org;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _fixedSpecified
        bool getFixedSpecified() const;

        /// <summary>
        /// Ссылка на схемный компонент.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ComponentRef")] public ComponentRef _ComponentRef;
        ComponentRef* _ComponentRef;

        /// <summary>
        /// Ссылка на посадочное место.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("FootprintRef")] public FootprintRef _FootprintRef;
        FootprintRef* _FootprintRef;

        /// <summary>
        /// Точка привязки объекта.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
        Org* _Org;

        /// <summary>
        /// Контакты компонента на плате.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Pins")][XmlArrayItem("Pin")] public List<CompInstance_Pin> _Pins;
        std::vector<CompInstance_Pin*> _Pins;
        bool ShouldSerialize_Pins();
        /// <summary>
        /// Монтажные отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Mntholes")][XmlArrayItem("Mnthole")] public List<CompInstance_Mnthole> _Mntholes;
        std::vector<CompInstance_Mnthole*> _Mntholes;
        bool ShouldSerialize_Mntholes();
        /// <summary>
        /// Атрибуты компонента.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Attributes")][XmlArrayItem("Attribute")] public List<CompInstance_Attribute> _Attributes;
        std::vector<CompInstance_Attribute*> _Attributes;
        bool ShouldSerialize_Attributes();

        /************************************************************************
         * Здесь находятся функции для работы с элементами класса CompInstance. *
         * Они не являются частью формата TopoR PCB.                            *
         * **********************************************************************/
        /// <summary>
        /// Для отображения имени компонента
        /// </summary>
        /// <returns></returns>
        QString ToString();
        /***********************************************************************/
    };

    /// <summary>
    /// Описание одиночного контакта..
    /// </summary>
public:
    class FreePad {
        /// <summary>
        /// Сторона объекта.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("side")] public side _side;
        side _side = static_cast<side>(0);

        /// <summary>
        /// Задаёт угол в градусах c точностью до тысячных долей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
        float _angle = 0.0F;

        /// <summary>
        /// Признак фиксации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("fixed")] public Bool _fixed;
        Bool _fixed = static_cast<Bool>(0);
        virtual ~FreePad() {
            delete _PadstackRef;
            delete _NetRef;
            delete _Org;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _fixedSpecified
        bool getFixedSpecified() const;

        /// <summary>
        /// Ссылка на стек контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
        PadstackRef* _PadstackRef;

        /// <summary>
        /// Cсылка на цепь.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("NetRef")] public NetRef _NetRef;
        NetRef* _NetRef;

        /// <summary>
        /// Точка привязки объекта.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
        Org* _Org;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Описание компонентов на плате (инстанции компонентов)
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Components")][XmlArrayItem("CompInstance")] public List<CompInstance> _Components;
    std::vector<CompInstance*> _Components;
    bool ShouldSerialize_Components();
    /// <summary>
    /// Описание одиночных контактов.(инстанции компонентов)
    /// </summary>

    // ORIGINAL LINE: [XmlArray("FreePads")][XmlArrayItem("FreePad")] public List<FreePad> _FreePads;
    std::vector<FreePad*> _FreePads;
    bool ShouldSerialize_FreePads();

    /*****************************************************************************
     * Здесь находятся функции для работы с элементами класса ComponentsOnBoard. *
     * Они не являются частью формата TopoR PCB.                                 *
     * ***************************************************************************/

    /// <summary>
    /// Добавление компонента
    /// </summary>
    /// <param name="name">Имя нового компонента. Если имя неуникально, будет добавлен префикс _</param>
    /// <param name="units">текущие единицы измерения</param>
    /// <param name="componentRef">ссылка на библиотеку компонентов</param>
    /// <param name="footprintRef">ссылка на библиотеку посадочных мест</param>
    /// <returns>Имя нового компонента</returns>
    QString AddComponent(const QString& name, units units, const QString& componentRef, const QString& footprintRef);
    /// <summary>
    /// Удаление компонента по имени
    /// </summary>
    /// <param name="name">уникальный имя компонента</param>
    /// <returns>true - если было произведено удаление, иначе (компонент не найден) - false</returns>
    bool RemoveComponent(const QString& name);
    /// <summary>
    /// Индекс компонента
    /// </summary>
    /// <param name="name">уникальное имя компонента</param>
    /// <returns>индекс компонента или -1, если компонент отсутствует</returns>
    int ComponentIndexOf(const QString& name);

    /// <summary>
    /// Переименование компонента
    /// </summary>
    /// <param name="oldname">старое имя компонента</param>
    /// <param name="newname">новое имя компонента</param>
    /// <returns>индекс компонента, если было произведено переименование, -1, если компонент не найден</returns>
    int RenameComponent(const QString& oldname, const QString& newname);

    /// <summary>
    /// Генерация уникального идентификатора
    /// </summary>
    /// <returns>string like "ABCDEFGH"</returns>
    QString UniqueId();

    /*************************************************************************************/
};
// } // namespace TopoR_PCB_Classes
