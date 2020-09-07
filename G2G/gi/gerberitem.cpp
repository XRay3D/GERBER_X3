// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gerberitem.h"

#include <QElapsedTimer>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "gbrfile.h"
#include "graphicsview.h"
#include "scene.h"

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
}

GerberItem::~GerberItem() { }

QRectF GerberItem::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath GerberItem::shape() const { return m_shape; }

void GerberItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (App::scene()->drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen); //QPen(Qt::black, 0.0));
        painter->drawPath(m_shape);
        return;
    }

    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_brColorPtr)
        m_brush.setColor(*m_brColorPtr);

    if (App::scene()->drawPdf()) {
        painter->setBrush(m_brush.color());
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    QColor brColor(m_brush.color());
    QColor pnColor(brColor);

    if (option->state & QStyle::State_Selected) {
        brColor.setAlpha(255);
        pnColor.setAlpha(255);
    }
    if (option->state & QStyle::State_MouseOver) {
        if (option->state & QStyle::State_Selected) {
            brColor = brColor.darker(120);
            pnColor = pnColor.darker(120);
        } else {
            brColor.setAlpha(200);
            pnColor.setAlpha(255);
        }
    }

    QBrush brush(brColor);
    QPen pen(pnColor, 0.0);

    if constexpr (false) {
        painter->setBrush(brush);
        painter->setPen(m_file ? pen : m_pen);
        painter->drawPath(m_shape);
    } else {
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(fillPolygon /*m_shape.toFillPolygon()*/);
        painter->strokePath(m_shape, m_file ? pen : m_pen);
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
