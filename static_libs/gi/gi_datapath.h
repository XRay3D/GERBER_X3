/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include "gi.h"
#include <QTimer>
#include <cmath>

class MainWindow;

namespace Gi {

class DataPath final : public Item {
    // Обводка «прицела» шириной 5 экранных пикселей. Она же -- геометрия
    // попадания: shape() отдаёт именно её, а не сам контур.
    //
    // Строится ЛЕНИВО. Раньше updateSelection() звался из конструктора (обводка
    // каждого пути на загрузке) и из paint() (обводка каждого видимого пути на
    // каждом шаге зума), причём правил boundingRect_ без
    // prepareGeometryChange() -- индекс BSP протухал.
    mutable QPainterPath selectionShape_;
    mutable double strokeScale_ = std::numeric_limits<double>::quiet_NaN();
    const QPainterPath& selectionShape(double sf) const;

    void redraw() override { update(); }
    void changeColor() override { }
    void geometryChanged() override;
    friend class ::MainWindow;

public:
    DataPath(Geo::Polylines curves, AbstractFile* file);
    // DataPath(const Path& path, AbstractFile* file)
    //     : DataPath{{toCurve(path)}, file} { }
    // DataPath(const Paths& paths, AbstractFile* file)
    //     : DataPath{toCurves(paths), file} { }

protected:
    // QGraphicsItem interface
    QPainterPath shape() const override;
    int type() const override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void paintGeometry(QPainter* painter, const RenderState& st) override;
    void paintHighlight(QPainter* painter, const RenderState& st) override;
};

} // namespace Gi
