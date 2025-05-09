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
#include "shape.h"
#include "graphicsview.h"
#include "math.h"

#include <QIcon>
#include <assert.h>
#include <boost/pfr.hpp>

using Shapes::Handle;

namespace ShTxt {

Shape::Shape(QPointF pt1) {
    loadIData();
    paths_.resize(1);
    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{pt1, Handle::Center}
        };
        redraw();
    }
    App::grView().addItem(this);
}

Shape::~Shape() {
    // scene()->removeItem(this);
    std::erase(editor->shapes, this);
    editor->reset();
}

void Shape::redraw() {
    QPainterPath painterPath;

    QFont font;
    font.fromString(iData.font);
    font.setPixelSize(1000);

    painterPath.addText(QPointF(), font, iData.text);
    auto bRect = painterPath.boundingRect();

    QFontMetrics fm(font);
    const double capHeight = fm.capHeight();
    const double scale = iData.height / capHeight;
    // const double xyScale = 100.0 / iData.xy;

    QPointF handlePt;

    handlePt -= [width = bRect.width(), capHeight](auto handleAlign) -> QPointF {
        // clang-format off
        switch(handleAlign) {
        case BotCenter:   return {width / 2, 0            };
        case BotLeft:     return {                        };
        case BotRight:    return {width,     0            };
        case Center:      return {width / 2, capHeight / 2};
        case CenterLeft:  return {0,         capHeight / 2};
        case CenterRight: return {width,     capHeight / 2};
        case TopCenter:   return {width / 2, capHeight    };
        case TopLeft:     return {0,         capHeight    };
        case TopRight:    return {width,     capHeight    };
        default:          return {                        };
        }
        // clang-format on
    }(iData.handleAlign);

    QTransform transform;
    transform.translate(-bRect.left() * scale, 0);
    transform.translate(handlePt.x() * scale, handlePt.y() * scale);
    if(iData.side == Bottom) {
        transform.translate((bRect.right() + bRect.left()) * scale, 0);
        //        matrix.scale(
        //            -scale * iData.xy > 0.0 ? 1 * iData.xy : 1,
        //            -scale * iData.xy < 0.0 ? 1 / iData.xy : 1);
        transform.scale(-scale * (100 / iData.xy), -scale);
    } else {
        // matrix.scale(+scale * xyScale, -scale);
        transform.scale(+scale * (100 / iData.xy), -scale);
    }
    {
        QPainterPath tmpPainterPath;
        for(auto& polygon: painterPath.toSubpathPolygons()) // text to polygons
            tmpPainterPath.addPolygon(polygon);
        painterPath = std::move(tmpPainterPath);
        tmpPainterPath = QPainterPath();
        for(auto& polygon: painterPath.toSubpathPolygons(transform)) // transform polygons with matrix
            tmpPainterPath.addPolygon(polygon);
        painterPath = std::move(tmpPainterPath);
    }
    transform.reset();
    transform.translate(handles.front().x(), handles.front().y());
    transform.rotate(iData.angle - 360);

    paths_.clear();
    shape_ = {};

    Clipper clipper;
    clipper.AddClip(~painterPath.toSubpathPolygons(transform));
    clipper.Execute(ClipType::Union, FillRule::NonZero, paths_);
    for(auto& sp: paths_) {
        sp.emplace_back(sp.front());
        shape_.addPolygon(~sp);
    }

    setPos(1, 1), setPos(0, 0);

    assert(handles.size() == 1);
}

QString Shape::text() const { return iData.text; }

void Shape::setText(const QString& value) {
    iData.text = value;
    redraw();
}

Side Shape::side() const { return iData.side; }

void Shape::setSide(const Side& side) {
    iData.side = side;
    redraw();
}

void Shape::setPt(const QPointF& point) {
    handles.front().setPos(point);
    redraw();
}

bool Shape::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::CheckStateRole:
            setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            setText(value.toString());
            return true;
        }
        break;
    case FileTree::Column::Side:
        if(role == Qt::EditRole) {
            setSide(static_cast<Side>(value.toBool()));
            return true;
        }
        break;
    default:
        break;
    }
    return AbstractShape::setData(index, value, role);
}

Qt::ItemFlags Shape::flags(const QModelIndex& index) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return AbstractShape::flags(index) | Qt::ItemIsEditable;
    case FileTree::Column::Side:
        return AbstractShape::flags(index) | Qt::ItemIsEditable;
    default:
        return AbstractShape::flags(index);
    }
}

QVariant Shape::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole:
            return QString("%1 (%2, %3)")
                .arg(name())
                //                .arg(giId_)
                .arg(text());
        case Qt::EditRole:
            return text();
        default:
            return AbstractShape::data(index, role);
        }
    case FileTree::Column::Side:
        switch(role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[side()];
        case Qt::EditRole:
            return static_cast<bool>(side());
        default:
            return AbstractShape::data(index, role);
        }
    default:
        return AbstractShape::data(index, role);
    }
}

void Shape::write(QDataStream& stream) const {
    Block{stream}.write(iData);
    // AbstractShape::write(stream);
}

void Shape::readAndInit(QDataStream& stream) {
    Block{stream}.read(iData);
    // AbstractShape::readAndInit(stream);
    redraw();
}

void Shape::saveIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    boost::pfr::for_each_field(iData, [&settings](auto& field, auto index) { // TODO for_each_field_name
        settings.setValue(boost::pfr::get_name<index, InternalData>(), field);
    });
}

Shape::InternalData Shape::loadIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    boost::pfr::for_each_field(iData, [&settings]<typename Ty>(Ty& field, auto index) { // TODO for_each_field_name
        field = settings.value(boost::pfr::get_name<index, InternalData>()).template value<Ty>();
    });
    return iData;
}

void Shape::save() { iDataCopy = iData; }

void Shape::restore() {
    iData = std::move(iDataCopy);
    AbstractShape::redraw();
}

void Shape::ok() { saveIData(); }

QPainterPath Shape::shape() const { return shape_; }

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value) {
    if(change == GraphicsItemChange::ItemSelectedChange) editor->reset();
    return AbstractShape::itemChange(change, value);
}

QString Shape::name() const { return QObject::tr("Text"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-text"); }

} // namespace ShTxt

#include "moc_shape.cpp"
