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
#define QT_DEBUG
namespace GCode {
class File;
}

class GiGcPath : public GraphicsItem {
public:
    GiGcPath(const Paths& paths, AbstractFile* file = nullptr);
    GiGcPath(const Path& path, AbstractFile* file = nullptr);
    ~GiGcPath() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths(int alternate = {}) const override;

private:
    AbstractFile* gcFile_;
#ifdef QT_DEBUG
    QPainterPath arrows_;
    double sc_ = 0;
    void updateArrows();
#endif
protected:
    void changeColor() override { }
};
#undef QT_DEBUG
