/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbrcomp_item.h"

#include "abstract_file.h"
#include "graphicsview.h"

#include <QPainter>
#include <QTimer>
#include <future>

namespace Gerber::Comp {

Item::Item(const Component& component, AbstractFile* file)
    : Gi::Item(file)
    , component_(component) {
    component.setitem(this);
    pathPins.resize(component_.pins().size());
    for(auto&& poly: component_.footprint())
        shape_.addPolygon(poly);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setToolTip(component_.toolTip());
}

QRectF Item::boundingRect() const {
    return shape().boundingRect();
}

QPainterPath Item::shape() const {
    if(!qFuzzyCompare(scale_, App::grView().scaleFactor()))
        scale_ = App::grView().scaleFactor();
    return shape_;
}

void drawText(QPainter* painter, const QString& str, const QColor& color, QPointF pt, double scale) {
    painter->save();
    static QFont f("Consolas");
    f.setPixelSize(20);
    f.setBold(true);
    const QRectF textRect = QFontMetricsF(f).boundingRect(QRectF(), Qt::AlignLeft, str);
    pt.rx() -= textRect.width() * scale * 0.5;
    pt.ry() += textRect.height() * scale * 0.5;
    painter->translate(pt);
    painter->scale(scale, -scale);
    painter->setPen(color);
    painter->setFont(f);
    painter->drawText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 2), str);
    painter->restore();
}

void Item::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    auto color{file_->color()};
    painter->setBrush(color);
    color.setAlpha(255);
    painter->setPen({selected_ ? Qt::red : color, 2 * scale_});
    auto fillPolygons{shape_.toFillPolygons()};
    if(fillPolygons.size()) {
        for(auto&& poly: fillPolygons)
            painter->drawPolygon(poly);
    } else {
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(shape_);
    }

    constexpr int s = 10;

    painter->setBrush(Qt::NoBrush);
    double min = std::min(shape_.boundingRect().width(), shape_.boundingRect().height());
    double k = std::min(min, scale_ * s);
    painter->drawLine(
        component_.referencePoint() + QPointF{k, k},
        component_.referencePoint() - QPointF{k, k});
    painter->drawLine(
        component_.referencePoint() + QPointF{-k, k},
        component_.referencePoint() - QPointF{-k, k});
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(
        {
            component_.referencePoint() + QPointF{k, k},
            component_.referencePoint() - QPointF{k, k}
    });
    if(scale_ < 0.05) {
        double size = std::min(min, scale_ * s);
        for(const auto& [number, description, pos]: component_.pins()) {
            Q_UNUSED(number)
            Q_UNUSED(description)
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(color, 2 * scale_, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->drawLine(pos + QPointF{+size, +size}, pos - QPointF{+size, +size});
            painter->drawLine(pos + QPointF{-size, +size}, pos - QPointF{-size, +size});
        }
        for(const auto& [path, pos]: pathPins) {
            painter->save();
            painter->translate(pos);
            painter->scale(scale_, -scale_);
            painter->setPen(QPen(Qt::black, 4.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            painter->drawPath(path);
            painter->restore();
        }
    }

    drawText(painter,
        component_.refdes(),
        Qt::red,
        shape_.boundingRect().center(),
        scale_);

    if(scale_ < 0.05)
        for(const auto& [number, description, pos]: component_.pins())
            drawText(painter,
                QString(description + '(' + number + ')'),
                App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF,
                pos + QPointF(0, scale_ * 60),
                scale_);
}

Paths Item::paths(int) const { return {}; }

} // namespace Gerber::Comp
