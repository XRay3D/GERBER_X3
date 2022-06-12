// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
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
AttDef::AttDef(SectionParser* sp)
    : Entity(sp) {
}

// void AttDef::draw(const InsertEntity* const i) const
//{
//     if (i) {
//         for (int r = 0; r < i->rowCount; ++r) {
//             for (int c = 0; c < i->colCount; ++c) {
//                 QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//                 QPointF rad(radius, radius);
//                 auto item = new ArcItem2(this, i->color());
//                 scene->addItem(item);
//                 //                item->setToolTip(layerName);
//                 i->transform(item, tr);
//                 i->attachToLayer(item);
//             }
//         }
//     } else {
//         auto item = new ArcItem2(this, color());
//         scene->addItem(item);
//         attachToLayer(item);
//     }
// }

void AttDef::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarkerAcDbText:          // 100
            break;                            //	Маркер подкласса (AcDbText)
        case Thickness:                       // 39
            break;                            //	Толщина (необязательно; значение по умолчанию = 0)
        case FirstAlignmentPtX:               // 10
            break;                            //	Первая точка выравнивания (в ОСК)
        case FirstAlignmentPtY:               // 20
            break;                            // DXF: Alue; APP: 3DPoint				//	Файл DXF: значение X; приложение: 3D-точка
        case FirstAlignmentPtZ:               // 30
            break;                            // DXF: N_ valuesOfTextStartPoint (in_OCS)	=	20, 30	,	//	Файл DXF: значения Y и Z начальной точки текста (в ОСК)
        case TextHeight:                      // 40
            break;                            //	Высота текста
        case DefaultValue:                    // 1
            break;                            //	Значение по умолчанию (строка)
        case TextRotation:                    // 50
            break;                            //	Поворот текста (необязательно; значение по умолчанию = 0)
        case RelativeXScaleFactor:            // 41
            break;                            //	Относительный масштабный коэффициент по оси X (ширина) (необязательно; значение по умолчанию = 1). Это значение корректируется при использовании вписываемого текста
        case ObliqueAngle:                    // 51
            break;                            //	Угол наклона (необязательно; значение по умолчанию = 0)
        case TextStyleName:                   // 7
            break;                            //	Имя стиля текста (необязательно; значение по умолчанию = STANDARD)
        case TextGenerationFlags:             // 71
            break;                            // see_TEXT_groupCodes	Флаги создания текста (необязательно; значение по умолчанию = 0); см. "Групповые коды TEXT"
        case HorizontalTextJustificationType: // 72
            break;                            // see_TEXT_groupCodes	Тип выравнивания текста по горизонтали (необязательно; значение по умолчанию = 0); см. "Групповые коды TEXT"
        case SecondAlignmentPointPtX:         // 11
            break;                            //	Вторая точка выравнивания (в ОСК) (необязательно)
        case SecondAlignmentPointPtY:         // 21
            break;                            //	Файл DXF: значения Y и Z второй точки выравнивания (в ОСК) (необязательно)
        case SecondAlignmentPointPtZ:         // 31
            break;
        case ExtrusionDirectionX: // 210
            break;                //	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        case ExtrusionDirectionY: // 220
            break;                //	Файл DXF: значение X; приложение: 3D-вектор
        case ExtrusionDirectionZ: // 230
            break;                //	Файл DXF: значения Y и Z направления выдавливания
            //        case SubclassMarkerAcDbAttributeDefinition: //100
            //            break; //	Маркер подкласса (AcDbAttributeDefinition)
        case VersionNumber:                 // 280
            break;                          //	Номер версии: 0 - 2010
        case PromptString:                  // 3
            break;                          //	Строка запроса
        case TagString:                     // 2
            break;                          //	Строка тега (не может содержать пробелы)
        case AttributeFlags:                // 70
            break;                          //	Флаги атрибута:
        case FieldLength:                   // 73
            break;                          //	Длина поля (необязательно; значение по умолчанию = 0) (в настоящее время не используется)
        case VerticalTextJustificationType: // 74
            break;                          //	Тип выравнивания текста по вертикали (необязательно, по умолчанию = 0); см. групповой код 73 в разделе TEXT
            //        case LockPositionFlag: //280
            //            break; //	Флаг фиксирования положения. Фиксирование положения атрибута в пределах вхождения блока
            //        case SubclassMarkerAcDbXrecord: //100
            //            break; //	Маркер подкласса (AcDbXrecord)
            //        case DuplicateRecordCloningFlag: //280
            //            break; //	Флаг клонирования повторяющихся записей (выбор способа объединения повторяющихся записей):
            //        case MTextFlag: //70
            //            break; //	Флаг мтекста:
            //        case isReallyLockedFlag: //70
            //            break; //	Флаг isReallyLocked:
            //        case NumberOfSecondaryAttributesOrAttributeDefinitions: //70
            //            break; //	Количество дополнительных атрибутов или определений атрибутов
            //        case hardPointerIdOfSecondaryAttributeOrAttributeDefinition: //340
            //            break; //	Идентификатор жесткого указателя дополнительных атрибутов или определений атрибутов
            //        case AlignmentPointOfAttributeOrAttributeDefinitionX: //10
            //            break; //	Точка выравнивания атрибута или определения атрибута
            //        case AlignmentPointOfAttributeOrAttributeDefinitionY: //20
            //            break; //	Файл DXF: значения Y и Z точки вставки
            //        case AlignmentPointOfAttributeOrAttributeDefinitionZ: //30
            //            break;
            //        case currentAnnotationScale: //40
            //            break; //	Текущий масштаб аннотаций
            //        case attributeOrAttributeDefinitionTagString: //2
            //            break; //	Строка тега атрибута или определения атрибута
            //        case SubclassMarkerAcDbEntity: //100
            //            break; //	Маркер подкласса (AcDbEntity)
        case LayerName: // 8
            break;      //	Имя слоя
            //        case SubclassMarkerAcDbMText: //100
            //            break; //	Маркер подкласса (AcDbMText)
            //        case InsertionPointX: //10
            //            break; //	Точка вставки
            //        case InsertionPointY: //20
            //            break; //	Файл DXF: значения Y и Z точки вставки
            //        case InsertionPointZ: //40
            //            break; //	Номинальная (начальная) высота текста
            //        case ReferenceRectangleWidth: //41
            //            break; //	Ширина опорного прямоугольника
        case DefinedAnnotationHeight: // 46
            break;                    //	Определенная высота аннотаций
            //        case AttachmentPoint: //71
            //            break; //	Точка вставки:
            //        case DrawingDirection: //72
            //            break; //	Направление чертежа:
            //        case TextString: //1
            //            break; //	Текстовая строка
            //        case AdditionalText: //3
            //            break; //	Дополнительный текст (всегда в виде блоков по 250 символов) (необязательно)
            //        case XValue: //7
            //            break; //	Файл DXF: значение X; приложение: имя стиля 3DVectText (STANDARD, если не указано) (необязательно)
            //        case XAxisDirectionVector: //11
            //            break; //	Вектор направления оси X (в МСК)
            //        case YAxisDirectionVector: //21
            //            break; //	Файл DXF: значения Y и Z вектора направления оси X (в МСК)
            //        case ZAxisDirectionVector: //31
            //            break;
        case HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity: // 42
            break;                                                   //	Ширина символов, образующих объект многострочного текста, по горизонтали.
            //        case RotationAngleInRadians: //50
            //            break; //	Угол поворота в радианах
            //        case MtextLineSpacingStyle: //73
            //            break; //	Стиль межстрочного интервала многострочного текста (необязательно):
        case MtextLineSpacingFactor: // 44
            break;                   //	Коэффициент межстрочного интервала многострочного текста (необязательно):
        case BackgroundFillSetting:  // 90
            break;                   //	Настройка заливки фона:
        case BackgroundColor:        // 63
            break;                   //	Цвет фона (если используется номер индекса цвета)
        case FillBoxScale:           // 45
            break;                   //	Масштаб рамки заливки (необязательно):
            //        case BackgroundFillColor: //63
            //            break; //	Цвет заливки фона (необязательно):
        case TransparencyOfBackgroundFillColor: // 441
            break;                              //	П+R[-99]C[-5]:RCрозрачность цвета заливки фона (не поддерживается)
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type AttDef::type() const { return Type::ATTDEF; }

GraphicObject AttDef::toGo() const { return {}; }

void AttDef::write(QDataStream& stream) const { }

void AttDef::read(QDataStream& stream) { }

} // namespace Dxf
