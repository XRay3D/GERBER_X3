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
#include "shape.h"
#include "graphicsview.h"
#include "math.h"

// #include <boost/pfr.hpp>

using Shapes::Handle;

namespace ShTxt {

Shape::Shape(Shapes::Plugin* plugin, QPointF pt1)
    : AbstractShape{plugin} {
    loadData();
    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{pt1, Handle::Center}
        };
        redraw();
    }
    App::grView().addItem(this);
}

void Shape::redraw() {

    txtData.font.setPixelSize(100);

    QFontMetrics fm{txtData.font};
    const double capHeight = fm.capHeight();
    const double scale     = txtData.height / capHeight;

    QPainterPath painterPath; // TODO align multiline text
    for(double i{}; auto&& txt: txtData.text.split(u'\n'))
        painterPath.addText({0.0, fm.height() * i++}, txtData.font, txt);

    auto bRect = painterPath.boundingRect();
    // const double xyScale = 100.0 / txtData.xy;

    QPointF handlePt = [width = bRect.width(), capHeight](auto handleAlign) -> QPointF {
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
    }(txtData.handleAlign)
        * -1;

    QTransform transform;
    transform.translate(-bRect.left() * scale, 0);
    transform.translate(handlePt.x() * scale, handlePt.y() * scale);
    transform.translate(handles[0].x(), handles[0].y());
    if(txtData.side == Bottom) {
        transform.translate((bRect.right() + bRect.left()) * scale, 0);
        transform.scale(-scale * (txtData.xy / 100.), -scale);
    } else {
        transform.scale(+scale * (txtData.xy / 100.), -scale);
    }

    shape_ = transform.map(painterPath);

    // {
    //     // text to polygons
    //     shape_.clear();
    //     for(auto& polygon: painterPath.toSubpathPolygons())
    //         shape_.addPolygon(polygon);
    //     painterPath.clear();
    //     for(auto& polygon: shape_.toSubpathPolygons(transform)) // transform polygons with matrix
    //         painterPath.addPolygon(polygon);
    // }

    // reverse
    // transform.reset();
    // transform.translate(handles.front().x(), handles.front().y());
    // transform.rotate(txtData.angle - 360);

#if 0
    shape_ = transform.map(painterPath);
#else
    shape_.clear();
    Paths64 paths;
    cl::Clipper64 clipper;
    clipper.AddClip(~painterPath.toSubpathPolygons(transform));
    clipper.Execute(cl::ClipType::Union, cl::FillRule::NonZero, paths);
    for(auto& sp: paths) {
        sp.emplace_back(sp.front());
        shape_.addPolygon(~sp);
    }
#endif
    curves_ = toCurves(shape_);

    setPos(1, 1), setPos(0, 0);

    assert(handles.size() == 1);
}

QString Shape::text() const { return txtData.text; }

void Shape::setText(const QString& value) {
    txtData.text = value;
    redraw();
}

Side Shape::side() const { return txtData.side; }

void Shape::setSide(const Side& side) {
    txtData.side = side;
    redraw();
}

void Shape::setPt(const QPointF& point) {
    handles.front() = point;
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
    default: break;
    }
    return AbstractShape::setData(index, value, role);
}

Qt::ItemFlags Shape::flags(const QModelIndex& index) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible: return AbstractShape::flags(index) | Qt::ItemIsEditable;
    case FileTree::Column::Side            : return AbstractShape::flags(index) | Qt::ItemIsEditable;
    default                                : return AbstractShape::flags(index);
    }
}

QVariant Shape::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole: return u"%1 (ID: %2)"_s.arg(text()).arg(id());
        case Qt::EditRole   : return text();
        default             : return AbstractShape::data(index, role);
        }
    case FileTree::Column::Side:
        switch(role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: return sideStrList[side()];
        case Qt::EditRole   : return static_cast<bool>(side());
        default             : return AbstractShape::data(index, role);
        }
    default: return AbstractShape::data(index, role);
    }
}

void Shape::write(QDataStream& stream) const {
    Block{stream}.write(txtData);
    // AbstractShape::write(stream);
}

void Shape::readAndInit(QDataStream& stream) {
    Block{stream}.read(txtData);
    AbstractShape::redraw();
}

void Shape::saveData() {
    QSettings settings;
    settings.beginGroup(u"ShapeText"_s);
    for_each_field(txtData, [&settings](auto& field, std::string_view name) { // TODO for_each_field_name
                                                                              // if constexpr(requires { field.family(); })
                                                                              // settings.setValue(get_name<index, ShapeData>(), field.toString());
                                                                              // else
        settings.setValue(name, field);
    });
}

Shape::ShapeData Shape::loadData() {
    QSettings settings;
    settings.beginGroup(u"ShapeText"_s);
    for_each_field(txtData, [&settings]<typename Ty>(Ty& field, std::string_view name) { // TODO for_each_field_name
                                                                                         // if constexpr(requires { field.family(); })
                                                                                         // field.fromString(settings.value(name).toString());
                                                                                         // else
        field = settings.value(name).template value<Ty>();
    });
    return txtData;
}

void Shape::save() { iDataCopy = txtData; }

void Shape::restore() {
    txtData = std::move(iDataCopy);
    AbstractShape::redraw();
}

void Shape::ok() { saveData(); }

QString Shape::name() const { return QObject::tr("Text"); }

QIcon Shape::icon() const { return QIcon::fromTheme(u"draw-text"_s); }

} // namespace ShTxt

#include "moc_shape.cpp"
