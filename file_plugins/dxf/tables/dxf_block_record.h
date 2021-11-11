/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "dxf_tableitem.h"
namespace Dxf {

struct BlockRecord : TableItem {
public:
    BlockRecord(SectionParser* sp);
    // TableItem interface
    enum VarType {
        SubclassMarker = 100, // Маркер подкласса (AcDbBlockTableRecord)
        BlockName = 2, // Имя блока
        Handle = 340, // Идентификатор/дескриптор жесткого указателя на связанный объект LAYOUT
        InsertionUnits = 70, // Единицы вставки блока.
        Explodability = 280, // Расчленяемость блока
        Scalability = 281, // Масштабируемость блока
        BinaryData = 310, // Файл DXF: двоичные данные предварительного просмотра растрового изображения (необязательно)
        ApplicationName = 1001, // Имя приложения расширенных данных, "ACAD" (необязательно)
        XdataString = 1000, //  Данные строк расширенных данных, "DesignCenter Data" (необязательно)
        BeginXdata = 1002, // Начало расширенных данных, "{" (необязательно)
        AutodeskDesignCenterVersionNumber = 1070, // Номер версии Центра управления AutoCAD
        InsertUnits = 1070, // Единицы вставки:
        //        0 = безразмерн.
        //        1 = дюймы
        //        2 = футы
        //        3 = мили
        //        4 = миллиметры
        //        5 = сантиметры
        //        6 = метры
        //        7 = километры
        //        8 = микродюймы
        //        9 = милы
        //        10 = ярды
        //        11 = ангстремы
        //        12 = нанометры
        //        13 = микроны
        //        14 = дециметры
        //        15 = декаметры
        //        16 = гектометры
        //        17 = гигаметры
        //        18 = астрономические единицы
        //        19 = световые годы
        //        20 = парсеки
        //        21 = футы США
        //        22 = дюймы США
        //        23 = ярды США
        //        24 = мили США
        EndXdata = 1002, //  Конец расширенных данных, "}"
    };

    enum Units {
        Unitless = 0, //
        Inches = 1, //
        Feet = 2, //
        Miles = 3, //
        Millimeters = 4, //
        Centimeters = 5, //
        Meters = 6, //
        Kilometers = 7, //
        Microinches = 8, //
        Mils = 9, //
        Yards = 10, //
        Angstroms = 11, //
        Nanometers = 12, //
        Microns = 13, //
        Decimeters = 14, //
        Decameters = 15, //
        Hectometers = 16, //
        Gigameters = 17, //
        AstronomicalUnits = 18, //
        LightYears = 19, //
        Parsecs = 20, //
    };

public:
    void
    parse(CodeData& code) override;
    Type type() const override { return TableItem::BLOCK_RECORD; };
};

}
