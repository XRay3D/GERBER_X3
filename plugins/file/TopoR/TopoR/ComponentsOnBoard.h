#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {
// Комоненты на плате (обязательный раздел).
struct ComponentsOnBoard {
    // Описание компонента на плате.
    struct CompInstance {
        /// \note !Если компонент расположен на нижней стороне платы, его посадочное место отображается зеркально относительно вертикальной оси посадочного места, описанного в библиотеке(т.е.без угла поворота). Стеки контактных площадок переворачиваются.

        // Описание контакта компонента на плате.
        struct Pin {
            /// \note !Если PadstackRef не указан, то стек контактных площадок берётся из посадочного места.
            // Номер контакта компонента.
            Xml::Attr<int> padNum;
            // Ссылка на стек контактных площадок.
            PadstackRef padstackRef;
            // Точка привязки объекта.
            Org org;
        };

        // Описание монтажного отверстия в компоненте на плате.
        struct Mnthole {
            // Ссылка на монтажное отверстие в посадочном месте.
            Xml::Attr<QString> mntholeRef;
            // Задаёт угол в градусах c точностью до тысячных долей.
            Xml::Attr<double> angle;
            // Ссылка на стек контактных площадок.
            PadstackRef padstackRef;
            // Cсылка на цепь.
            NetRef netRef;
        };

        // Описание атрибута компонента на плате.
        struct Attribute {
            // Описание ярлыка компонента на плате.
            struct Label {
                // Задаёт угол в градусах c точностью до тысячных долей.
                Xml::Attr<double> angle;
                // Параметр надписей и ярлыков: зеркальность отображения.
                Xml::Attr<Bool> mirror;
                // Параметр надписей (ярлыков): способ выравнивания текста.
                Xml::Attr<align> align_;
                // Флаг видимости.
                Xml::Attr<Bool> visible;
                // Ссылка на слой.
                LayerRef layerRef;
                // Ссылка на стиль надписей.
                TextStyleRef textStyleRef;
                // Точка привязки объекта.
                Org org;
                QTransform transform() const;
            };

            // Тип предопределённого атрибута компонента.
            Xml::Optional<Xml::Attr<type, NoOpt>> type_;
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
            // Значение атрибута.
            Xml::Optional<Xml::Attr<QString, NoOpt>> value;
            // Ярлыки.
            Xml::Array<Label> Labels;
        };

        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Уникальный идентификатор компонента. Используется при синхронизации. Необязательный атрибут.
        Xml::Attr<QString> uniqueId; /// \note Если не задан, то будет создан при импорте файла.
        // Сторона объекта.
        Xml::Attr<side, NoOpt> side_;
        /// \note !Значение Both возможно только при описании запретов размещения.
        // Задаёт угол в градусах c точностью до тысячных долей.
        Xml::Attr<double> angle;
        // Признак фиксации.
        Xml::Attr<Bool> fixed;
        // Ссылка на схемный компонент.
        ComponentRef componentRef;
        // Ссылка на посадочное место.
        FootprintRef footprintRef;
        // Точка привязки объекта.
        Org org;
        // Контакты компонента на плате.
        Xml::ArrayElem<Pin> Pins;
        // Монтажные отверстия.
        Xml::ArrayElem<Mnthole> Mntholes;
        // Атрибуты компонента.
        Xml::ArrayElem<Attribute> Attributes;

        /************************************************************************
         * Здесь находятся функции для работы с элементами класса CompInstance. *
         * Они не являются частью формата TopoR PCB.                            *
         * **********************************************************************/

        // Для отображения имени компонента
        operator QString() const { return name; }
        QTransform transform() const;
        /***********************************************************************/
    };

    // Описание одиночного контакта..
    struct FreePad {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Сторона объекта.
        Xml::Attr<side, NoOpt> side_;
        // Задаёт угол в градусах c точностью до тысячных долей.
        Xml::Attr<double> angle;
        // Признак фиксации.
        Xml::Attr<Bool> fixed;
        // Ссылка на стек контактных площадок.
        PadstackRef padstackRef;
        // Cсылка на цепь.
        Xml::Optional<NetRef> netRef;
        // Точка привязки объекта.
        Org org;
        QTransform transform() const;
    };

    // Версия раздела.
    Xml::Attr<QString> version;
    // Описание компонентов на плате (инстанции компонентов)
    Xml::ArrayElem<CompInstance> Components;
    // Описание одиночных контактов (инстанции компонентов)
    Xml::ArrayElem<FreePad> FreePads;

    /*****************************************************************************
     * Здесь находятся функции для работы с элементами класса ComponentsOnBoard. *
     * Они не являются частью формата TopoR PCB.                                 *
     * ***************************************************************************/

    // Добавление компонента
    /// \param name \brief Имя нового компонента. Если имя неуникально, будет добавлен префикс _
    /// \param units \brief текущие единицы измерения
    /// \param componentRef \brief ссылка на библиотеку компонентов
    /// \param footprintRef \brief ссылка на библиотеку посадочных мест
    /// \return  Имя нового компонента
    QString AddComponent(QString name, units units, const QString& componentRef, const QString& footprintRef);

    // Удаление компонента по имени
    /// \param name \brief уникальный имя компонента
    /// \return  true - если было произведено удаление, иначе (компонент не найден) - false
    bool RemoveComponent(const QString& name);

    // Индекс компонента
    /// \param name \brief уникальное имя компонента
    /// \return  индекс компонента или -1, если компонент отсутствует
    int ComponentIndexOf(const QString& name);

    // Переименование компонента
    /// \param oldname \brief старое имя компонента
    /// \param newname \brief новое имя компонента
    /// \return  индекс компонента, если было произведено переименование, -1, если компонент не найден
    int RenameComponent(const QString& oldname, const QString& newname);

    // Генерация уникального идентификатора
    /// \return  string like "ABCDEFGH"
    QString UniqueId();
    /*************************************************************************************/
};

} // namespace TopoR
