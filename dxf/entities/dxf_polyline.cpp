#include "dxf_polyline.h"
#include "dxf_insert.h"
#include "dxffile.h"

namespace Dxf {

POLYLINE::POLYLINE(SectionParser* sp)
    : Entity(sp)
{
}

void POLYLINE::draw(const INSERT_ET* const i) const
{
    auto path = [this](const QVector<VERTEX_>& vertex) -> Path {
        Path points;
        QPainterPath path;
        auto addSeg = [&path](const VERTEX_& source, const VERTEX_& target) {
            if (path.isEmpty())
                path.moveTo(source);

            if (source.bulge == 0.0) {
                path.lineTo(target);
                return;
            }

            auto [center, start_angle_, end_angle_, radius] = bulgeToArc(source, target, source.bulge);

            const double start_angle = qRadiansToDegrees(qAtan2(center.y() - source.y, center.x() - source.x));
            const double end_angle = qRadiansToDegrees(qAtan2(center.y() - target.y, center.x() - target.x));
            double span = end_angle - start_angle;
            if (span < -180 || (qFuzzyCompare(span, -180) && !(end_angle > start_angle)))
                span += 360;
            else if (span > 180 || (qFuzzyCompare(span, 180) && (end_angle > start_angle)))
                span -= 360;

            QPointF pr(radius, radius);
            path.arcTo(QRectF(center + pr, center - pr),
                -start_angle,
                -span);
        };

        for (int i = 0; i < vertex.size() - 1; ++i) {
            addSeg(vertex[i], vertex[i + 1]);
        }
        if (polylineFlags & ClosedPolyline) {
            addSeg(vertex.last(), vertex.first());
        }

        QMatrix m;
        m.scale(1000, 1000);

        QPainterPath path2;
        for (auto& poly : path.toSubpathPolygons(m))
            path2.addPolygon(poly);

        QMatrix m2;
        m2.scale(0.001, 0.001);
        auto p(path2.toSubpathPolygons(m2));

        return p.value(0);
    };
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(sp->file, this, path(vertex), {});
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        GraphicObject go(sp->file, this, path(vertex), {});
        attachToLayer(std::move(go));
    }
}

void POLYLINE::parse(CodeData& code)
{
    do {
        if (code != "VERTEX") {
            data << (code = sp->nextCode());
            switch (code.code()) {
            case StartWidth:
                startWidth = code;
                break;
            case EndWidth:
                endWidth = code;
                break;
            case PolylineFlag:
                polylineFlags = code;
                break;
            default:
                parseEntity(code);
            }
        } else {
            vertex.append(Dxf::VERTEX_(sp));
            vertex.last().parse(code);
        }
    } while (code != "SEQEND");
    data << code;
    do {
        data << (code = sp->nextCode());
        parseEntity(code);
    } while (code.code() != 0);

    qDebug() << data;
}

QPolygonF POLYLINE::poly(const QVector<VERTEX_>& vertex) const
{
    QPolygonF p;
    p.reserve(vertex.size());
    for (auto& v : vertex)
        p.append({ v.x, v.y });
    return p;
}
}
