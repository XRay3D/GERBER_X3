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
/// Раздел «Стили надписей».
/// </summary>
class TextStyles : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание стиля надписей.
    /// </summary>
public:
    class TextStyle {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Параметр стиля надписей: название шрифта.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("fontName")] public string _fontName;
        QString _fontName;

        /// <summary>
        /// Параметр стиля надписей: высота символов в текущих единицах.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("height", DataType = "float")] public float _height;
        float _height = 0.0F;

        /// <summary>
        /// Параметр стиля надписей: жирность шрифта.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("bold")] public Bool _bold;
        Bool _bold = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _boldSpecified
        bool getBoldSpecified() const;

        /// <summary>
        /// Параметр стиля надписей: курсив.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("italic")] public Bool _italic;
        Bool _italic = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _italicSpecified
        bool getItalicSpecified() const;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Стили надписей.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("TextStyle")] public List<TextStyle> _TextStyles;
    std::vector<TextStyle*> _TextStyles;
    bool ShouldSerialize_TextStyles();

    /**********************************************************************
     * Здесь находятся функции для работы с элементами класса TextStyles. *
     * Они не являются частью формата TopoR PCB.                          *
     * ********************************************************************/

    /**********************************************************************/
};
// } // namespace TopoR_PCB_Classes
