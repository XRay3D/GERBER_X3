#pragma once

#include "Commons.h"
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе u"Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г."_s.
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {

class Layer : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Имя объекта или ссылка на именованный объект.
    /// </summary>
public:
    QS_ATTR(QString, name) // ORIGINAL LINE: [XmlAttribute(u"name"_s)] public string _name;

    /// <summary>
    /// Тип слоя. Значение по умолчанию – Signal.
    /// </summary>

    QS_ATTR(layer_type, type) // ORIGINAL LINE: [XmlAttribute(u"type"_s)] public layer_type _type;

    /// <summary>
    /// Параметр слоя: слой содержит очертания компонентов.
    /// </summary>
    /// <remarks>! Для сигнальных, опорных, диэлектрических и документирующих слоёв параметр compsOutline отсутствует.</remarks>

    QS_ATTR(Bool, compsOutline) // ORIGINAL LINE: [XmlAttribute(u"compsOutline"_s)] public Bool _compsOutline;

    // ORIGINAL LINE: [XmlIgnore] public bool _compsOutlineSpecified
    bool getCompsOutlineSpecified() const;

    /// <summary>
    /// Параметр слоя: толщина.
    /// </summary>
    /// <remarks>! Для документирующих слоёв и слоёв с типом Assy параметр thickness отсутствует.</remarks>

    QS_ATTR(float, thickness); // ORIGINAL LINE: [XmlAttribute(u"thickness"_s, DataType = u"float"_s)] public float _thickness;

    // ORIGINAL LINE: [XmlIgnore] public bool _thicknessSpecified
    bool getThicknessSpecified() const;

    /*****************************************************************
     * Здесь находятся функции для работы с элементами класса Layer. *
     * Они не являются частью формата TopoR PCB.                     *
     * ***************************************************************/
public:
    Layer();

    Layer(const QString& name, layer_type type, Bool compsOutline, float thickness);

    QString ToString();
    /*****************************************************************/
};

/// <summary>
/// Раздел «Слои». (Обязательный раздел)
/// </summary>
class Layers : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание слоя.
    /// </summary>
public:
    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QS_FIELD(QString, version);

    /// <summary>
    /// Описание слоёв в стеке. Порядок описания должен соответствовать порядку слоёв в стеке.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"StackUpLayers"_s)][XmlArrayItem(u"Layer"_s)] public List<Layer> _StackUpLayers;
    // std::vector<Layer*> _StackUpLayers;
    QS_COLLECTION_OBJECTS(std::vector, Layer, StackUpLayers)

    bool ShouldSerialize_StackUpLayers();
    /// <summary>
    /// Описание слоёв вне стека.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"UnStackLayers"_s)][XmlArrayItem(u"Layer"_s)] public List<Layer> _UnStackLayers;
    // std::vector<Layer*> _UnStackLayers;
    QS_COLLECTION_OBJECTS(std::vector, Layer, UnStackLayers)

    bool ShouldSerialize_UnStackLayers();

    /******************************************************************
     * Здесь находятся функции для работы с элементами класса Layers. *
     * Они не являются частью формата TopoR PCB.                      *
     * ****************************************************************/
    /// <summary>
    /// Проверяет существование слоя, на который ссылается ссылка
    /// </summary>
    /// <param name=u"lref"_s>Ссылка на слой</param>
    /// <returns>true, если слой существует</returns>
    bool LayerStackUpContains(LayerRef* lref);
    /// <summary>
    /// Проверяет существование слоя, на который ссылается ссылка
    /// </summary>
    /// <param name=u"lref"_s>Ссылка на слой</param>
    /// <returns>true, если слой существует</returns>
    bool LayerUnStackContain(LayerRef* lref);
    /******************************************************************/
};
// } // namespace TopoR_PCB_Classes
