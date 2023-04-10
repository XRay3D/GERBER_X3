// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shtext.h"

#include "abstract_file.h"
#include "graphicsview.h"
#include "shhandler.h"
#include "shnode.h"
#include "shtextdialog.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

namespace Shapes {

Text::Text(QPointF pt1) {
    loadIData();
    paths_.resize(1);
    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.front()->setPos(pt1);
    redraw();
    App::graphicsView().addItem(this);
}

void Text::redraw() {
    QPainterPath painterPath;

    QFont font;
    font.fromString(iData.font);
    font.setPixelSize(1000);

    painterPath.addText(QPointF(), font, iData.text);
    auto bRect = painterPath.boundingRect();

    QFontMetrics fm(font);
    const auto capHeight = fm.capHeight();
    const auto scale = iData.height / capHeight;
    const auto xyScale = 100.0 / iData.xy;

    QPointF handlePt;

    switch (iData.handleAlign) {
    case BotCenter:
        handlePt -= QPointF(bRect.width() * 0.5, 0);
        break;
    case BotLeft:
        handlePt -= QPointF();
        break;
    case BotRight:
        handlePt -= QPointF(bRect.width(), 0);
        break;
    case Center:
        handlePt -= QPointF(bRect.width() * 0.5, capHeight * 0.5);
        break;
    case CenterLeft:
        handlePt -= QPointF(0, capHeight * 0.5);
        break;
    case CenterRight:
        handlePt -= QPointF(bRect.width(), capHeight * 0.5);
        break;
    case TopCenter:
        handlePt -= QPointF(bRect.width() * 0.5, capHeight);
        break;
    case TopLeft:
        handlePt -= QPointF(0, capHeight);
        break;
    case TopRight:
        handlePt -= QPointF(bRect.width(), capHeight);
        break;
    }

    QTransform transform;
    transform.translate(-bRect.left() * scale, 0);
    transform.translate(handlePt.x() * scale, handlePt.y() * scale);
    if (iData.side == Bottom) {
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
        for (auto& polygon : painterPath.toSubpathPolygons()) { // text to polygons
            tmpPainterPath.addPolygon(polygon);
        }
        painterPath = std::move(tmpPainterPath);
        tmpPainterPath = QPainterPath();
        for (auto& polygon : painterPath.toSubpathPolygons(transform)) { // transform polygons with matrix
            tmpPainterPath.addPolygon(polygon);
        }
        painterPath = std::move(tmpPainterPath);
    }
    transform.reset();
    transform.translate(handlers.front()->pos().x(), handlers.front()->pos().y());
    transform.rotate(iData.angle - 360);

    paths_.clear();
    shape_ = {};

    Clipper clipper;
    for (auto& sp : painterPath.toSubpathPolygons(transform)) {
        clipper.AddClip({sp});
        //        paths_.push_back(sp);
        //        shape_.addPolygon(sp);
    }
    clipper.Execute(ClipType::Union, FillRule::NonZero, paths_);
    for (auto& sp : paths_) {
        sp.emplace_back(sp.front());
        shape_.addPolygon(sp);
    }

    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});
    //    update();
}

QString Text::text() const { return iData.text; }

void Text::setText(const QString& value) {
    iData.text = value;
    redraw();
}

Side Text::side() const { return iData.side; }

void Text::setSide(const Side& side) {
    iData.side = side;
    redraw();
}

void Text::setPt(const QPointF& point) {
    handlers.front()->setPos(point);
    redraw();
}

bool Text::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (FileTree_::Column(index.column())) {
    case FileTree_::Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            setText(value.toString());
            return true;
        }
        break;
    case FileTree_::Column::Side:
        if (role == Qt::EditRole) {
            setSide(static_cast<Side>(value.toBool()));
            return true;
        }
        break;
    default:
        break;
    }
    return AbstractShape::setData(index, value, role);
}

Qt::ItemFlags Text::flags(const QModelIndex& index) const {
    switch (FileTree_::Column(index.column())) {
    case FileTree_::Column::NameColorVisible:
        return AbstractShape::flags(index) | Qt::ItemIsEditable;
    case FileTree_::Column::Side:
        return AbstractShape::flags(index) | Qt::ItemIsEditable;
    default:
        return AbstractShape::flags(index);
    }
}

QVariant Text::data(const QModelIndex& index, int role) const {
    switch (FileTree_::Column(index.column())) {
    case FileTree_::Column::NameColorVisible:
        switch (role) {
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
    case FileTree_::Column::Side:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return node_->sideStrList[side()];
        case Qt::EditRole:
            return static_cast<bool>(side());
        default:
            return AbstractShape::data(index, role);
        }
    default:
        return AbstractShape::data(index, role);
    }
}

void Text::menu(QMenu& menu, FileTree_::View* tv) const {
    AbstractShape::menu(menu, tv);
    menu.addAction(QIcon::fromTheme("draw-text"), QObject::tr("&Edit Text"), [this, tv] {
        ShTextDialog dlg({const_cast<Text*>(this)}, tv);
        dlg.exec();
    });
}

void Text::write(QDataStream& stream) const { stream << iData; }

void Text::read(QDataStream& stream) { stream >> iData; }

void Text::saveIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    settings.setValue("font", iData.font);
    settings.setValue("text", iData.text);
    settings.setValue("side", iData.side);
    settings.setValue("angle", iData.angle);
    settings.setValue("height", iData.height);
    settings.setValue("xy", iData.xy);
    settings.setValue("handleAlign", iData.handleAlign);
}

Text::InternalData Text::loadIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    iData.font = settings.value("font").toString();
    iData.text = settings.value("text", QObject::tr("Text")).toString();
    iData.side = static_cast<Side>(settings.value("side", Side::Top).toInt());
    iData.angle = settings.value("angle", 0.0).toDouble();
    iData.height = settings.value("height", 10.0).toDouble();
    iData.xy = settings.value("xy", 10.0).toDouble();
    iData.handleAlign = settings.value("handleAlign", BotLeft).toInt();
    return iData;
}

void Text::save() { iDataCopy = iData; }

void Text::restore() {
    iData = std::move(iDataCopy);
    redraw();
}

void Text::ok() { saveIData(); }

void Text::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    ShTextDialog dlg({this}, nullptr);
    dlg.exec();
    redraw();
}

QPainterPath Text::shape() const { return shape_; }

QString Text::name() const { return QObject::tr("Text"); }

QIcon Text::icon() const { return QIcon::fromTheme("draw-text"); }

QDataStream& operator<<(QDataStream& stream, const Text::InternalData& d) {
    stream << d.text;
    stream << d.font;
    stream << d.angle;
    stream << d.height;
    stream << d.xy;
    stream << d.handleAlign;
    stream << d.side;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Text::InternalData& d) {
    stream >> d.text;
    stream >> d.font;
    stream >> d.angle;
    stream >> d.height;
    stream >> d.xy;
    stream >> d.handleAlign;
    stream >> d.side;
    return stream;
}

////////////////////////////////////////////////////////////
/// \brief PluginText::PluginText
///

int PluginImpl::type() const { return GiType::ShText; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-text"); }

AbstractShape* PluginImpl::createShape(const QPointF& point) const { return new Text(point); }

} // namespace Shapes

#include "moc_shtext.cpp"
