/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_parser.h"

#include "tables/topor_layer.h"
#include "topor_file.h"

#include "TopoR_PCB_File.h"
#include <lxmlser.hpp>

#include "geo/boolean.h"

#include <QtMath>
#include <concepts>
#include <variant>

namespace TopoR {

namespace {

    double unitScale(dist d) {
        switch(d) {
        case dist::mm : return 1.0;
        case dist::mkm: return 0.001;
        case dist::cm : return 10.0;
        case dist::dm : return 100.0;
        case dist::m  : return 1000.0;
        case dist::mil: return 0.0254;
        case dist::inch: return 25.4;
        }
        return 1.0;
    }

    QPointF pt(const Coordinates::Coord& c, double k) { return {c.x * k, c.y * k}; }

    // Сегменты полилинии (Polyline/Contour/FilledContour): Start + Segments.
    template <typename P>
    Geo::Polyline segmentsToPolyline(const P& p, double k) {
        Geo::Polyline path;
        path.emplace_back(pt(p.Start, k));
        for(auto&& seg: p.Segments) {
            std::visit([&](auto&& s) {
                using T = std::decay_t<decltype(s)>;
                const QPointF cur = path.back();
                const QPointF end = pt(s.End, k);
                if constexpr(std::same_as<T, SegmentLine>) {
                    path.back().bulge = 0.0;
                } else if constexpr(std::same_as<T, SegmentArcCCW> || std::same_as<T, SegmentArcCW>) {
                    const QPointF center = pt(s.Center, k);
                    path.back().bulge = Geo::bulgeOf(cur, end, center,
                        std::same_as<T, SegmentArcCCW> ? Geo::Vertex::Ccw : Geo::Vertex::Cw);
                } else if constexpr(std::same_as<T, SegmentArcByAngle>) {
                    path.back().bulge = Geo::bulgeOf(qDegreesToRadians(s.angle));
                } else if constexpr(std::same_as<T, SegmentArcByMiddle>) {
                    path.back().bulge = Geo::bulgeOf(cur, pt(s.Middle, k), end);
                }
                path.emplace_back(end);
            },
                seg);
        }
        return path;
    }

    // Figure-варианты (BoardOutline.Contour/Voids, Footprint::Copper/Keepout,
    // Constructive::Keepout) -> Geo::Polyline. Не замкнута -- замыкает вызывающий.
    template <typename Variant>
    Geo::Polyline figureToPolyline(const Variant& figure, double k) {
        return std::visit([&](auto&& f) -> Geo::Polyline {
            using T = std::decay_t<decltype(f)>;
            Geo::Polyline path;
            if constexpr(std::same_as<T, Rect> || std::same_as<T, FilledRect>) {
                if(f.Dots.size() >= 2) {
                    const QPointF a = pt(f.Dots[0], k), b = pt(f.Dots[1], k);
                    path = Geo::rectangle(std::abs(b.x() - a.x()), std::abs(b.y() - a.y()), (a + b) * 0.5);
                }
            } else if constexpr(std::same_as<T, Line> || std::same_as<T, Polygon>) {
                for(auto&& d: f.Dots) path.emplace_back(pt(d, k));
            } else if constexpr(std::same_as<T, Circle> || std::same_as<T, FilledCircle>) {
                path = Geo::circle(f.diameter * k, pt(f.Center, k));
            } else if constexpr(std::same_as<T, Polyline> || std::same_as<T, Contour> || std::same_as<T, FilledContour>) {
                path = segmentsToPolyline(f, k);
            } else if constexpr(std::same_as<T, ArcCCW> || std::same_as<T, ArcCW>) {
                path.emplace_back(pt(f.Start, k));
                path.back().bulge = Geo::bulgeOf(pt(f.Start, k), pt(f.End, k), pt(f.Center, k),
                    std::same_as<T, ArcCCW> ? Geo::Vertex::Ccw : Geo::Vertex::Cw);
                path.emplace_back(pt(f.End, k));
            } else if constexpr(std::same_as<T, ArcByAngle>) {
                path.emplace_back(pt(f.Start, k));
                path.back().bulge = Geo::bulgeOf(qDegreesToRadians(f.angle));
                path.emplace_back(pt(f.End, k));
            } else if constexpr(std::same_as<T, ArcByMiddle>) {
                path.emplace_back(pt(f.Start, k));
                path.back().bulge = Geo::bulgeOf(pt(f.Start, k), pt(f.Middle, k), pt(f.End, k));
                path.emplace_back(pt(f.End, k));
            }
            return path;
        },
            figure);
    }

    // Трасса проводника (Wire::Subwire): Start + Tracks (TrackLine/TrackArc/TrackArcCW).
    Geo::Polyline trackToPolyline(const Start& start, const std::vector<std::variant<TrackLine, TrackArc, TrackArcCW>>& tracks, double k) {
        Geo::Polyline path;
        path.emplace_back(pt(start, k));
        for(auto&& track: tracks) {
            std::visit([&](auto&& t) {
                using T = std::decay_t<decltype(t)>;
                const QPointF cur = path.back();
                const QPointF end = pt(t.End, k);
                if constexpr(std::same_as<T, TrackLine>) {
                    path.back().bulge = 0.0;
                } else {
                    const QPointF center = pt(t.Center, k);
                    path.back().bulge = Geo::bulgeOf(cur, end, center,
                        std::same_as<T, TrackArc> ? Geo::Vertex::Ccw : Geo::Vertex::Cw);
                }
                path.emplace_back(end);
            },
                track);
        }
        return path;
    }

    // Контактная площадка (Pad*) -> Geo::Polygons в ЛОКАЛЬНЫХ координатах стека
    // (без учёта положения вывода/компонента -- см. вызывающих).
    Geo::Polygons padShapeToPolygons(const std::variant<LocalLibrary::PadCircle, LocalLibrary::PadOval, LocalLibrary::PadRect, LocalLibrary::PadPoly>& pad, double k) {
        return std::visit([&](auto&& p) -> Geo::Polygons {
            using T = std::decay_t<decltype(p)>;
            if constexpr(std::same_as<T, LocalLibrary::PadCircle>) {
                return Geo::Polygons{Geo::Polylines{Geo::circle(p.diameter * k)}};
            } else if constexpr(std::same_as<T, LocalLibrary::PadOval>) {
                // Капсула между двумя концами Stretch/2, раздутая на diameter --
                // тот же приём, что offsetting.md документирует для штриха.
                const QPointF half = pt(p.Stretch, k) * 0.5;
                Geo::Polyline axis{{-half, 0.0}, {half, 0.0}};
                auto region = Geo::Inflate(Geo::Polylines{axis}, p.diameter * k);
                if(p.Shift) region = Geo::translated(region, pt(p.Shift, k));
                return region;
            } else if constexpr(std::same_as<T, LocalLibrary::PadRect>) {
                const double w = p.width * k, h = p.height * k;
                Geo::Polygons region;
                if(p.handling == Handling::Rounding && p.handlingValue > 0) {
                    const double r = p.handlingValue * k;
                    region = Geo::Inflate(
                        Geo::Polygons{Geo::Polygon{Geo::rectangle(std::max(w - 2 * r, 0.0), std::max(h - 2 * r, 0.0))}},
                        2 * r);
                } else {
                    region = Geo::Polygons{Geo::Polygon{Geo::rectangle(w, h)}};
                }
                if(p.Shift) region = Geo::translated(region, pt(p.Shift, k));
                return region;
            } else if constexpr(std::same_as<T, LocalLibrary::PadPoly>) {
                Geo::Polyline poly;
                for(auto&& d: p.Dots) poly.emplace_back(pt(d, k));
                poly.close();
                return Geo::Polygons{Geo::Polygon{poly}};
            }
            return {};
        },
            pad);
    }

    // Объединение всех форм стека -- см. упрощение в плане: Reference (какому
    // слою принадлежит форма) для v1 не разбирается, площадка одинакова на
    // обеих внешних медных слоях.
    Geo::Polygons padstackToPolygons(const LocalLibrary::Padstack& ps, double k) {
        Geo::Polygons region;
        for(auto&& pad: ps.Pads) region |= padShapeToPolygons(pad, k);
        return region;
    }

    QTransform compTransform(const ComponentsOnBoard::CompInstance& inst) {
        QTransform t = inst.transform();
        if(inst.side == side::Bottom) {
            QTransform mirror;
            mirror.scale(-1, 1);
            t *= mirror;
        }
        return t;
    }

} // namespace

File* Parser::parseFile(const QString& fileName) {
    const std::string path = fileName.toStdString();
    XML::Serializer xml{path};

    TopoR_PCB_File pcb;
    xml >> pcb;

    const double k = unitScale(pcb.Header.Units.dist);

    auto file = new File;

    // --- Классификация слоёв стека: первый Signal = верхняя медь, последний
    // Signal = нижняя медь; всё до первого -- Top-сторона, всё после
    // последнего -- Bottom-сторона (проверено на реальном arz_4L.fst, см. план).
    int firstSignal = -1, lastSignal = -1;
    for(int i{}; i < int(pcb.Layers.StackUpLayers.size()); ++i)
        if(pcb.Layers.StackUpLayers[i].type == layertype::Signal) {
            if(firstSignal < 0) firstSignal = i;
            lastSignal = i;
        }

    auto layerKindFor = [&](int index, layertype type) -> std::optional<LayerKind> {
        const bool top = firstSignal < 0 || index <= firstSignal;
        switch(type) {
        case layertype::Signal: return index == firstSignal ? LayerKind::CopperTop : index == lastSignal ? LayerKind::CopperBottom
                                                                                                          : LayerKind::CopperInner;
        case layertype::Silk  : return top ? LayerKind::SilkTop : LayerKind::SilkBottom;
        case layertype::Mask  : return top ? LayerKind::MaskTop : LayerKind::MaskBottom;
        default               : return {};
        }
    };

    QString copperTopName, copperBottomName;
    for(int i{}; i < int(pcb.Layers.StackUpLayers.size()); ++i) {
        const auto& sl = pcb.Layers.StackUpLayers[i];
        if(i == firstSignal) copperTopName = QString::fromStdString(sl.name);
        if(i == lastSignal) copperBottomName = QString::fromStdString(sl.name);
        if(auto kind = layerKindFor(i, sl.type))
            file->layer(QString::fromStdString(sl.name), *kind);
    }
    // Плата в одном сигнальном слое (Example_01/SingleLayer.fst): считать его и
    // верхом, и низом сразу незачем -- Top уже покрывает случай.

    Layer* copperTop = copperTopName.isEmpty() ? nullptr : file->layer(copperTopName, LayerKind::CopperTop);
    Layer* copperBottom = copperBottomName.isEmpty() || copperBottomName == copperTopName
        ? nullptr
        : file->layer(copperBottomName, LayerKind::CopperBottom);

    // --- Контур платы.
    Layer* outline = file->layer(QObject::tr("Board Outline"), LayerKind::BoardOutline);
    for(auto&& shape: pcb.Constructive.BoardOutline.Contour) {
        Geo::Polyline pl = figureToPolyline(shape.NonfilledFigure, k);
        if(pl.size() < 3) continue;
        pl.close();
        GraphicObject go;
        go.type = GraphicObject::Polygon;
        go.path = pl;
        go.fill = Geo::Polygons{Geo::Polygon{pl}};
        outline->addGraphicObject(std::move(go));
    }
    for(auto&& v: pcb.Constructive.BoardOutline.Voids) {
        Geo::Polyline pl = figureToPolyline(v.FilledFigure, k);
        if(pl.size() < 3) continue;
        pl.close();
        for(GraphicObject& go: outline->graphicObjects())
            go.fill -= Geo::Polygons{Geo::Polygon{pl}};
    }

    // --- Проводники (аналог "путей апертуры" Gerber).
    for(auto&& wire: pcb.Connectivity.Wires) {
        const QString layerName = QString::fromStdString(wire.LayerRef.name);
        if(layerName.isEmpty()) continue;
        Layer* l = file->layer(layerName, LayerKind::CopperInner);
        for(auto&& sub: wire.Subwires) {
            Geo::Polyline pl = trackToPolyline(sub.Start, sub.Tracks, k);
            if(pl.size() < 2) continue;
            pl.width = sub.width * k;
            GraphicObject go;
            go.type = GraphicObject::PolyLine;
            go.path = pl;
            go.fill = Geo::Inflate(Geo::Polylines{pl});
            l->addGraphicObject(std::move(go));
        }
    }

    // --- Переходные отверстия: площадка на обеих внешних медных слоях +
    // отверстие в синтетическом слое "Vias" (аналог Excellon-разметки Gerber).
    Layer* vias = file->layer(QObject::tr("Vias"), LayerKind::Vias);
    for(auto&& via: pcb.Connectivity.Vias) {
        const auto vs = pcb.LocalLibrary.getViastack(via.ViastackRef.name);
        if(!vs) continue;
        Geo::Polygons pads;
        for(auto&& p: vs->ViaPads) pads |= padShapeToPolygons(p, k);
        const QPointF org = pt(via.Org, k);
        pads = Geo::translated(pads, org);
        for(Layer* l: {copperTop, copperBottom})
            if(l) {
                GraphicObject go;
                go.type = GraphicObject::Circle;
                go.pos = org;
                go.fill = pads;
                l->addGraphicObject(std::move(go));
            }
        GraphicObject hole;
        hole.type = GraphicObject::Circle;
        hole.pos = org;
        hole.fill = Geo::Polygons{Geo::Polylines{Geo::circle(vs->holeDiameter * k, org)}};
        hole.name = QString::number(vs->holeDiameter * k);
        vias->addGraphicObject(std::move(hole));
    }

    // --- Раскладка компонентов + площадки посадочных мест. Компоненты не
    // заводят отдельный слой в дереве -- их показывает диалог "Components"
    // (см. topor_node.cpp), геометрия площадок уходит прямо на медные слои.
    for(auto&& inst: pcb.ComponentsOnBoard.Components) {
        Component comp;
        comp.refDes = QString::fromStdString(inst.name);
        comp.componentRef = QString::fromStdString(inst.ComponentRef.name);
        comp.footprint = QString::fromStdString(inst.FootprintRef.name);
        comp.side = inst.side == side::Bottom ? ::Bottom : ::Top;
        comp.angle = inst.angle;
        comp.pos = pt(inst.Org, k);
        comp.fixed = inst.fixed == Bool::on;
        for(auto&& attr: inst.Attributes)
            if(attr.type && *attr.type == type::PartName) comp.value = QString::fromStdString(attr.value);
        file->components().push_back(std::move(comp));

        const auto fp = pcb.LocalLibrary.getFootprint(inst.FootprintRef.name);
        if(!fp) continue;
        const QTransform t = compTransform(inst);

        for(auto&& pad: fp->Pads) {
            const auto ps = pcb.LocalLibrary.getPadstack(pad.PadstackRef.name);
            if(!ps) continue;
            Geo::Polygons region = padstackToPolygons(*ps, k);
            region = Geo::transformed(region, pad.transform());
            region = Geo::transformed(region, t);

            GraphicObject go;
            go.type = GraphicObject::Circle;
            go.name = QString::fromStdString(inst.name) + u'.' + QString::number(pad.padNum);
            go.fill = region;
            for(Layer* l: {copperTop, copperBottom})
                if(l) l->addGraphicObject(GraphicObject{go});
        }
    }

    // --- Одиночные площадки вне компонентов.
    for(auto&& fp: pcb.ComponentsOnBoard.FreePads) {
        const auto ps = pcb.LocalLibrary.getPadstack(fp.PadstackRef.name);
        if(!ps) continue;
        Geo::Polygons region = padstackToPolygons(*ps, k);
        QTransform t = fp.transform();
        if(fp.side == side::Bottom) {
            QTransform mirror;
            mirror.scale(-1, 1);
            t *= mirror;
        }
        region = Geo::transformed(region, t);
        for(Layer* l: {copperTop, copperBottom})
            if(l) {
                GraphicObject go;
                go.type = GraphicObject::Circle;
                go.name = QString::fromStdString(fp.name);
                go.fill = region;
                l->addGraphicObject(std::move(go));
            }
    }

    return file;
}

} // namespace TopoR
