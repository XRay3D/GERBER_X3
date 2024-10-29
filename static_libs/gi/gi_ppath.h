/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include <cgal_solver.h>
#include <qpainterpath.h>

namespace GCode {
class File;
}

namespace Gi {

class PPath : public Item {
public:
    PPath(const Polygon_with_holes_2& paths, AbstractFile* file = nullptr);
    PPath(const Polygon_2& path, AbstractFile* file = nullptr);
    PPath(const QPainterPath& path, AbstractFile* file = nullptr);
    ~PPath() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths(int alternate = {}) const override;

private:
    QPainterPath arrows_;
    double sc_ = 0;
    void updateArrows();

protected:
    void changeColor() override { }
};
} // namespace Gi
