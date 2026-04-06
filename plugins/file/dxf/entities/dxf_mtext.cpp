/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
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
    : Entity{sp} {
}

void MText::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case SubclassMarker                                        : break;                                 // 100 Маркер подкласса (AcDbMText)
        case InsertionPointPointX                                  : insertionPoint.rx() = code; break;     // 10 // Файл DXF: значение X; приложение: 3D-точка
        case InsertionPointPointY                                  : insertionPoint.ry() = code; break;     // 20 // 20, 30 Файл DXF: значение Y и Z для точки вставки
        case InsertionPointPointZ                                  : break;                                 // 30 //
        case NominalTextHeight                                     : nominalTextHeight = code; break;       // 40 Номинальная (начальная) высота текста
        case ReferenceRectangleWidth                               : referenceRectangleWidth = code; break; // 41 Ширина ссылочного прямоугольника
        case AttachmentPoint                                       : attachmentPoint = code; break;         // 71 Точки вставки:
        case DrawingDirection                                      : drawingDirection = code; break;        // 72 Направление чертежа:
        case TextString                                            : textString = code.string(); break;     // 1 Текстовая строка. Если длина текстовой строки меньше 250 символов, все символы отображаются в группе с кодом 1. Если длина текстовой строки больше 250 символов, строка делится на фрагменты по 250 символов, которые отображаются в одном или нескольких кодах группы 3. Если используются коды группы 3, последняя группа — это группа 1, которая содержит менее 250 символов
        case AdditionalText                                        : break;                                 // 3 Дополнительный текст (всегда в виде фрагментов по 250 символов) (необязательно)
        case TextStyleName                                         : textStyleName = code.string(); break;  // 7 Имя стиля текста (СТАНДАРТ, если не указано) (необязательно)
        case ExtrusionDirectionX                                   : break;                                 // 210 Направление выдавливания (необязательно; значение по умолчанию = 0, 0, 1)
        case ExtrusionDirectionY                                   : break;                                 // 220 // 220, 230 Файл DXF: значения Y и Z для направления выдавливания (необязательно)
        case ExtrusionDirectionZ                                   : break;                                 // 230 //
        case XAxisDirectionVector                                  : break;                                 // 11 Вектор направления осиX( в МСК)
        case YAxisDirectionVector                                  : break;                                 // 21 // 21, 31 Файл DXF: значения Y и Z вектора направления оси X (в МСК)
        case ZAxisDirectionVector                                  : break;                                 // 31 //
        case HorizontalWidthOfTheCharactersThatMakeUpTheMtextEntity: break;                                 // 42 Горизонтальная ширина символов, образующих объект многострочного текста. Это значение всегда будет меньше либо равно значению группового кода 41 (только для чтения; игнорируется, если указано)
        case VerticalHeightOfTheMtextEntity                        : break;                                 // 43 Вертикальная высота объекта многострочного текста (только для чтения; игнорируется, если указано)
        case RotationAngleInRadians                                : rotation = code; break;                // 50 ?????????? // 50 Угол поворота в радианах ??????????
        case MtextLineSpacingStyle                                 : break;                                 // 73 Стиль межстрочного интервала многострочного текста (необязательно):
        case MtextLineSpacingFactor                                : break;                                 // 44 Коэффициент межстрочного интервала многострочного текста (необязательно):
        case BackgroundFillSetting                                 : break;                                 // 90 Настройка заливки фона:
        case FillBoxScale                                          : break;                                 // 45 Масштаб рамки заливки (необязательно):
        case BackgroundFillColor                                   : break;                                 // 63 Цвет заливки фона (необязательно):
        case TransparencyOfBackgroundFillColor                     : break;                                 // 441 Прозрачность цвета заливки фона (не поддерживается)
        case ColumnType                                            : break;                                 // 75 Тип столбца
        case ColumnCount                                           : break;                                 // 76 Число столбцов
        case ColumnFlowReversed                                    : break;                                 // 78 Обратный порядок столбцов
        case ColumnAutoheight                                      : break;                                 // 79 Автоматическая высота столбцов
        case ColumnWidth                                           : break;                                                                            // 48 Ширина столбцов
        case ColumnGutter:
            break; // 49 Интервал между столбцами
            // case ColumnHeights: break; //50            // 50 Высота столбца; после этого кода идет число столбцов (Int16), а затем и число высот столбцов
        default: Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type MText::type() const { return Type::MTEXT; }

extern QDebug operator<<(QDebug debug, const QFontMetricsF& fm);

DxfGo MText::toGo() const {
    qInfo("MText");
    // double ascent = {};
    double descent = {};
    double height = {};
    double scaleX = {};
    double scaleY = {};

    QString text{textString};
    text.replace(u"\\P"_s, u"\n"_s);
    QStringList list(text.split(u';').back().split(u'\n'));
    QFont font;
    QPointF offset;
    QSizeF size;
    if(sp->file->styles().contains(textStyleName)) {
        Style* style = sp->file->styles()[textStyleName];
        font = style->font;
        if(Settings::overrideFonts()) {
            font.setFamily(Settings::defaultFont());
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fm{font};
        // offset.ry() -= fmf.descent();
        // ascent = fm.ascent();
        descent = fm.descent();
        height = fm.height();
        size = fm.size(0, text);
        scaleX = scaleY = std::max(style->fixedTextHeight, nominalTextHeight) / fm.height();
        if(drawingDirection == ByStyle) {
            if(style->textGenerationFlag & Style::MirroredInX)
                scaleX = -scaleX;
            if(style->textGenerationFlag & Style::MirroredInY)
                scaleY = -scaleY;
        }
    } else {
        font.setFamily(Settings::defaultFont());
        font.setPointSize(100);
        if(Settings::overrideFonts()) {
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fm{font};
        // offset.ry() -= fmf.descent();
        // ascent = fm.ascent();
        descent = fm.descent();
        height = fm.height();
        size = fm.size(0, text);
        scaleX = scaleY = nominalTextHeight / fm.height();
    }

    [&] {
        switch(attachmentPoint) {
        case TopLeft     : return offset += {0, size.height() - descent};                     // вверху слева
        case TopCenter   : return offset += {-size.width() / 2, size.height() - descent};     // вверху по центру
        case TopRight    : return offset += {-size.width(), size.height() - descent};         // вверху справа
        case MiddleLeft  : return offset += {size.height() / 2 - descent, -descent};          // посередине слева
        case MiddleCenter: return offset += {-size.width() / 2, size.height() / 2 - descent}; // посередине по центру
        case MiddleRight : return offset += {-size.width(), size.height() / 2 - descent};     // посередине справа
        case BottomLeft  : return offset;                                                     // снизу слева;
        case BottomCenter: return offset += {-size.width() / 2, 0};                           // снизу по центру
        case BottomRight : return offset += {-size.width(), 0};                               // снизу справа
        default          : return offset;
        }
    }();

    QPainterPath path;

    for(int i = list.size() - 1; i >= 0; --i) {
        double x = {};
        switch(attachmentPoint) {
        case TopLeft: // вверху слева
        case MiddleLeft:
        case BottomLeft:                                                                                                    // снизу слева; break; // посередине слева
        case TopCenter:                                                                                                     // вверху по центру
        case MiddleCenter:                                                                                                  // посередине по центру
        case BottomCenter: x = (size.width() - QFontMetricsF(font).size(Qt::TextSingleLine, list[i]).width()) * 0.5; break; // снизу по центру
        case TopRight    :                                                                                                  // вверху справа
        case MiddleRight :                                                                                                  // посередине справа
        case BottomRight : x = size.width() - QFontMetricsF(font).size(Qt::TextSingleLine, list[i]).width(); break;          // снизу справа
        }
        path.addText(offset - QPointF(-x, size.height() - height * (i + 1)), font, list[i]);
    }

    QTransform m;
    m.scale(u * scaleX, -u * scaleY);

    QPainterPath path2;
    for(auto& poly: path.toSubpathPolygons(m))
        path2.addPolygon(poly);

    QTransform m2;
    m2.translate(insertionPoint.x(), insertionPoint.y());
    // m2.rotate(qRadiansToDegrees(rotationAngleInRadians));
    m2.rotate(rotation > 360 ? rotation * 0.01 : rotation);
    m2.scale(d, d);

    DxfGo go{id, {}, ~path2.toSubpathPolygons(m2)}; // return {id, {}, path2.toSubpathPolygons(m2)};
    return go;
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

#include "moc_dxf_mtext.cpp"
