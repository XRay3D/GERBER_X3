// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "graphicsitem.h"
#include "itemgroup.h"

GraphicsItem::GraphicsItem(AbstractFile* file)
    : m_file(file)
    , m_pen(QPen(Qt::white, 0.0))
    , m_brush(Qt::white)
{
}

QBrush GraphicsItem::brush() const { return m_brush; }

QPen GraphicsItem::pen() const { return m_pen; }

void GraphicsItem::setBrush(const QBrush& brush) { m_brush = brush; }

void GraphicsItem::setPen(const QPen& pen) { m_pen = pen; }

//void GraphicsItem::setItemGroup(ItemGroup* itemGroup) { m_ig = itemGroup; }

//ItemGroup* GraphicsItem::parentItemGroup() const { return m_ig; }

//QPointF GraphicsItem::center() const { return m_rect.center() + pos(); }

void GraphicsItem::setPenColor(QColor& penColor) { m_pnColorPrt = &penColor; }

void GraphicsItem::setBrushColor(QColor& brushColor) { m_brColorPtr = &brushColor; }

const AbstractFile* GraphicsItem::file() const { return m_file; }

int GraphicsItem::id() const { return m_id; }
