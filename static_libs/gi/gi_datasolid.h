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
    explicit DataFill(Curves curves, AbstractFile* file);

    ~DataFill() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // Item interface
    void redraw() override;

    // Curvess& getCurvess();
    void setCurves(Curves paths, int alternate = {}) override;
    // Item interface
    void changeColor() override;
};
} // namespace Gi
