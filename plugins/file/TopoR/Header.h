#pragma once

#include "Commons.h"
#include <string>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
class Units_Header : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
public:
    /// <summary>
    /// Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
    /// </summary>
    // dist _dist = static_cast<dist>(0);
    QS_ATTR(dist_, dist) // ORIGINAL LINE: [XmlAttribute("dist")] public dist _dist;

    /// <summary>
    /// Единицы измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
    /// </summary>
    // enum time_ _time = static_cast<enum time_>(0);
    QS_ATTR(time_, time) // ORIGINAL LINE: [XmlAttribute("time")] public time_ _time;
};

/// <summary>
/// Раздел «Заголовок файла».
/// </summary>
class Header : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Единицы измерений для всего файла.
    /// </summary>
public:
    /// <summary>
    /// Название формата файла.
    /// </summary>

public:
    QS_FIELD(QString, Format) // ORIGINAL LINE: [XmlElement("Format")] public string _Format;

    /// <summary>
    /// Версия формата.
    /// </summary>
    QS_FIELD(QString, Version) // ORIGINAL LINE: [XmlElement("Version")] public string _Version;

    /// <summary>
    /// Название программы, создавшей файл.
    /// </summary>
    QS_FIELD(QString, Program) // ORIGINAL LINE: [XmlElement("Program")] public string _Program;

    /// <summary>
    /// Дата и время создания файла (в произвольной форме).
    /// </summary>
    QS_FIELD(QString, Date) // ORIGINAL LINE: [XmlElement("Date")] public string _Date;

    /// <summary>
    /// Формат импортированного файла, из которого был получен дизайн.
    /// </summary>
    QS_FIELD(QString, OriginalFormat) // ORIGINAL LINE: [XmlElement("OriginalFormat")] public string _OriginalFormat;

    /// <summary>
    /// Импортированный файл. Путь к файлу задаётся относительно каталога содержащего файл проекта.
    /// </summary>
    QS_FIELD(QString, OriginalFile) // ORIGINAL LINE: [XmlElement("OriginalFile")] public string _OriginalFile;

    /// <summary>
    /// Единицы измерения для всего файла.
    /// </summary>
    QS_OBJECT(Units_Header, Units) // ORIGINAL LINE: [XmlElement("Units")] public Units_Header _Units;
};
// } // namespace TopoR_PCB_Classes
