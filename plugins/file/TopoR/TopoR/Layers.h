#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Слои». (Обязательный раздел)
struct Layers {
    // Описание слоя.
    struct Layer {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Тип слоя. Значение по умолчанию – Signal.
        Xml::Attr<LayerType, NoOpt> type;
        // Параметр слоя: слой содержит очертания компонентов.
        /// \note !Для сигнальных, опорных, диэлектрических и документирующих слоёв параметр compsOutline отсутствует.
        Xml::Attr<Bool> compsOutline;
        // Параметр слоя: толщина.
        /// \note !Для документирующих слоёв и слоёв с типом Assy параметр thickness отсутствует.
        Xml::Optional<Xml::Attr<double, NoOpt>> thickness;
        /*****************************************************************
         * Здесь находятся функции для работы с элементами класса Layer. *
         * Они не являются частью формата TopoR PCB.                     *
         * ***************************************************************/
        // Layer() { }
        // Layer(const QString& name, LayerType type, Bool compsOutline, double thickness) {
        //     name_ = name;
        //     type_ = type;
        //     compsOutline_ = compsOutline;
        //     thickness_ = thickness;
        // }
        QString ToString() { return name; }

        bool isCompsOutline() const { return type == LayerType::Assy; }
        bool hasThickness() const { return type != LayerType::Assy; }
        /*****************************************************************/
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Описание слоёв в стеке. Порядок описания должен соответствовать порядку слоёв в стеке.
    Xml::ArrayElem<Layer> StackUpLayers;
    // Описание слоёв вне стека.
    Xml::ArrayElem<Layer> UnStackLayers;
    /******************************************************************
     * Здесь находятся функции для работы с элементами класса Layers. *
     * Они не являются частью формата TopoR PCB.                      *
     * ****************************************************************/
    // Проверяет существование слоя, на который ссылается ссылка
    /// \param '1 \brief Ссылка на слой
    /// \return  true, если слой существует
    bool LayerStackUpContains(LayerRef lref) const;
    // Проверяет существование слоя, на который ссылается ссылка
    /// \param '1 \brief Ссылка на слой
    /// \return  true, если слой существует
    bool LayerUnStackContain(LayerRef lref) const;
    /******************************************************************/
};

} // namespace TopoR
