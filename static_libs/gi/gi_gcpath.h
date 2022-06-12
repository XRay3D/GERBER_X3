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
#define QT_DEBUG
namespace GCode {
class File;
}

class GiGcPath : public GraphicsItem {
public:
    GiGcPath(const Paths& paths, GCode::File* file = nullptr);
    GiGcPath(const Path& path, GCode::File* file = nullptr);
    ~GiGcPath() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths(int alternate = {}) const override;

private:
    GCode::File* m_gcFile;
#ifdef QT_DEBUG
    QPainterPath m_arrows;
    double m_sc = 0;
    void updateArrows();
#endif
protected:
    void changeColor() override { }
};
#undef QT_DEBUG
