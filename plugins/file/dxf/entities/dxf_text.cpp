/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_text.h"
#include "dxf_file.h"
#include "settings.h"
#include "tables/dxf_style.h"

#include <QFont>

namespace Dxf {

Text::Text(SectionParser* sp)
    : Entity{sp} {
}

void Text::parse(CodeData& code) {
    do {
        switch(code.code()) {
        case SubclassMarker       : break;
        case Thickness            : thickness = code; break;
        case FirstAlignmentPtX    : pt1.rx() = code; break;
        case FirstAlignmentPtY    : pt1.ry() = code; break;
        case FirstAlignmentPtZ    : break;
        case TextHeight           : textHeight = code; break;
        case Text_                : text = code.string(); break;
        case Rotation             : rotation = code; break;
        case RelativeScaleX       : break;
        case ObliqueAngle         : break;
        case TextStyleName        : textStyleName = code.string(); break;
        case TextGenerationFlags  : textGenerationFlag = code; break;
        case HorizontalJustType   : horizontalJustType = code; break;
        case VerticalJustType     : verticalJustType = code; break;
        case SecondAlignmentPointX: pt2.rx() = code; break;
        case SecondAlignmentPointY: pt2.ry() = code; break;
        case SecondAlignmentPointZ: break;
        case ExtrusionDirectionX  : break;
        case ExtrusionDirectionY  : break;
        case ExtrusionDirectionZ  : break;
        default                   : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Text::type() const { return Type::TEXT; }

QDebug operator<<(QDebug debug, const QFontMetricsF& fm) {
    return debug; // NOTE QDebug operator<<(QDebug debug, const QFontMetricsF& fm) {
    QDebugStateSaver saver{debug};
    debug.nospace() << u"FM("_s;
    debug.nospace() << u"\n\tascent: "_s << fm.ascent();
    debug.nospace() << u"\n\taverageCharWidth: "_s << fm.averageCharWidth();
    debug.nospace() << u"\n\tcapHeight: "_s << fm.capHeight();
    debug.nospace() << u"\n\tdescent: "_s << fm.descent();
    debug.nospace() << u"\n\theight: "_s << fm.height();
    debug.nospace() << u"\n\tleading: "_s << fm.leading();
    debug.nospace() << u"\n\tlineSpacing: "_s << fm.lineSpacing();
    debug.nospace() << u"\n\tlineWidth: "_s << fm.lineWidth();
    debug.nospace() << u"\n\tmaxWidth: "_s << fm.maxWidth();
    debug.nospace() << u"\n\tminLeftBearing: "_s << fm.minLeftBearing();
    debug.nospace() << u"\n\tminRightBearing: "_s << fm.minRightBearing();
    debug.nospace() << u"\n\toverlinePos: "_s << fm.overlinePos();
    debug.nospace() << u"\n\tstrikeOutPos: "_s << fm.strikeOutPos();
    debug.nospace() << u"\n\tunderlinePos: "_s << fm.underlinePos();
    debug.nospace() << u"\n\txHeight: "_s << fm.xHeight();
    debug.nospace() << ')';
    return debug;
}

DxfGo Text::toGo() const {
    qInfo("Text");

    // for (auto& code : data)

    double ascent{};
    double scaleX{};
    double scaleY{};

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
        QFontMetricsF fmf{font};
        scaleX = scaleY = std::max(style->fixedTextHeight, textHeight) / fmf.height();
        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        size = fmf.size(0, text);
    } else {
        font.setFamily(Settings::defaultFont());
        font.setPointSize(100);
        if(Settings::overrideFonts()) {
            font.setBold(Settings::boldFont());
            font.setItalic(Settings::italicFont());
        }
        QFontMetricsF fmf{font};
        scaleX = scaleY = textHeight / fmf.height();
        offset.ry() -= fmf.descent();
        ascent = fmf.ascent();
        size = fmf.size(0, text);
    }
    // qDebug(u"scale X %f Y %f"_s, scaleX, scaleY);
    switch(horizontalJustType) {
    case Left   : offset.rx(); break;                     // 0
    case Center : offset.rx() -= size.width() / 2; break; // 1
    case Right  : offset.rx() -= size.width(); break;     // 2
    case Aligned: break;                                  // 3
    case MiddleH: break;                                  // 4
    case Fit    : break;                                      // 5
    }

    switch(verticalJustType) {
    case Baseline: break;                            // 0
    case Bottom  : break;                            // 1
    case MiddleV : offset.ry() += ascent / 2; break; // 2
    case Top     : offset.ry() += ascent; break;          // 3
    }

    if(textGenerationFlag & MirroredInX)
        scaleX = -scaleX;
    if(textGenerationFlag & MirroredInY)
        scaleY = -scaleY;

    QPainterPath path;
    path.addText(offset, font, text);

    QTransform m;
    m.scale(u * scaleX, -u * scaleY);
    QPainterPath path2;
    for(auto& poly: path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.translate(pt2.x(), pt2.y());
    m2.rotate(rotation > 360 ? rotation * 0.01 : rotation);
    m2.scale(d, d);

    DxfGo go{id, {}, ~path2.toSubpathPolygons(m2)}; // return {id, {}, path2.toSubpathPolygons(m2)};
    return go;
}

void Text::write(QDataStream& stream) const {
    stream << text;
    stream << textStyleName;

    stream << pt1;
    stream << pt2;

    stream << textGenerationFlag;
    stream << horizontalJustType;
    stream << verticalJustType;

    stream << thickness;
    stream << textHeight;
    stream << rotation;
}

void Text::read(QDataStream& stream) {
    stream >> text;
    stream >> textStyleName;

    stream >> pt1;
    stream >> pt2;

    stream >> textGenerationFlag;
    stream >> horizontalJustType;
    stream >> verticalJustType;

    stream >> thickness;
    stream >> textHeight;
    stream >> rotation;
}

} // namespace Dxf

#include "moc_dxf_text.cpp"
