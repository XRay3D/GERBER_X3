/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_entity.h"

namespace Dxf {

struct MText final : Entity {
    Q_GADGET

public:
    MText(SectionParser* sp);

    // Entity interface
    void parse(CodeData& code) override;
    Type type() const override { return Type::MTEXT; }
    GraphicObject toGo() const override;

    enum DataEnum {
        SubclassMarker = 100, //	100	Маркер подкласса (AcDbMText)
        InsertionPointPointX = 10, //	10	Точка вставки        //		Файл DXF: значение X; приложение: 3D-точка
        InsertionPointPointY = 20, //	20, 30	Файл DXF: значение Y и Z для точки вставки
        InsertionPointPointZ = 30,
        NominalTextHeight = 40, //	40	Номинальная (начальная) высота текста
        ReferenceRectangleWidth = 41, //	41	Ширина ссылочного прямоугольника
        /**/ AttachmentPoint = 71, //	71	Точки вставки:
        /**/ DrawingDirection = 72, //	72	Направление чертежа:
        TextString = 1, //	1	Текстовая строка. Если длина текстовой строки меньше 250 символов, все символы отображаются в группе с кодом 1. Если длина текстовой строки больше 250 символов, строка делится на фрагменты по 250 символов, которые отображаются в одном или нескольких кодах группы 3. Если используются коды группы 3, последняя группа — это группа 1, которая содержит менее 250 символов
        AdditionalText = 3, //	3	Дополнительный текст (всегда в виде фрагментов по 250 символов) (необязательно)
        TextStyleName = 7, //	7	Имя стиля текста (СТАНДАРТ, если не указано) (необязательно)
        ExtrusionDirectionX = 210, //	210	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        //		Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionY = 220, //	220, 230	Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        ExtrusionDirectionZ = 230,
        XAxisDirectionVector = 11, //	11	Вектор направления осиX( в МСК)
        //     Dxf: x value; App: 3dVector		=		,//		Файл DXF: значение X; приложение: 3D-вектор
        //     A GroupCode 50 (rotationAngleInRadians) PassedAsDxfInputIsConvertedToTheEquivalentDirectionVector (ifBothA Code 50 AndCodes 11, 21, 31 ArePassed, TheLastOneWins). ThisIsProvidedAsA ConvenienceForConversionsFromTextObjects		=		,//		Групповой код 50 (угол поворота в радианах), переданный как входные данные DXF, преобразуется в эквивалентный вектор направления (если передаются код 50 и коды 11, 21, 31, последний превалирует). Этот код служит для упрощения преобразования текстовых объектов
        YAxisDirectionVector = 21, //	21, 31	Файл DXF: значения Y и Z вектора направления оси X (в МСК)
        ZAxisDirectionVector = 31,
        HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity = 42, //	42	Горизонтальная ширина символов, образующих объект многострочного текста. Это значение всегда будет меньше либо равно значению группового кода 41 (только для чтения; игнорируется, если указано)
        VerticalHeightOfTheMtextEntity = 43, //	43	Вертикальная высота объекта многострочного текста (только для чтения; игнорируется, если указано)
        RotationAngleInRadians = 50, //	50	Угол поворота в радианах
        MtextLineSpacingStyle = 73, //	73	Стиль межстрочного интервала многострочного текста (необязательно):
        //     1 = AtLeast (tallerCharactersWillOverride)		=		,//		1 = не менее (более высокие символы переопределяют значение)
        //     2 = Exact (tallerCharactersWillNotOverride)		=		,//		2 = точно (более высокие символы не переопределяют значение)
        MtextLineSpacingFactor = 44, //	44	Коэффициент межстрочного интервала многострочного текста (необязательно):
        //     PercentageOfDefault(3 - on - 5) LineSpacingToBeApplied.ValidValuesRangeFrom 0.25 To 4.00 =, //		Применяется процент от межстрочного интервала по умолчанию (3 на 5). Допустимый диапазон значений — от 0,25 до 4,00
        BackgroundFillSetting = 90, //	90	Настройка заливки фона:
        //     0 = BackgroundFillOff		=		,//		0 = заливка фона откл.
        //     1 = UseBackgroundFillColor		=		,//		1 = использование цвета заливки фона
        //     2 = UseDrawingWindowColorAsBackgroundFillColor		=		,//		2 = использование цвета окна чертежа как цвета заливки фона
        //     BackgroundColor (ifRgbColor)		=	420 - 429	,//	420–429	Цвет фона (если используется цвет RGB)
        //     BackgroundColor (ifColorName)		=	430 - 439	,//	430–439	Цвет фона (если используется имя цвета)
        FillBoxScale = 45, //	45	Масштаб рамки заливки (необязательно):
        //     DeterminesHowMuchBorderThereIsAroundTheText.=, //		Определение величины границы вокруг текста.
        BackgroundFillColor = 63, //	63	Цвет заливки фона (необязательно):
        //     ColorToUseForBackgroundFillWhenGroupCode 90 Is 1. =, //		Цвет для заполнения фона, когда групповой код 90 равен 1.
        TransparencyOfBackgroundFillColor = 441, //	441	Прозрачность цвета заливки фона (не поддерживается)
        ColumnType = 75, //	75	Тип столбца
        ColumnCount = 76, //	76	Число столбцов
        ColumnFlowReversed = 78, //	78	Обратный порядок столбцов
        ColumnAutoheight = 79, //	79	Автоматическая высота столбцов
        ColumnWidth = 48, //	48	Ширина столбцов
        ColumnGutter = 49, //	49	Интервал между столбцами
        ColumnHeights = 50, //	50	Высота столбца; после этого кода идет число столбцов (Int16), а затем и число высот столбцов
    };
    Q_ENUM(DataEnum)

    enum AttachmentPointE {
        TopLeft = 1, //      вверху слева
        TopCenter = 2, //    вверху по центру
        TopRight = 3, //     вверху справа
        MiddleLeft = 4, //   посередине слева
        MiddleCenter = 5, // посередине по центру
        MiddleRight = 6, //  посередине справа
        BottomLeft = 7, //   снизу слева;
        BottomCenter = 8, // снизу по центру
        BottomRight = 9, //  снизу справа

    };
    Q_ENUM(AttachmentPointE)

    enum DrawingDirectionE {
        LeftToRight = 1, // слева направо
        TopToBottom = 3, // сверху вниз
        ByStyle = 5 //      по стилю (направление потока наследуется из связанного стиля текста)
    };

    Q_ENUM(DrawingDirectionE)

    QString textString; // Dxf::MText::TextString DC(1, Str "T1\\PT2")
    QString textStyleName; // Dxf::MText::TextStyleName DC(7, Str "_Default_")
    QPointF insertionPoint; // Dxf::MText::InsertionPointPointX DC(10, Dou 235)
    double rotation = 0; // Dxf::MText::RotationAngleInRadians DC(50, Dou 135)
    double nominalTextHeight = 0; // Dxf::MText::NominalTextHeight DC(40, Dou 2.54)
    double referenceRectangleWidth = 0; // Dxf::MText::ReferenceRectangleWidth DC(41, Dou 1)
    int16_t attachmentPoint = 0; // Dxf::MText::AttachmentPoint DC(71, I16 9)
    int16_t drawingDirection = 0; // Dxf::MText::DrawingDirection DC(72, I16 5)
};

}
