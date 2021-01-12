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
#include "shtext.h"
#include "scene.h"
#include "shhandler.h"
#include "shtextdialog.h"
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <interfaces/file.h>

#include "leakdetector.h"

namespace Shapes {

Text::Text(QPointF pt1)
    : data(lastUsed)
    , fileName(qApp->applicationDirPath() + "/XrSoft/Text.dat")
{
    m_paths.resize(1);
    handlers = { new Handler(this, Handler::Center) };
    handlers.first()->setPos(pt1);

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> lastUsed;
    } else {
        qWarning("Couldn't open Text.dat file.");
    }
    data = lastUsed;
    redraw();

    App::scene()->addItem(this);
}

int Text::type() const { return static_cast<int>(GiType::ShText); }

void Text::redraw()
{
    QPainterPath painterPath;

    QFont font;
    font.fromString(data.font);
    font.setPixelSize(1000);

    painterPath.addText(QPointF(), font, data.text);
    auto bRect = painterPath.boundingRect();

    QFontMetrics fm(font);
    const auto capHeight = fm.capHeight();
    const auto scale = data.height / capHeight;

    QPointF handlePt;

    switch (data.handleAlign) {
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
    if (data.side == Bottom) {
        matrix.translate((bRect.right() + bRect.left()) * scale, 0);
        matrix.scale(-scale, -scale);
    } else
        matrix.scale(scale, -scale);
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
    matrix.translate(handlers.first()->pos().x(), handlers.first()->pos().y());
    matrix.rotate(data.angle - 360);

    m_paths.clear();
    m_shape = QPainterPath();

    for (auto& sp : painterPath.toSubpathPolygons(matrix)) {
        m_paths.append(sp);
        m_shape.addPolygon(sp);
    }
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Text::text() const { return data.text; }

void Text::setText(const QString& value)
{
    data.text = value;
    redraw();
}

Side Text::side() const { return data.side; }

void Text::setSide(const Side& side)
{
    data.side = side;
    redraw();
}

void Text::write(QDataStream& stream) const { stream << data; }

void Text::read(QDataStream& stream) { stream >> data; }

void Text::save() { dataCopy = data; }

void Text::restore()
{
    data = std::move(dataCopy);
    redraw();
}

void Text::ok()
{
    lastUsed = data;
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        lastUsed.text = QObject::tr("Text");
        out << lastUsed;
    } else {
        qWarning("Couldn't open Text.dat file.");
    }
}

void Text::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    ShTextDialog dlg({ this }, nullptr);
    dlg.exec();
    redraw();
}

QPainterPath Text::shape() const { return m_shape; }

QString Text::name() const { return QObject::tr("Text"); }

QIcon Text::icon() const { return QIcon::fromTheme("draw-text"); }

QDataStream& operator<<(QDataStream& stream, const Text::InternalData& d)
{
    stream << d.text;
    stream << d.font;
    stream << d.angle;
    stream << d.height;
    stream << d.handleAlign;
    stream << d.side;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Text::InternalData& d)
{
    stream >> d.text;
    stream >> d.font;
    stream >> d.angle;
    stream >> d.height;
    stream >> d.handleAlign;
    stream >> d.side;
    return stream;
}

////////////////////////////////////////////////////////////
/// \brief PluginText::PluginText
///
PluginText::PluginText() { }

PluginText::~PluginText() { }

QObject* PluginText::getObject() { return this; }

int PluginText::type() const { return static_cast<int>(GiType::ShText); }

void PluginText::setupInterface(App* a) { app.set(a); }

QJsonObject PluginText::info() const
{
    return QJsonObject {
        { "Name", "Text" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Text" }
    };
}

QIcon PluginText::icon() const { return QIcon::fromTheme("draw-text"); }

Shape* PluginText::createShape() { return new Text(); }

Shape* PluginText::createShape(const QPointF& point)
{
    QTimer::singleShot(100, [this] { emit actionUncheck(); });
    return new Text(point);
}

bool PluginText::addShapePoint(const QPointF&) { return false; }

void PluginText::updateShape(const QPointF&) { }

void PluginText::finalizeShape()
{
    shape = nullptr;
    emit actionUncheck();
}

}
