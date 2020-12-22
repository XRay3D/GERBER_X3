#include "dxf_lwpolyline.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QtMath>

//QPainterPath construct_path(const Polygon_2& pgn)
//{
//    QPainterPath result;
//    auto toQRectF = [](const CGAL::Bbox_2& bb) -> QRectF {
//        return QRectF(
//            bb.xmin(),
//            bb.ymin(),
//            bb.xmax() - bb.xmin(),
//            bb.ymax() - bb.ymin());
//    };
//    auto r = toQRectF(pgn.bbox());
//    if (r.width() * r.height() < 0.001)
//        return result;
//    if constexpr (0) {
//        auto current = pgn.curves_begin();
//        auto end = pgn.curves_end();
//        result.moveTo(QPointF(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y())));
//        do {
//            const auto& curve = *current;
//            const QPointF target = QPointF(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
//            if (curve.is_linear()) {
//                result.lineTo(target);
//            } else if (curve.is_circular()) {
//                const bool isClockwise = (curve.supporting_circle().orientation() == CGAL::CLOCKWISE);
//                const QRectF rect = toQRectF(curve.supporting_circle().bbox());
//                const QPointF center = QPointF(CGAL::to_double(curve.supporting_circle().center().x()), CGAL::to_double(curve.supporting_circle().center().y()));
//                const QPointF source = QPointF(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
//                const QPointF p1 = source - center;
//                const QPointF p2 = target - center;
//                const double asource = qAtan2(p1.y(), p1.x());
//                const double atarget = qAtan2(p2.y(), p2.x());
//                double aspan = atarget - asource;
//                if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
//                    aspan += 2.0 * CGAL_PI;
//                else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
//                    aspan -= 2.0 * CGAL_PI;
//                result.arcTo(rect.normalized(), qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
//            }
//        } while (++current != end);
//    } else {
//        Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);
//        // Degenerate polygon, ring.size() < 3
//        if (pgn.orientation() == CGAL::ZERO) {
//            qWarning() << "construct_path: Ignoring degenerated polygon";
//            return result;
//        }
//        const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;
//        auto current = pgn.curves_begin();
//        auto end = pgn.curves_end();
//        result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));
//        do {
//            const auto& curve = *current;
//            const auto& source = curve.source();
//            const auto& target = curve.target();
//            if (curve.is_linear()) {
//                result.lineTo(QPointF(CGAL::to_double(target.x()), CGAL::to_double(target.y())));
//            } else if (curve.is_circular()) {
//                const QRectF rect(toQRectF(curve.supporting_circle().bbox()));
//                const auto center = curve.supporting_circle().center();
//                const double asource = qAtan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
//                const double atarget = qAtan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
//                double aspan = atarget - asource;
//                if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
//                    aspan += 2.0 * CGAL_PI;
//                else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
//                    aspan -= 2.0 * CGAL_PI;
//                result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
//            } else { // ?!?
//                Q_UNREACHABLE();
//            }
//        } while (++current != end);
//        result.lineTo(CGAL::to_double(pgn.curves_begin()->source().x()), CGAL::to_double(pgn.curves_begin()->source().y()));
//    }
//    return result;
//}

namespace Dxf {
class ArcItem final : public ::QGraphicsItem {
    QPainterPath path;
    const LWPOLYLINE* lwpl;
    QColor color;

    double radius;

    void addSeg(LWPOLYLINE::Segment source, LWPOLYLINE::Segment target)
    {
        if (path.isEmpty())
            path.moveTo(source);

        if (qFuzzyIsNull(source.bulge)) {
            path.lineTo(target);
            return;
        }

        const QLineF l1(source, target);
        const double lenght = l1.length() * 0.5;
        const double height = lenght * source.bulge;
        qDebug() << "bulge" << source.bulge;
        radius = (height * height + lenght * lenght) / (height * 2);
        QLineF l2((source + target) / 2, target);
        l2 = l2.normalVector();
        l2.setLength(height);

        QPointF c(l2.p2());
        QPointF center;
        {
            double ax2 = source.x() * source.x();
            double bx2 = target.x() * target.x();
            double cx2 = c.x() * c.x();

            double ay2 = source.y() * source.y();
            double by2 = target.y() * target.y();
            double cy2 = c.y() * c.y();

            double d = source.x() * target.y() + source.y() * c.x() - target.y() * c.x() - source.x() * c.y() - target.x() * source.y() + target.x() * c.y();

            center = QPointF(
                +0.5 / d * ( //
                    source.y() * cx2 + source.y() * cy2 + target.y() * ax2 + target.y() * ay2 + c.y() * bx2 + c.y() * by2 - //
                    source.y() * bx2 - source.y() * by2 - target.y() * cx2 - target.y() * cy2 - c.y() * ax2 - c.y() * ay2),
                -0.5 / d * ( //
                    source.x() * cx2 + source.x() * cy2 + target.x() * ax2 + target.x() * ay2 + c.x() * bx2 + c.x() * by2 - //
                    source.x() * bx2 - source.x() * by2 - target.x() * cx2 - target.x() * cy2 - c.x() * ax2 - c.x() * ay2));
        }

        QRectF br(center + QPointF(radius, radius), center - QPointF(radius, radius));

        //        aAngle = angle(center, source);
        //        bAngle = angle(center, target);
        const double aSource = qRadiansToDegrees(qAtan2(center.y() - source.y(), center.x() - source.x()));
        const double aTarget = qRadiansToDegrees(qAtan2(center.y() - target.y(), center.x() - target.x()));
        double aSpan = aTarget - aSource;

        if (aSpan < -180 || (qFuzzyCompare(aSpan, -180) && !(aTarget > aSource)))
            aSpan += 360;
        else if (aSpan > 180 || (qFuzzyCompare(aSpan, 180) && (aTarget > aSource)))
            aSpan -= 360;

        if (lwpl->polylineFlag == LWPOLYLINE::Closed) {
            path.arcTo(br, -aSource, -aSpan);
            path.arcTo(br, -aTarget, -aSpan);
        } else {
            qDebug() << "\tsize" << lwpl->poly.size();
            qDebug() << "1" << aSource << "2" << aTarget;
#ifdef QT_DEBUG
            path.lineTo(target);
#else
            path.arcTo(br, -aSource, -aSpan);
#endif
        }
    }

public:
    ArcItem(const LWPOLYLINE* lwpl, const QColor& color)
        : lwpl(lwpl)
        , color(color)
    {
        for (int i = 0; i < lwpl->poly.size() - 1; ++i) {
            addSeg(lwpl->poly[i], lwpl->poly[i + 1]);
        }
    }

    ~ArcItem() = default;

    // QGraphicsItem interface
    QRectF boundingRect() const override { return path.boundingRect(); }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override
    {
        painter->setPen(QPen(color, lwpl->constantWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
    }
};

LWPOLYLINE::LWPOLYLINE(SectionParser* sp)
    : Entity(sp)
{
}

void LWPOLYLINE::draw(const INSERT_ET* const i) const
{

    auto path = [this](const QVector<Segment>& vertex) -> Path {
        Path points;
        QPainterPath path;
        auto addSeg = [&path, this](const Segment& source, const Segment& target) {
            if (path.isEmpty())
                path.moveTo(source);

            if (source.bulge == 0.0) {
                path.lineTo(target);
                return;
            }

            auto [center, start_angle_, end_angle_, radius] = bulgeToArc(source, target, source.bulge);

            const double start_angle = qRadiansToDegrees(qAtan2(center.y() - source.y(), center.x() - source.x()));
            const double end_angle = qRadiansToDegrees(qAtan2(center.y() - target.y(), center.x() - target.x()));
            double span = end_angle - start_angle;
            if (span < -180 || (qFuzzyCompare(span, -180) && !(end_angle > start_angle)))
                span += 360;
            else if (span > 180 || (qFuzzyCompare(span, 180) && (end_angle > start_angle)))
                span -= 360;

            const QPointF pr(radius, radius);
            const QRectF br(center + pr, center - pr);
            if (polylineFlag == LWPOLYLINE::Closed) {
                path.arcTo(br, -start_angle, -span);
                path.arcTo(br, -end_angle, -span);
            } else {
                path.arcTo(br, -start_angle, -span);
            }
        };

        for (int i = 0; i < vertex.size() - 1; ++i) {
            addSeg(vertex[i], vertex[i + 1]);
        }
        //        if (polylineFlag == Closed) {
        //            addSeg(vertex.last(), vertex.first());
        //        }

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
                GraphicObject go(sp->file, this, path(poly), {});
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        GraphicObject go(sp->file, this, path(poly), {});
        attachToLayer(std::move(go));
    }
}

void LWPOLYLINE::parse(CodeData& code)
{
    Segment pt;
    do {
        data << code;
        switch (static_cast<VarType>(code.code())) {
        case SubclassMrker:
            break;
        case NumberOfVertices:
            numberOfVertices = code;
            break;
        case PolylineFlag:
            polylineFlag = code;
            break;
        case ConstantWidth:
            constantWidth = code;
            break;
        case Elevation:
            elevation = code;
            break;
        case Thickness:
            thickness = code;
            break;
        case VertexCoordinatesX:
            pt.rx() = code;
            break;
        case VertexCoordinatesY:
            pt.ry() = code;
            poly << pt;
            break;
        case VertexID:
            break;
        case StartWidth:
            startWidth = code;
            break;
        case EndWidth:
            endWidth = code;
            break;
        case Bulge: // betven points
            poly.last().bulge = code;
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}
}
