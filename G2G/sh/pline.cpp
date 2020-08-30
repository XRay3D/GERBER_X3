#include "pline.h"
#include "shandler.h"
#include <scene.h>

namespace Shapes {
PolyLine::PolyLine(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    sh = { new Handler(this, Handler::Center), new Handler(this), new Handler(this, Handler::Adder), new Handler(this) };
    sh[1]->setPos(pt1);
    sh[3]->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(-std::numeric_limits<double>::max());
    App::scene()->addItem(this);
    for (auto var : sh) {
        scene()->addItem(var);
    }
}

PolyLine::~PolyLine() { qDebug(Q_FUNC_INFO); }

void PolyLine::redraw()
{
    Path& path = m_paths.first();
    path.clear();
    for (int i = 1, e = sh.size(); i < e; ++i) {
        if (sh[i]->getHType() == Handler::Corner)
            path.append(toIntPoint(sh[i]->pos()));
    }
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();
    sh[0]->QGraphicsItem::setPos(m_shape.boundingRect().center());
    m_scale = std::numeric_limits<double>::max();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF PolyLine::calcPos(Handler* handler)
{
    if (handler->getHType() == Handler::Adder) {
        int idx = sh.indexOf(handler);
        Handler* h;
        {
            Handler* h1 = sh[idx + 1];
            sh.insert(idx + 1, h = new Handler(this, Handler::Adder));
            h->QGraphicsItem::setPos(
                QLineF(handler->pos(), h1->pos()).center());
            scene()->addItem(h);
        }
        {
            Handler* h1 = sh[idx];
            sh.insert(idx, h = new Handler(this, Handler::Adder));
            h->QGraphicsItem::setPos(
                QLineF(handler->pos(), h1->pos()).center());
            scene()->addItem(h);
        }
        handler->setHType(Handler::Corner);
    } else {
        int idx = sh.indexOf(handler);
        if (handler != sh[1]) {
            sh[idx - 1]->QGraphicsItem::setPos(
                QLineF(handler->pos(), sh[idx - 2]->pos()).center());
        }
        if (handler != sh.last()) {
            sh[idx + 1]->QGraphicsItem::setPos(
                QLineF(handler->pos(), sh[idx + 2]->pos()).center());
        }
    }
    return handler->pos();
}

void PolyLine::setPt(const QPointF& pt)
{
    sh[sh.size() - 2]->QGraphicsItem::setPos(QLineF(sh[sh.size() - 3]->pos(), pt).center());
    sh.last()->Handler::setPos(pt);
    redraw();
}

void PolyLine::addPt(const QPointF& pt)
{
    Handler* h1 = sh.last();
    Handler* h2;
    sh.append(h2 = new Handler(this, Handler::Adder));
    sh.last()->QGraphicsItem::setPos(pt);

    sh.append(new Handler(this));
    sh.last()->QGraphicsItem::setPos(pt);
    h2->QGraphicsItem::setPos(QLineF(h1->pos(), pt).center());
    scene()->addItem(h2);
    scene()->addItem(sh.last());
    redraw();
}

bool PolyLine::closed() { return sh[1]->pos() == sh.last()->pos(); }

}
