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
#include "dxf_insert.h"
#include <QFont>

#include <QGraphicsTextItem>
#include <QPainter>

namespace Dxf {
class TextItem final : public ::QGraphicsItem {
    const Text* text;
    QPainterPath path;
    QColor color;

public:
    TextItem(const Text* text, const QColor& color)
        : text(text)
        , color(color)
    {
        QFont f("Input Mono");
        f.setPointSizeF(text->textHeight);

        path.addText(text->pt2, f, text->text);
        text->pt2 + QPointF(-3, -1);
    }
    ~TextItem() = default;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override
    {
        return path.boundingRect();
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override
    {
        painter->setPen({ color, 0.0 });
        //painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        //painter->drawPath(path);

        {
            QFont f("Input");
            f.setPointSizeF(text->textHeight * 0.8);
            painter->setFont(f);
        }
        painter->save();
        painter->scale(1, -1);
        //painter->translate(0, text->pt2.x());
        painter->drawText(QPointF(text->pt2.x(), -text->pt2.y()), text->text);
        painter->restore();
    }
};

Text::Text(SectionParser* sp)
    : Entity(sp)
{
}

void Text::draw(const InsertEntity* const /*i*/) const
{
    //    if (i) {
    //        for (int r = 0; r < i->rowCount; ++r) {
    //            for (int c = 0; c < i->colCount; ++c) {
    //                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
    //                auto item = new TextItem(this, i->color());
    //                scene->addItem(item);
    //                i->transform(item, pt1 + tr);
    //                i->attachToLayer(item);
    //            }
    //        }
    //    } else {
    //        auto item = new TextItem(this, color());
    //        scene->addItem(item);
    //        item->setPos(pt1);
    //        attachToLayer(item);
    //    }
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
            text = QString(code);
            break;
        case Rotation:
            break;
        case RelativeScaleX:
            break;
        case ObliqueAngle:
            break;
        case TextStyleName:
            break;
        case TextGenerationFlags:
            textGenerationFlags = static_cast<TextGenerationFlagsE>(code.operator int64_t());
            break;
        case HorizontalJustType:
            horizontalJustType = static_cast<HorizontalJustTypeE>(code.operator int64_t());
            break;
        case VerticalJustType:
            verticalJustType = static_cast<VerticalJustTypeE>(code.operator int64_t());
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
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

GraphicObject Text::toGo() const {  return { sp->file, this, {}, {} }; }

}
