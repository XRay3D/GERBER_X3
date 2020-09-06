// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shtext.h"
#include "shhandler.h"
#include "shtextdialog.h"
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "scene.h"

namespace Shapes {

Text::Text(QPointF pt1)
    : d(d_)
    , fileName(qApp->applicationDirPath() + "/XrSoft/Text.dat")
{
    m_paths.resize(1);
    handlers = { new Handler(this, Handler::Center) };
    handlers.first()->setPos(pt1);

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> d_;
    } else {
        qWarning("Couldn't open Text.dat file.");
    }
    d = d_;
    redraw();

    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(handlers.first());
}

void Text::redraw()
{
    QPainterPath painterPath;

    QFont font;
    font.fromString(d.font);
    font.setPixelSize(1000);

    painterPath.addText(QPointF(), font, d.text);
    auto bRect = painterPath.boundingRect();

    QFontMetrics fm(font);
    const auto capHeight = fm.capHeight();
    const auto scale = d.height / capHeight;

    QPointF handlePt;

    switch (d.handleAlign) {
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
    if (d.side == Bottom) {
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
    matrix.rotate(d.angle - 360);

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

QPointF Text::calcPos(Handler* sh) { return sh->pos(); }

QString Text::text() const { return d.text; }

void Text::setText(const QString& value)
{
    d.text = value;
    redraw();
}

Side Text::side() const { return d.side; }

void Text::setSide(const Side& side)
{
    d.side = side;
    redraw();
}

void Text::write(QDataStream& stream) const { stream << d; }

void Text::read(QDataStream& stream) { stream >> d; }

void Text::save() { dCopy = d; }

void Text::restore()
{
    d = std::move(dCopy);
    redraw();
}

void Text::ok()
{
    d_ = d;
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        d_.text = QObject::tr("Text");
        out << d_;
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

void Text::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_brColorPtr)
        m_brush.setColor(*m_brColorPtr);
    QColor color(m_pen.color());
    if (option->state & QStyle::State_Selected)
        color = Qt::green;
    if (option->state & QStyle::State_MouseOver)
        color = Qt::red;
    color.setAlpha(50);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(m_shape);
    Shape::paint(painter, option, widget);
}

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
}
