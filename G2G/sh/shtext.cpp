// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shtext.h"
#include "shandler.h"
#include "shtextdialog.h"
#include <mainwindow.h>
#include <scene.h>

namespace Shapes {

Text::Text(QPointF pt1)
    : m_text("Text")
    , font(font_)
    , m_side(side_)
    , angle(angle_)
    , height(height_)
    , handleAlign(handleAlign_)
{
    m_paths.resize(1);
    sh = { new Handler(this, true) };
    sh.first()->setPos(pt1);

    if (font_.isEmpty()) {
        QSettings s;
        s.beginGroup("Shapes::Text");
        font = font_ = s.value("font").toString();
        m_side = side_ = static_cast<Side>(s.value("side").toInt());
        angle = angle_ = s.value("angle").toDouble();
        height = height_ = s.value("height").toDouble();
        handleAlign = handleAlign_ = s.value("handleAlign").toInt();
    }

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh.first());
}

Text::~Text()
{
    QSettings s;
    s.beginGroup("Shapes::Text");
    s.setValue("font", font_);
    s.setValue("side", static_cast<int>(side_));
    s.setValue("angle", angle_);
    s.setValue("height", height_);
    s.setValue("handleAlign", handleAlign_);
}

void Text::redraw()
{
    QPainterPath painterPath;

    QFont font;
    font.fromString(this->font);
    font.setPointSize(100);

    painterPath.addText(QPointF(), font, m_text);
    auto bRect = painterPath.boundingRect();

    QFontMetrics fm(font);
    const auto capHeight = fm.capHeight();
    const auto scale = height / capHeight;

    QPointF handlePt;

    switch (handleAlign) {
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

    QMatrix matrix;
    matrix.translate(-bRect.left() * scale, 0);
    matrix.translate(handlePt.x() * scale, handlePt.y() * scale);
    if (m_side == Bottom) {
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
    matrix.translate(sh.first()->pos().x(), sh.first()->pos().y());
    matrix.rotate(angle - 360);

    m_paths.clear();
    m_shape = QPainterPath();

    for (auto& sp : painterPath.toSubpathPolygons(matrix)) {
        m_paths.append(toPath(sp));
        m_shape.addPolygon(sp);
    }
    m_scale = std::numeric_limits<double>::max();

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF Text::calcPos(Handler* sh) const { return sh->pos(); }

void Text::write(QDataStream& stream) const
{
    stream << m_text;
    stream << font;
    stream << angle;
    stream << height;
    stream << handleAlign;
    stream << m_side;
}

void Text::read(QDataStream& stream)
{
    stream >> m_text;
    stream >> font;
    stream >> angle;
    stream >> height;
    stream >> handleAlign;
    stream >> m_side;
}

void Text::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    ShTextDialog dlg({ this }, App::mainWindow());
    dlg.exec();
    redraw();
}

QPainterPath Text::shape() const { return m_shape; }
}
