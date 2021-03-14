/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <graphicsitem.h>
#define QT_DEBUG
namespace GCode {
class File;
}

class GcPathItem : public GraphicsItem {
public:
    GcPathItem(const Paths& paths, GCode::File* file = nullptr);
    GcPathItem(const Path& path, GCode::File* file = nullptr);
    ~GcPathItem() override = default;
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
#undef QT_DEBUG
