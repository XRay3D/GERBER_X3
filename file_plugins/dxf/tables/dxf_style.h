/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "dxf_abstracttable.h"
#include "dxf_types.h"

#include <QFont>

namespace Dxf {

struct Style : AbstractTable {
public:
    Style(SectionParser* sp);
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return AbstractTable::STYLE; };

    enum DataCode {
        SubclassMarker = 100, //	Маркер подкласса (AcDbTextStyleTableRecord)
        StyleName = 2, //	Имя стиля
        /**/ StandardFlag = 70, //	Стандартные значения флагов (кодовые битовые значения):
        FixedTextHeight = 40, //	Фиксированная высота текста; значение 0, если нефиксированная
        WidthFactor = 41, //	Коэффициент сжатия
        ObliqueAngle = 50, //	Угол наклона
        /**/ TextGenerationFlag = 71, //	Флаги создания текста:
        LastHeightUsed = 42, //	Последняя использованная высота
        PrimaryFontFileName = 3, //	Имя файла основного шрифта
        BigfontFileName = 4, //	Имя файла большого шрифта; значение пусто, если файл отсутствует
        ALongValueWhichContainsATruetypeFontsPitchAndFamily_CharacterSet_AndItalicAndBoldFlags = 1071, //	Длинное значение, содержащее шаг и семейство шрифта TrueType, набор символов и флаги полужирного шрифта и курсива
        FontFamily = 1000, //
    };

    enum StandardFlags {
        ThisEntryDescribesAShape = 1, //	1 = если задано, эта запись описывает форму
        VerticalText = 4, //	4 = вертикальный текст
        TableEntryIsExternallyDependentOnAnXref = 16, //	16 = если задано это значение, запись таблицы внешне зависима от внешней ссылки
        TheExternallyDependentXrefHasBeenSuccessfullyResolved = 32, //	32 = если заданы и этот бит, и бит 16, внешне зависимая внешняя ссылка успешно разрешается
        TheTableEntryWasReferencedByAtLeastOneEntityInTheDrawingTheLastTimeTheDrawingWasEdited = 64, //	64 = если задано это значение, то в тот момент, когда чертеж редактировался в последний раз, на запись таблицы ссылался хотя бы один объект на чертеже. (Этот флаг нужен для команд AutoCAD. Его можно игнорировать в большинстве программ для чтения файлов DXF и не нужно задавать в программах, записывающих файлы DXF)
    };

    enum TextGenerationFlags {
        Norm = 0, // my val
        MirroredInX = 2, //	2 = текст в обратном направлении (зеркально отражен по X)
        MirroredInY = 4, //	4 = текст перевернут (зеркально отражен по Y)
    };

    QString styleName;
    double fixedTextHeight = 0;
    int16_t textGenerationFlag = 0;
    int16_t standardFlag = 0;
    QFont font;
};

}
