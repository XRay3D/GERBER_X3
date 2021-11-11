// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcdrillitem.h"

namespace GCode {

DrillItem::DrillItem(double diameter, GCode::File* file)
    : AbstractDrillItem(reinterpret_cast<FileInterface*>(file))
    , m_diameter(diameter)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    create();
    changeColor();
}

bool DrillItem::isSlot() { return false; }

Paths DrillItem::paths(int) const
{
    Path path(CirclePath(m_diameter * uScale, (pos())));
    ReversePath(path);
    return { path };
}

void DrillItem::create()
{
    m_shape = QPainterPath();
    auto path(CirclePath(m_diameter * uScale));
    path.push_back(path.front());
    m_shape.addPolygon(path);
    m_rect = m_shape.boundingRect();
}

}
