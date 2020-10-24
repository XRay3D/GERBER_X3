/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "graphicsitem.h"

namespace GCode {
class File;
}

class PathItem : public GraphicsItem {
public:
    PathItem(const Paths& paths, GCode::File* file = nullptr);
    PathItem(const Path& path, GCode::File* file = nullptr);
    ~PathItem() override = default;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;

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
