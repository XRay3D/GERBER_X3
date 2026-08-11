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

namespace Gerber {
class File;
}

namespace Gi {

class DataFill final : public Item {

public:
    explicit DataFill(Geo::Polygons curves, AbstractFile* file);

    ~DataFill() override = default;

    // QGraphicsItem interface
    void paintGeometry(QPainter* painter, const RenderState& st) override;
    int type() const override;
    // Item interface
    void redraw() override;

    // Geo::Polygons& getCurvess();
    void setCurves(Geo::Polylines paths, int alternate = {}) override;
    Geo::Polygons region() const override;
    // Item interface
    void changeColor() override;

private:
    // Заливка -- единственный элемент, у которого вложенность известна точно:
    // она пришла из файла разобранным регионом. Плоские контуры (curves_) её
    // теряют, поэтому регион хранится отдельно, а не выводится из них.
    Geo::Polygons region_;
};
} // namespace Gi
