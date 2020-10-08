// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gerberitem.h"

#include "gbrfile.h"
#include "graphicsview.h"
#include "scene.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

GerberItem::GerberItem(Paths& paths, Gerber::File* file)
    : GraphicsItem(file)
    , m_paths(paths)
{
    for (Path path : m_paths) {
        if (path.size())
            path.append(path.first());
        m_shape.addPolygon(toQPolygon(path));
    }
    fillPolygon = m_shape.toFillPolygon();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);

    connect(this, &GerberItem::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
}

GerberItem::~GerberItem() { }

QRectF GerberItem::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath GerberItem::shape() const { return m_shape; }

void GerberItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (App::scene()->drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    if (m_bodyColor.alpha()) {
        painter->setBrush(m_bodyColor);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(fillPolygon);
    }
    if (m_pathColor.alpha()) {
        m_pen.setColor(m_pathColor);
        painter->strokePath(m_shape, m_pen);
    }
}

int GerberItem::type() const { return GiGerber; }

void GerberItem::redraw()
{
    m_shape = QPainterPath();
    for (Path path : m_paths) {
        path.append(path.first());
        m_shape.addPolygon(toQPolygon(path));
    }
    fillPolygon = m_shape.toFillPolygon();
    setPos({ 1, 1 }); // костыли
    setPos({ 0, 0 });
    //update();
}

Paths GerberItem::paths() const { return m_paths; }

Paths* GerberItem::rPaths() { return &m_paths; }

void GerberItem::changeColor()
{
    {
        auto animation = new QPropertyAnimation(this, "bodyColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_bodyColor);
        m_bodyColor = m_colorPtr ? *m_colorPtr : m_color;
        if (colorState & Selected) {
            m_bodyColor.setAlpha(255);
            m_bodyColor = (colorState & Hovered) ? m_bodyColor.lighter(150)
                                                 : m_bodyColor;
        } else {
            m_bodyColor = (colorState & Hovered) ? (m_bodyColor.setAlpha(255), m_bodyColor.darker(125))
                                                 : m_bodyColor;
        }
        animation->setEndValue(m_bodyColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    {
        auto animation = new QPropertyAnimation(this, "pathColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_pathColor);
        m_pathColor = m_colorPtr ? *m_colorPtr : m_color;
        switch (colorState) {
        case Default:
            m_pathColor.setAlpha(0);
            break;
        case Hovered:
            m_pathColor.setAlpha(255);
            m_pathColor = m_pathColor.darker(125);
            break;
        case Selected:
            break;
        case Hovered | Selected:
            m_pathColor.setAlpha(255);
            m_pathColor = m_pathColor.lighter(150);
            break;
        }
        animation->setEndValue(m_pathColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
