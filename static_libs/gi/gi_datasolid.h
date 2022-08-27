/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "gi.h"

namespace Gerber {
class File;
}

class GiDataSolid final : public GraphicsItem {
    Paths& paths_;

public:
    explicit GiDataSolid(Paths& paths_, FileInterface* file);
    ~GiDataSolid() override;

    // QGraphicsItem interface
    //   QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    // GraphicsItem interface
    void redraw() override;
    void setPaths(Paths paths, int alternate = {}) override;
    // GraphicsItem interface
    void changeColor() override;
};
