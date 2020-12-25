#include "dxf_hatch.h"
#include "dxf_file.h"
#include <QGraphicsPolygonItem>

#include <QPolygonF>

namespace Dxf {
Hatch::Hatch(SectionParser* sp)
    : Entity(sp)
{
}

void Hatch::draw(const InsertEntity* const /*i*/) const
{
    //    QPainterPath path;
    //    QPolygonF poly;
    //    QPointF pt;
    //    double width = 0;
    //    for (auto& d : data) {
    //        if (d.code() == 10) {
    //            pt.rx() = d;
    //        } else if (d.code() == 20) {
    //            pt.ry() = d;
    //            if (!pt.isNull())
    //                poly.append(pt);
    //        } else if (d.code() == 11) {
    //            pt.rx() = d;
    //        } else if (d.code() == 21) {
    //            pt.ry() = d;
    //            if (!pt.isNull())
    //                poly.append(pt);
    //        } else if (d.code() == 40) {
    //            width = d;
    //        } else if (d.code() == 92) {
    //            path.addPolygon(poly);
    //            poly.clear();
    //        }
    //    }
    //    path.addPolygon(poly);

    //    if (i) {
    //        Q_UNUSED(i)
    //    } else {
    //        attachToLayer(scene->addPath(path, QPen(color(), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), color()));
    //    }
}

void Hatch::parse(CodeData& code)
{
    do {
        data.push_back(code);
        code = sp->nextCode();
        parseEntity(code);
    } while (code.code() != 0);
}

GraphicObject Hatch::toGo() const {  return { sp->file, this, {}, {} }; }

}
