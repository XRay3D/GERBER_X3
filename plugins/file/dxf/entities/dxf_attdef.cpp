/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_attdef.h"
#include "dxf_insert.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include <QGraphicsEllipseItem>

#include <QPainter>

namespace Dxf {

void AttDef::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case SubclassMarkerAcDbText         : break; // 100 // Маркер подкласса (AcDbText)
        case Thickness                      : break; // 39 // Толщина (необязательно; значение по умолчанию = 0)
        case FirstAlignmentPtX              : break; // 10 // Первая точка выравнивания (в ОСК)
        case FirstAlignmentPtY              : break; // 20 // DXF: Alue; APP: 3DPoint    // Файл DXF: значение X; приложение: 3D-точка
        case FirstAlignmentPtZ              : break; // 30 // DXF: N_ valuesOfTextStartPoint (in_OCS) = 20, 30 , // Файл DXF: значения Y и Z начальной точки текста (в ОСК)
        case TextHeight                     : break; // 40 // Высота текста
        case DefaultValue                   : break; // 1 // Значение по умолчанию (строка)
        case TextRotation                   : break; // 50 // Поворот текста (необязательно; значение по умолчанию = 0)
        case RelativeXScaleFactor           : break; // 41 // Относительный масштабный коэффициент по оси X (ширина) (необязательно; значение по умолчанию = 1). Это значение корректируется при использовании вписываемого текста
        case ObliqueAngle                   : break; // 51 // Угол наклона (необязательно; значение по умолчанию = 0)
        case TextStyleName                  : break; // 7 // Имя стиля текста (необязательно; значение по умолчанию = STANDARD)
        case TextGenerationFlags            : break; // 71 // see_TEXT_groupCodes Флаги создания текста (необязательно; значение по умолчанию = 0); см. u"Групповые коды TEXT"_s
        case HorizontalTextJustificationType: break; // 72 // see_TEXT_groupCodes Тип выравнивания текста по горизонтали (необязательно; значение по умолчанию = 0); см. u"Групповые коды TEXT"_s
        case SecondAlignmentPointPtX        : break; // 11 // Вторая точка выравнивания (в ОСК) (необязательно)
        case SecondAlignmentPointPtY        : break; // 21 // Файл DXF: значения Y и Z второй точки выравнивания (в ОСК) (необязательно)
        case SecondAlignmentPointPtZ        : break; // 31
        case ExtrusionDirectionX            : break; // 210 // Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        case ExtrusionDirectionY            : break;             // 220 // Файл DXF: значение X; приложение: 3D-вектор
        case ExtrusionDirectionZ:
            break;                  // 230 // Файл DXF: значения Y и Z направления выдавливания
                                    // case SubclassMarkerAcDbAttributeDefinition: break; //100 // Маркер подкласса (AcDbAttributeDefinition)
        case VersionNumber : break; // 280 // Номер версии: 0 - 2010
        case PromptString  : break; // 3 // Строка запроса
        case TagString     : break; // 2 // Строка тега (не может содержать пробелы)
        case AttributeFlags: break; // 70 // Флаги атрибута:
        case FieldLength   : break;    // 73 // Длина поля (необязательно; значение по умолчанию = 0) (в настоящее время не используется)
        case VerticalTextJustificationType:
            break; // 74 // Тип выравнивания текста по вертикали (необязательно, по умолчанию = 0); см. групповой код 73 в разделе TEXT
            // case LockPositionFlag: /break; /280 // Флаг фиксирования положения. Фиксирование положения атрибута в пределах вхождения блока
            // case SubclassMarkerAcDbXrecord: break; //100 // Маркер подкласса (AcDbXrecord)
            // case DuplicateRecordCloningFlag: break; //280 // Флаг клонирования повторяющихся записей (выбор способа объединения повторяющихся записей):
            // case MTextFlag: break; //70 // Флаг мтекста:
            // case isReallyLockedFlag: break; //70 // Флаг isReallyLocked:
            // case NumberOfSecondaryAttributesOrAttributeDefinitions: break; //70 // Количество дополнительных атрибутов или определений атрибутов
            // case hardPointerIdOfSecondaryAttributeOrAttributeDefinition: /break; /340 // Идентификатор жесткого указателя дополнительных атрибутов или определений атрибутов
            // case AlignmentPointOfAttributeOrAttributeDefinitionX: break; //10 // Точка выравнивания атрибута или определения атрибута
            // case AlignmentPointOfAttributeOrAttributeDefinitionY: break; //20 // Файл DXF: значения Y и Z точки вставки
            // case AlignmentPointOfAttributeOrAttributeDefinitionZ: break; //30
            // case currentAnnotationScale: break; //40 // Текущий масштаб аннотаций
            // case attributeOrAttributeDefinitionTagString:break;  //2 // Строка тега атрибута или определения атрибута
            // case SubclassMarkerAcDbEntity: /break; /100 // Маркер подкласса (AcDbEntity)
        case LayerName:
            break; // 8 // Имя слоя
            // case SubclassMarkerAcDbMText: /break; /100 // Маркер подкласса (AcDbMText)
            // case InsertionPointX: break; //10 // Точка вставки
            // case InsertionPointY: break; //20 // Файл DXF: значения Y и Z точки вставки
            // case InsertionPointZ: break; //40 // Номинальная (начальная) высота текста
            // case ReferenceRectangleWidth: break; //41 // Ширина опорного прямоугольника
        case DefinedAnnotationHeight:
            break; // 46 // Определенная высота аннотаций
            // case AttachmentPoint: break; //71 // Точка вставки:
            // case DrawingDirection: break; //72 // Направление чертежа:
            // case TextString:break;  //1 // Текстовая строка
            // case AdditionalText:break;  //3 // Дополнительный текст (всегда в виде блоков по 250 символов) (необязательно)
            // case XValue:break;  //7 // Файл DXF: значение X; приложение: имя стиля 3DVectText (STANDARD, если не указано) (необязательно)
            // case XAxisDirectionVector: break; //11 // Вектор направления оси X (в МСК)
            // case YAxisDirectionVector: break; //21 // Файл DXF: значения Y и Z вектора направления оси X (в МСК)
            // case ZAxisDirectionVector: break; //31
        case HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity:
            break; // 42 // Ширина символов, образующих объект многострочного текста, по горизонтали.
            // case RotationAngleInRadians: break; //50 // Угол поворота в радианах
            // case MtextLineSpacingStyle: break; //73 // Стиль межстрочного интервала многострочного текста (необязательно):
        case MtextLineSpacingFactor: break; // 44 // Коэффициент межстрочного интервала многострочного текста (необязательно):
        case BackgroundFillSetting : break; // 90 // Настройка заливки фона:
        case BackgroundColor       : break;        // 63 // Цвет фона (если используется номер индекса цвета)
        case FillBoxScale:
            break; // 45 // Масштаб рамки заливки (необязательно):
            // case BackgroundFillColor: break; //63 // Цвет заливки фона (необязательно):
        case TransparencyOfBackgroundFillColor: break; // 441 // П+R[-99]C[-5]:RCрозрачность цвета заливки фона (не поддерживается)
        default                               : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type AttDef::type() const { return Type::ATTDEF; }

DxfGo AttDef::toGo() const {
    qInfo("AttDef");
    qInfo("TODO");
    return {};
}

} // namespace Dxf
