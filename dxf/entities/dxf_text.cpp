// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_text.h"
#include "dxf_file.h"
#include "settings.h"
#include "tables/dxf_style.h"

#include <QFont>

namespace Dxf {

Text::Text(SectionParser* sp)
    : Entity(sp)
{
}

void Text::parse(CodeData& code)
{
    do {
        switch (code.code()) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case FirstAlignmentPtX:
            pt1.rx() = code;
            break;
        case FirstAlignmentPtY:
            pt1.ry() = code;
            break;
        case FirstAlignmentPtZ:
            break;
        case TextHeight:
            textHeight = code;
            break;
        case Text_:
            text = code.string();
            break;
        case Rotation:
            rotation = code;
            break;
        case RelativeScaleX:
            break;
        case ObliqueAngle:
            break;
        case TextStyleName:
            textStyleName = code.string();
            break;
        case TextGenerationFlags:
            textGenerationFlag = code;
            break;
        case HorizontalJustType:
            horizontalJustType = code;
            break;
        case VerticalJustType:
            verticalJustType = code;
            break;
        case SecondAlignmentPointX:
            pt2.rx() = code;
            break;
        case SecondAlignmentPointY:
            pt2.ry() = code;
            break;
        case SecondAlignmentPointZ:
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

QDebug operator<<(QDebug debug, const QFontMetricsF& fm)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "FM(";
    debug.nospace() << "\n\tascent: " << fm.ascent();
    debug.nospace() << "\n\taverageCharWidth: " << fm.averageCharWidth();
    debug.nospace() << "\n\tcapHeight: " << fm.capHeight();
    debug.nospace() << "\n\tdescent: " << fm.descent();
    debug.nospace() << "\n\theight: " << fm.height();
    debug.nospace() << "\n\tleading: " << fm.leading();
    debug.nospace() << "\n\tlineSpacing: " << fm.lineSpacing();
    debug.nospace() << "\n\tlineWidth: " << fm.lineWidth();
    debug.nospace() << "\n\tmaxWidth: " << fm.maxWidth();
    debug.nospace() << "\n\tminLeftBearing: " << fm.minLeftBearing();
    debug.nospace() << "\n\tminRightBearing: " << fm.minRightBearing();
    debug.nospace() << "\n\toverlinePos: " << fm.overlinePos();
    debug.nospace() << "\n\tstrikeOutPos: " << fm.strikeOutPos();
    debug.nospace() << "\n\tunderlinePos: " << fm.underlinePos();
    debug.nospace() << "\n\txHeight: " << fm.xHeight();
    debug.nospace() << ')';
    return debug;
}

GraphicObject Text::toGo() const
{
    //    qDebug() << __FUNCTION__ << data.size();
    //    for (auto& code : data)
    //        qDebug() << "\t" << DataEnum(code.code()) << code;

    double ascent = 0.0;
    double scaleX = 0.0;
    double scaleY = 0.0;

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
        scaleX = scaleY = std::max(style->fixedTextHeight, textHeight) / fmf.height();
        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        size = fmf.size(0, text);
        qDebug() << __FUNCTION__ << fmf;
    } else {
        font.setFamily(Settings::defaultFont());
        font.setPointSize(100);
        if (Settings::overrideFonts()) {
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fmf(font);
        scaleX = scaleY = textHeight / fmf.height();
        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        size = fmf.size(0, text);
        qDebug() << __FUNCTION__ << fmf;
    }
    qDebug() << __FUNCTION__ << scaleX << scaleY;
    switch (horizontalJustType) {
    case Left: // 0
        offset.rx();
        break;
    case Center: // 1
        offset.rx() -= size.width() / 2;
        break;
    case Right: // 2
        offset.rx() -= size.width();
        break;
    case Aligned: // 3
        break;
    case MiddleH: // 4
        break;
    case Fit: // 5
        break;
    }

    switch (verticalJustType) {
    case Baseline: // 0
        break;
    case Bottom: // 1
        break;
    case MiddleV: // 2
        offset.ry() += ascent / 2;
        break;
    case Top: // 3
        offset.ry() += ascent;
        break;
    }

    if (textGenerationFlag & MirroredInX)
        scaleX = -scaleX;
    if (textGenerationFlag & MirroredInY)
        scaleY = -scaleY;

    QPainterPath path;
    path.addText(offset, font, text);

    QMatrix m;
    m.scale(1000 * scaleX, -1000 * scaleY);

    QPainterPath path2;
    for (auto& poly : path.toFillPolygons(m))
        path2.addPolygon(poly);

    QMatrix m2;
    m2.translate(pt2.x(), pt2.y());
    m2.rotate(rotation > 360 ? rotation * 0.01 : rotation);
    m2.scale(0.001, 0.001);
    Paths paths;
    for (auto& poly : path2.toFillPolygons(m2))
        paths.append(poly);

    return { sp->file, this, {}, paths };
}
}
