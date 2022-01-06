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

#include "dxf_entity.h"
namespace Dxf {
struct AttDef final : Entity {
    AttDef(SectionParser* sp);

    // EntityInterface
public:
//    void draw(const InsertEntity* const i = nullptr) const override;
    void parse(CodeData& code) override;
    Type type() const override;;
    GraphicObject toGo() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

    enum DataEnum {
        SubclassMarkerAcDbText = 100, //	Маркер подкласса (AcDbText)
        Thickness = 39, //	Толщина (необязательно; значение по умолчанию = 0)
        FirstAlignmentPtX = 10, //	Первая точка выравнивания (в ОСК)
        FirstAlignmentPtY = 20, // DXF: Alue; APP: 3DPoint				//	Файл DXF: значение X; приложение: 3D-точка
        FirstAlignmentPtZ = 30, // DXF: N_ valuesOfTextStartPoint (in_OCS)	=	20, 30	,	//	Файл DXF: значения Y и Z начальной точки текста (в ОСК)

        TextHeight = 40, //	Высота текста
        DefaultValue = 1, //	Значение по умолчанию (строка)
        TextRotation = 50, //	Поворот текста (необязательно; значение по умолчанию = 0)
        RelativeXScaleFactor = 41, //	Относительный масштабный коэффициент по оси X (ширина) (необязательно; значение по умолчанию = 1). Это значение корректируется при использовании вписываемого текста
        ObliqueAngle = 51, //	Угол наклона (необязательно; значение по умолчанию = 0)
        TextStyleName = 7, //	Имя стиля текста (необязательно; значение по умолчанию = STANDARD)
        TextGenerationFlags = 71, //see_TEXT_groupCodes	Флаги создания текста (необязательно; значение по умолчанию = 0); см. "Групповые коды TEXT"
        HorizontalTextJustificationType = 72, //see_TEXT_groupCodes	Тип выравнивания текста по горизонтали (необязательно; значение по умолчанию = 0); см. "Групповые коды TEXT"
        SecondAlignmentPointPtX = 11, //	Вторая точка выравнивания (в ОСК) (необязательно)
        //	Файл DXF: значение X; приложение: 3D-точка
        //	Имеет значение, только если значения групповых кодов 72 или 74 не равны нулю
        SecondAlignmentPointPtY = 21, //	Файл DXF: значения Y и Z второй точки выравнивания (в ОСК) (необязательно)
        SecondAlignmentPointPtZ = 31,
        ExtrusionDirectionX = 210, //	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        ExtrusionDirectionY = 220, //	Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionZ = 230, //	Файл DXF: значения Y и Z направления выдавливания

        SubclassMarkerAcDbAttributeDefinition = 100, //	Маркер подкласса (AcDbAttributeDefinition)
        VersionNumber = 280, //	Номер версии: 0 - 2010
        PromptString = 3, //	Строка запроса
        TagString = 2, //	Строка тега (не может содержать пробелы)
        AttributeFlags = 70, //	Флаги атрибута:
        //        1 = AttributeIsInvisible (doesNotAppear)				//	1 = атрибут является невидимым (не отображается)
        //        2 = ThisIsA constantAttribute				//	2 = это постоянный атрибут
        //        4 = VerificationIsRequiredOnInputOfThisAttribute				//	4 = при вводе этого атрибута необходима проверка
        //        8 = AttributeIsPreset (noPromptDuringInsertion)				//	8 = атрибут заранее установлен (во время вставки запрос не выводится)
        FieldLength = 73, //	Длина поля (необязательно; значение по умолчанию = 0) (в настоящее время не используется)
        VerticalTextJustificationType = 74, //	Тип выравнивания текста по вертикали (необязательно, по умолчанию = 0); см. групповой код 73 в разделе TEXT
        LockPositionFlag = 280, //	Флаг фиксирования положения. Фиксирование положения атрибута в пределах вхождения блока
        SubclassMarkerAcDbXrecord = 100, //	Маркер подкласса (AcDbXrecord)
        DuplicateRecordCloningFlag = 280, //	Флаг клонирования повторяющихся записей (выбор способа объединения повторяющихся записей):
        //        1 = KeepExisting				//	1 = оставить существующие
        MTextFlag = 70, //	Флаг мтекста:
        //        2 = multilineAttribute				//	2 = многострочный атрибут
        //        4 = constantMultilineAttributeDefinition				//	4 = постоянное определение многострочного атрибута
        isReallyLockedFlag = 70, //	Флаг isReallyLocked:
        //        0 = unlocked				//	0 = разблокировано
        //        1 = locked				//	1 = заблокировано
        NumberOfSecondaryAttributesOrAttributeDefinitions = 70, //	Количество дополнительных атрибутов или определений атрибутов
        hardPointerIdOfSecondaryAttributeOrAttributeDefinition = 340, //	Идентификатор жесткого указателя дополнительных атрибутов или определений атрибутов
        AlignmentPointOfAttributeOrAttributeDefinitionX = 10, //	Точка выравнивания атрибута или определения атрибута
        //        DXF: XValue; APP: 3DPoint				//	Файл DXF: значение X; приложение: 3D-точка
        AlignmentPointOfAttributeOrAttributeDefinitionY = 20, //	Файл DXF: значения Y и Z точки вставки
        AlignmentPointOfAttributeOrAttributeDefinitionZ = 30,
        currentAnnotationScale = 40, //	Текущий масштаб аннотаций
        attributeOrAttributeDefinitionTagString = 2, //	Строка тега атрибута или определения атрибута
        //        EntityType (MTEXT)		0		//	Тип объекта ((MTEXT)
        SubclassMarkerAcDbEntity = 100, //	Маркер подкласса (AcDbEntity)
        //        AbsentOr_zeroIndicatesEntityIsInModelSpace. 1 indicatesEntityIsInPaperSpace 	=	67	,	//	Отсутствие значения или ноль указывают на наличие объекта в пространстве модели. 1 указывает на то, что объект находится в пространстве листа (необязательно)
        LayerName = 8, //	Имя слоя
        SubclassMarkerAcDbMText = 100, //	Маркер подкласса (AcDbMText)
        InsertionPointX = 10, //	Точка вставки
        //	Файл DXF: значение X; приложение: 3D-точка
        InsertionPointY = 20, //	Файл DXF: значения Y и Z точки вставки
        InsertionPointZ = 40, //	Номинальная (начальная) высота текста
        ReferenceRectangleWidth = 41, //	Ширина опорного прямоугольника
        DefinedAnnotationHeight = 46, //	Определенная высота аннотаций
        AttachmentPoint = 71, //	Точка вставки:
        //        1 = TopLeft; 2 = TopCenter; 3 = TopRight				//	1 = вверху слева; 2 = вверху по центру; 3 = вверху справа;
        //        4 = MiddleLeft; 5 = MiddleCenter; 6 = MiddleRight				//	4 = посередине слева; 5 = посередине по центру; 6 = посередине справа;
        //        7 = BottomLeft; 8 = BottomCenter; 9 = BottomRight				//	7 = снизу слева; 8 = снизу по центру; 9 = снизу справа
        DrawingDirection = 72, //	Направление чертежа:
        //        1 = LeftToRight				//	1 = слева направо
        //        3 = TopToBottom				//	3 = сверху вниз
        //        5 = ByStyle (theFlowDirectionIsInheritedFromTheAssociatedTextStyle)				//	5 = по стилю (направление наследуется из связанного стиля текста)
        TextString = 1, //	Текстовая строка
        //        IfTheTextStringIsLessThan 250 characters, allCharactersAppearIn_group 1. IfTheTextStringIs_greaterThan 250 characters, theStringIsDividedInto 250-characterChunks, whichAppearInOneOrMore_group 3 codes. If_group 3 codesAreUsed, theLast_groupIsA group 1 andHasFewerThan 250 characters.				//	Если строка содержит меньше 250 символов, все символы отображаются в группе с кодом 1. Если строка содержит больше 250 символов, строка делится на блоки по 250 символов, которые отображаются в одной или нескольких группах с кодом 3. Если используются группы с кодом 3, последней группой является группа 1, и она содержит менее 250 символов.
        AdditionalText = 3, //	Дополнительный текст (всегда в виде блоков по 250 символов) (необязательно)
        XValue = 7, //	Файл DXF: значение X; приложение: имя стиля 3DVectText (STANDARD, если не указано) (необязательно)
        //        ExtrusionDirectionX = 210, //	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        //        //	Файл DXF: значение X; приложение: 3D-вектор
        //        ExtrusionDirectionY = 220, //	Файл DXF: значения Y и Z направления выдавливания (необязательно)
        //        ExtrusionDirectionZ = 230,
        XAxisDirectionVector = 11, //	Вектор направления оси X (в МСК)
        //	Файл DXF: значение X; приложение: 3D-вектор
        YAxisDirectionVector = 21, //	Файл DXF: значения Y и Z вектора направления оси X (в МСК)
        ZAxisDirectionVector = 31,
        HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity = 42, //	Ширина символов, образующих объект многострочного текста, по горизонтали.
        //        ThisValueWillAlwaysBeEqualToOrLessThanTheValueOf_groupCode 41 (read-only, ignoredIfSupplied).				//	Это значение всегда будет равно или меньше, чем значение группового кода 41 (только для чтения; игнорируется, если предоставлено).
        //        VerticalHeightOfTheMtextEntity (read-only, ignoredIfSupplied)	=	43	,	//	Высота объекта многострочного текста по вертикали (только для чтения; игнорируется, если указано)
        RotationAngleInRadians = 50, //	Угол поворота в радианах
        MtextLineSpacingStyle = 73, //	Стиль межстрочного интервала многострочного текста (необязательно):
        //        1 = AtLeast (tallerCharactersWillOverride)				//	1 = не менее (более высокие символы переопределяют значение)
        //        2 = Exact (tallerCharactersWillNotOverride)				//	2 = точно (более высокие символы не переопределяют значение)
        MtextLineSpacingFactor = 44, //	Коэффициент межстрочного интервала многострочного текста (необязательно):
        //        PercentageOfDefault (3-on-5) lineSpacingToBeApplied.				//	Применяется процент от межстрочного интервала по умолчанию (3 на 5).
        //        ValidValuesRangeFrom 0.25 to 4.00				//	Допустимый диапазон значений — от 0,25 до 4,00
        BackgroundFillSetting = 90, //	Настройка заливки фона:
        //        0 = BackgroundFillOff				//	0 = заливка фона откл.
        //        1 = UseBackgroundFillColor				//	1 = использование цвета заливки фона
        //        2 = UseDrawingWindowColorAsBackgroundFillColor				//	2 = использование цвета окна чертежа как цвета заливки фона
        BackgroundColor = 63, //	Цвет фона (если используется номер индекса цвета)
        //        BackgroundColor (if_RGBColor)	=	420-429	,	//	Цвет фона (если используется цвет RGB)
        //        BackgroundColor (ifColorName)	=	430-439	,	//	Цвет фона (если используется имя цвета)
        FillBoxScale = 45, //	Масштаб рамки заливки (необязательно):
        //        DeterminesHowMuchBorderIsAroundTheText.				//	Определение размеров границы вокруг текста.
        BackgroundFillColor = 63, //	Цвет заливки фона (необязательно):
        //        ColorToUseForBackgroundFillWhen_groupCode 90 is 1.				//	Цвет, используемый для заливки фона, когда групповой код 90 равен 1.
        TransparencyOfBackgroundFillColor = 441, //	П+R[-99]C[-5]:RCрозрачность цвета заливки фона (не поддерживается)
    };
};
}
