#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Слои». (Обязательный раздел)
struct Layers {
    // Описание слоя.
    struct Layer {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Тип слоя. Значение по умолчанию – Signal.
        [[= XML::Attr]] layertype type{};
        // Параметр слоя: слой содержит очертания компонентов.
        // ! Для сигнальных, опорных, диэлектрических и документирующих слоёв параметр compsOutline отсутствует.
        [[= XML::Attr]] Bool compsOutline{};
        // public bool compsOutlineSpecified
        bool getCompsOutlineSpecified() const;
        // Параметр слоя: толщина.
        // ! Для документирующих слоёв и слоёв с типом Assy параметр thickness отсутствует.
        [[= XML::Attr]] double thickness{};
        // public bool thicknessSpecified
        bool getThicknessSpecified() const;
        /*****************************************************************
         * Здесь находятся функции для работы с элементами класса Layer. *
         * Они не являются частью формата TopoR PCB.                     *
         * ***************************************************************/
        Layer();
        Layer(const std::string& name, layertype type, Bool compsOutline, double thickness);
        std::string ToString();
        /*****************************************************************/
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Описание слоёв в стеке. Порядок описания должен соответствовать порядку слоёв в стеке.
    //[XmlArrayItem("Layer")] public List<Layer> StackUpLayers;
    [[= XML::Array]] std::vector<Layer> StackUpLayers;
    bool ShouldSerialize_StackUpLayers();
    // Описание слоёв вне стека.
    //[XmlArrayItem("Layer")] public List<Layer> UnStackLayers;
    [[= XML::Array]] std::vector<Layer> UnStackLayers;
    bool ShouldSerialize_UnStackLayers();
    /******************************************************************
     * Здесь находятся функции для работы с элементами класса Layers. *
     * Они не являются частью формата TopoR PCB.                      *
     * ****************************************************************/
    // Проверяет существование слоя, на который ссылается ссылка
    // <param name="lref">Ссылка на слой</param>   // true, если слой существует
    bool LayerStackUpContains(LayerRef lref);
    // Проверяет существование слоя, на который ссылается ссылка
    // <param name="lref">Ссылка на слой</param>   // true, если слой существует
    bool LayerUnStackContain(LayerRef lref);
    /******************************************************************/
};
} // namespace TopoR
