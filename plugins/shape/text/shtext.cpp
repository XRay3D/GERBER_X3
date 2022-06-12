// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shtext.h"

#include "file.h"
#include "scene.h"
#include "shhandler.h"
#include "shnode.h"
#include "shtextdialog.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

namespace Shapes {

Text::Text(QPointF pt1)
    : iData(loadIData()) {
    m_paths.resize(1);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));

    handlers.front()->setPos(pt1);

    redraw();

    App::scene()->addItem(this);
}

int Text::type() const { return static_cast<int>(GiType::ShText); }

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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QMatrix matrix;
#else
    QTransform matrix;
#endif
    matrix.translate(-bRect.left() * scale, 0);
    matrix.translate(handlePt.x() * scale, handlePt.y() * scale);
    if (iData.side == Bottom) {
        matrix.translate((bRect.right() + bRect.left()) * scale, 0);
        //        matrix.scale(
        //            -scale * iData.xy > 0.0 ? 1 * iData.xy : 1,
        //            -scale * iData.xy < 0.0 ? 1 / iData.xy : 1);
        matrix.scale(-scale * (100 / iData.xy), -scale);
    } else {
        // matrix.scale(+scale * xyScale, -scale);
        matrix.scale(+scale * (100 / iData.xy), -scale);
    }
    {
        QPainterPath tmpPainterPath;
        for (auto& polygon : painterPath.toSubpathPolygons()) { // text to polygons
            tmpPainterPath.addPolygon(polygon);
        }
        painterPath = std::move(tmpPainterPath);
        tmpPainterPath = QPainterPath();
        for (auto& polygon : painterPath.toSubpathPolygons(matrix)) { // transform polygons with matrix
            tmpPainterPath.addPolygon(polygon);
        }
        painterPath = std::move(tmpPainterPath);
    }
    matrix.reset();
    matrix.translate(handlers.front()->pos().x(), handlers.front()->pos().y());
    matrix.rotate(iData.angle - 360);

    m_paths.clear();
    m_shape = {};

    Clipper clipper;
    for (auto& sp : painterPath.toSubpathPolygons(matrix)) {
        clipper.AddPath(sp, ClipperLib::ptClip);
        //        m_paths.push_back(sp);
        //        m_shape.addPolygon(sp);
    }
    clipper.Execute(ClipperLib::ctUnion, m_paths, ClipperLib::pftNonZero);
    for (auto& sp : m_paths) {
        sp.emplace_back(sp.front());
        m_shape.addPolygon(sp);
    }

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
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

bool Text::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            setText(value.toString());
            return true;
        }
        break;
    case FileTree::Column::Side:
        if (role == Qt::EditRole) {
            setSide(static_cast<Side>(value.toBool()));
            return true;
        }
        break;
    default:
        break;
    }
    return Shape::setData(index, value, role);
}

Qt::ItemFlags Text::flags(const QModelIndex& index) const {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return Shape::flags(index) | Qt::ItemIsEditable;
    case FileTree::Column::Side:
        return Shape::flags(index) | Qt::ItemIsEditable;
    default:
        return Shape::flags(index);
    }
}

QVariant Text::data(const QModelIndex& index, int role) const {
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return QString("%1 (%2, %3)")
                .arg(name())
                .arg(m_giId)
                .arg(text());
        case Qt::EditRole:
            return text();
        default:
            return Shape::data(index, role);
        }
    case FileTree::Column::Side:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return m_node->sideStrList[side()];
        case Qt::EditRole:
            return static_cast<bool>(side());
        default:
            return Shape::data(index, role);
        }
    default:
        return Shape::data(index, role);
    }
}

void Text::menu(QMenu& menu, FileTree::View* tv) const {
    Shape::menu(menu, tv);
    menu.addAction(QIcon::fromTheme("draw-text"), QObject::tr("&Edit Text"), [this, tv] {
        ShTextDialog dlg({ const_cast<Text*>(this) }, tv);
        dlg.exec();
    });
}

void Text::write(QDataStream& stream) const { stream << iData; }

void Text::read(QDataStream& stream) { stream >> iData; }

void Text::saveIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    settings.setValue("font", lastUsedIData.font);
    settings.setValue("text", lastUsedIData.text);
    settings.setValue("side", lastUsedIData.side);
    settings.setValue("angle", lastUsedIData.angle);
    settings.setValue("height", lastUsedIData.height);
    settings.setValue("xy", lastUsedIData.xy);
    settings.setValue("handleAlign", lastUsedIData.handleAlign);
}

Text::InternalData Text::loadIData() {
    QSettings settings;
    settings.beginGroup("ShapeText");
    lastUsedIData.font = settings.value("font").toString();
    lastUsedIData.text = settings.value("text", QObject::tr("Text")).toString();
    lastUsedIData.side = static_cast<Side>(settings.value("side", Side::Top).toInt());
    lastUsedIData.angle = settings.value("angle", 0.0).toDouble();
    lastUsedIData.height = settings.value("height", 10.0).toDouble();
    lastUsedIData.xy = settings.value("xy", 10.0).toDouble();
    lastUsedIData.handleAlign = settings.value("handleAlign", BotLeft).toInt();
    return lastUsedIData;
}

void Text::save() { iDataCopy = iData; }

void Text::restore() {
    iData = std::move(iDataCopy);
    redraw();
}

void Text::ok() {
    lastUsedIData = iData;
    saveIData();
}

void Text::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseDoubleClickEvent(event);
    ShTextDialog dlg({ this }, nullptr);
    dlg.exec();
    redraw();
}

QPainterPath Text::shape() const { return m_shape; }

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
PluginText::PluginText() { }

PluginText::~PluginText() { }

int PluginText::type() const { return static_cast<int>(GiType::ShText); }

QIcon PluginText::icon() const { return QIcon::fromTheme("draw-text"); }

Shape* PluginText::createShape() { return new Text(); }

Shape* PluginText::createShape(const QPointF& point) {
    QTimer::singleShot(100, [this] { emit actionUncheck(); });
    return new Text(point);
}

bool PluginText::addShapePoint(const QPointF&) { return false; }

void PluginText::updateShape(const QPointF&) { }

void PluginText::finalizeShape() {
    if (shape)
        shape->finalize();
    shape = nullptr;
    emit actionUncheck();
}
} // namespace Shapes
