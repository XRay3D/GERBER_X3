#include "pline.h"
#include "shandler.h"
#include <scene.h>

namespace Shapes {
PolyLine::PolyLine(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    sh = { new Handler(this /*, true*/), new Handler(this) };
    sh.first()->setPos(pt1);
    sh.last()->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh.first());
    App::scene()->addItem(sh.last());
}

PolyLine::PolyLine(QDataStream& stream)
    : Shape(stream)
{
}

PolyLine::~PolyLine() { qDebug(Q_FUNC_INFO); }

void PolyLine::redraw()
{
    Path& path = m_paths.first();
    path.resize(sh.size());
    for (int i = 0, e = sh.size(); i < e; ++i) {
        path[i] = toIntPoint(sh[i]->pos());
    }
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF PolyLine::calcPos(Handler* sh) const { return sh->pos(); }

void PolyLine::setPt(const QPointF& pt)
{
    if (sh.last()->pos() == pt)
        return;
    sh.last()->setPos(pt);
    redraw();
}

void PolyLine::addPt(const QPointF& pt)
{
    sh.append(new Handler(this));
    sh.last()->setPos(pt);
    App::scene()->addItem(sh.last());
    redraw();
}

bool PolyLine::closed() { return sh.first()->pos() == sh.last()->pos(); }
}
