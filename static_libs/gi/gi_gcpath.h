/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#define QT_DEBUG
namespace GCode {
class File;
}

class GiGcPath : public GraphicsItem {
public:
    GiGcPath(const PathsD& paths, GCode::File* file = nullptr);
    GiGcPath(const PathD& path, GCode::File* file = nullptr);
    ~GiGcPath() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    PathsD paths(int alternate = {}) const override;

private:
    GCode::File* gcFile_;
#ifdef QT_DEBUG
    QPainterPath arrows_;
    double sc_ = 0;
    void updateArrows();
#endif
protected:
    void changeColor() override { }
};
#undef QT_DEBUG
