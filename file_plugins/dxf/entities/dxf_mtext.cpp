// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "dxf_mtext.h"
#include "dxf_file.h"
#include "settings.h"
#include "tables/dxf_style.h"

#include <QFont>

namespace Dxf {

MText::MText(SectionParser* sp)
    : Entity(sp) {
}

void MText::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarker:       //100
            break;                 //	100	Маркер подкласса (AcDbMText)
        case InsertionPointPointX: //10
            insertionPoint.rx() = code;
            break;                 //		Файл DXF: значение X; приложение: 3D-точка
        case InsertionPointPointY: //20
            insertionPoint.ry() = code;
            break;                 //	20, 30	Файл DXF: значение Y и Z для точки вставки
        case InsertionPointPointZ: //30
            break;
        case NominalTextHeight: //40
            nominalTextHeight = code;
            break;                    //	40	Номинальная (начальная) высота текста
        case ReferenceRectangleWidth: //41
            referenceRectangleWidth = code;
            break;            //	41	Ширина ссылочного прямоугольника
        case AttachmentPoint: //71
            attachmentPoint = code;
            break;             //	71	Точки вставки:
        case DrawingDirection: //72
            drawingDirection = code;
            break;       //	72	Направление чертежа:
        case TextString: //1
            textString = code.string();
            break;           //	1	Текстовая строка. Если длина текстовой строки меньше 250 символов, все символы отображаются в группе с кодом 1. Если длина текстовой строки больше 250 символов, строка делится на фрагменты по 250 символов, которые отображаются в одном или нескольких кодах группы 3. Если используются коды группы 3, последняя группа — это группа 1, которая содержит менее 250 символов
        case AdditionalText: //3
            break;           //	3	Дополнительный текст (всегда в виде фрагментов по 250 символов) (необязательно)
        case TextStyleName:  //7
            textStyleName = code.string();
            break;                //	7	Имя стиля текста (СТАНДАРТ, если не указано) (необязательно)
        case ExtrusionDirectionX: //210
            break;                //	210	Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        case ExtrusionDirectionY: //220
            break;                //	220, 230	Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        case ExtrusionDirectionZ: //230
            break;
        case XAxisDirectionVector: //11
            break;                 //	11	Вектор направления осиX( в МСК)
        case YAxisDirectionVector: //21
            break;                 //	21, 31	Файл DXF: значения Y и Z вектора направления оси X (в МСК)
        case ZAxisDirectionVector: //31
            break;
        case HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity: //42
            break;                                                   //	42	Горизонтальная ширина символов, образующих объект многострочного текста. Это значение всегда будет меньше либо равно значению группового кода 41 (только для чтения; игнорируется, если указано)
        case VerticalHeightOfTheMtextEntity:                         //43
            break;                                                   //	43	Вертикальная высота объекта многострочного текста (только для чтения; игнорируется, если указано)
        case RotationAngleInRadians:                                 //50 ??????????
            rotation = code;
            break;                              //	50	Угол поворота в радианах ??????????
        case MtextLineSpacingStyle:             //73
            break;                              //	73	Стиль межстрочного интервала многострочного текста (необязательно):
        case MtextLineSpacingFactor:            //44
            break;                              //	44	Коэффициент межстрочного интервала многострочного текста (необязательно):
        case BackgroundFillSetting:             //90
            break;                              //	90	Настройка заливки фона:
        case FillBoxScale:                      //45
            break;                              //	45	Масштаб рамки заливки (необязательно):
        case BackgroundFillColor:               //63
            break;                              //	63	Цвет заливки фона (необязательно):
        case TransparencyOfBackgroundFillColor: //441
            break;                              //	441	Прозрачность цвета заливки фона (не поддерживается)
        case ColumnType:                        //75
            break;                              //	75	Тип столбца
        case ColumnCount:                       //76
            break;                              //	76	Число столбцов
        case ColumnFlowReversed:                //78
            break;                              //	78	Обратный порядок столбцов
        case ColumnAutoheight:                  //79
            break;                              //	79	Автоматическая высота столбцов
        case ColumnWidth:                       //48
            break;                              //	48	Ширина столбцов
        case ColumnGutter:                      //49
            break;                              //	49	Интервал между столбцами
            //        case ColumnHeights: //50
            //            break; //	50	Высота столбца; после этого кода идет число столбцов (Int16), а затем и число высот столбцов
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type MText::type() const { return Type::MTEXT; }

extern QDebug operator<<(QDebug debug, const QFontMetricsF& fm);

GraphicObject MText::toGo() const {
    //    qDebug() << data.size();
    //    for (auto& code : data)
    //        qDebug() << "\t" << DataEnum(code.code()) << code;

    double ascent = 0.0;
    double descent = 0.0;
    double height = 0.0;
    double scaleX = 0.0;
    double scaleY = 0.0;

    QString text(textString);
    text.replace("\\P", "\n");
    QStringList list(text.split('\n'));
    qDebug() << list;
    QFont font;
    QPointF offset;
    QSizeF size;
    if (sp->file->styles().contains(textStyleName)) {
        Style* style = sp->file->styles()[textStyleName];
        font = style->font;
        if (Settings::overrideFonts()) {
            font.setFamily(Settings::defaultFont());
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fmf(font);
        //        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        descent = fmf.descent();
        height = fmf.height();
        size = fmf.size(0, text);
        scaleX = scaleY = std::max(style->fixedTextHeight, nominalTextHeight) / fmf.height();
        if (drawingDirection == ByStyle) {
            if (style->textGenerationFlag & Style::MirroredInX)
                scaleX = -scaleX;
            if (style->textGenerationFlag & Style::MirroredInY)
                scaleY = -scaleY;
        }
        qDebug() << fmf;
    } else {
        font.setFamily(Settings::defaultFont());
        font.setPointSize(100);
        if (Settings::overrideFonts()) {
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fmf(font);
        //        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        descent = fmf.descent();
        height = fmf.height();
        size = fmf.size(0, text);
        scaleX = scaleY = nominalTextHeight / fmf.height();
        qDebug() << fmf;
    }
    qDebug() << size << height << (size.height() / height) << ascent;

    switch (attachmentPoint) {
    case TopLeft: //      вверху слева
        offset.ry() += size.height() - descent;
        break;
    case TopCenter: //    вверху по центру
        offset.rx() -= size.width() / 2;
        offset.ry() += size.height() - descent;
        break;
    case TopRight: //     вверху справа
        offset.rx() -= size.width();
        offset.ry() += size.height() - descent;
        break;
    case MiddleLeft: //   посередине слева
        offset.ry() += size.height() / 2 - descent;
        offset.ry() -= descent;
        break;
    case MiddleCenter: // посередине по центру
        offset.rx() -= size.width() / 2;
        offset.ry() += size.height() / 2 - descent;
        break;
    case MiddleRight: //  посередине справа
        offset.rx() -= size.width();
        offset.ry() += size.height() / 2 - descent;
        break;
    case BottomLeft: //   снизу слева;
        break;
    case BottomCenter: // снизу по центру
        offset.rx() -= size.width() / 2;
        break;
    case BottomRight: //  снизу справа
        offset.rx() -= size.width();
        break;
    }

    QPainterPath path;

    for (int i = list.size() - 1; i >= 0; --i) {
        double x = 0.0;
        switch (attachmentPoint) {
        case TopLeft:    //      вверху слева
        case MiddleLeft: //   посередине слева
        case BottomLeft: //   снизу слева;
            break;
        case TopCenter:    //    вверху по центру
        case MiddleCenter: // посередине по центру
        case BottomCenter: // снизу по центру
            x = (size.width() - QFontMetricsF(font).size(Qt::TextSingleLine, list[i]).width()) * 0.5;
            break;
        case TopRight:    //     вверху справа
        case MiddleRight: //  посередине справа
        case BottomRight: //  снизу справа
            x = size.width() - QFontMetricsF(font).size(Qt::TextSingleLine, list[i]).width();
            break;
        }
        path.addText(offset - QPointF(-x, size.height() - height * (i + 1)), font, list[i]);
    }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMatrix m;
    m.scale(u * scaleX, -u * scaleY);

    QPainterPath path2;
    for (auto& poly : path.toFillPolygons(m))
        path2.addPolygon(poly);

    QMatrix m2;
    m2.translate(insertionPoint.x(), insertionPoint.y());
    //    m2.rotate(qRadiansToDegrees(rotationAngleInRadians));
    m2.rotate(rotation > 360 ? rotation * 0.01 : rotation);
    m2.scale(d, d);
    Paths paths;
    for (auto& poly : path2.toFillPolygons(m2))
        paths.push_back(poly);
#else
    QTransform m;
    m.scale(u * scaleX, -u * scaleY);

    QPainterPath path2;
    for (auto& poly : path.toFillPolygons(m))
        path2.addPolygon(poly);

    QTransform m2;
    m2.translate(insertionPoint.x(), insertionPoint.y());
    //    m2.rotate(qRadiansToDegrees(rotationAngleInRadians));
    m2.rotate(rotation > 360 ? rotation * 0.01 : rotation);
    m2.scale(d, d);
    Paths paths;
    for (auto& poly : path2.toFillPolygons(m2))
        paths.push_back(poly);
#endif
    return { id, {}, paths };
}

void MText::write(QDataStream& stream) const {
    stream << textString;
    stream << textStyleName;
    stream << insertionPoint;
    stream << rotation;
    stream << nominalTextHeight;
    stream << referenceRectangleWidth;
    stream << attachmentPoint;
    stream << drawingDirection;
}

void MText::read(QDataStream& stream) {
    stream >> textString;
    stream >> textStyleName;
    stream >> insertionPoint;
    stream >> rotation;
    stream >> nominalTextHeight;
    stream >> referenceRectangleWidth;
    stream >> attachmentPoint;
    stream >> drawingDirection;
}
} // namespace Dxf
