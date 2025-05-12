/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
namespace GCode {
class File;
}

namespace Gi {

class GcPath : public Item {
public:
    GcPath(const Path& path, AbstractFile* file = nullptr);
    GcPath(const Paths& paths, AbstractFile* file = nullptr);
    ~GcPath() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths(int alternate = {}) const override;

private:
    [[maybe_unused]] AbstractFile* gcFile_;
    QPainterPath arrows_;
    double sc_{};
    void updateArrows();

protected:
    void changeColor() override { }
};
} // namespace Gi
