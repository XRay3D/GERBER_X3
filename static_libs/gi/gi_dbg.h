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
#include <qpainterpath.h>

namespace Gi {

class Debug : public Item {
public:
    Debug(const Paths& paths, const QPen& = {Qt::white, 1.0});
    Debug(const Path& path, const QPen& = {Qt::white, 1.0});
    Debug(const QPainterPath& path, const QPen& = {Qt::white, 1.0});
    ~Debug() override = default;
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
