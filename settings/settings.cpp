// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "settings.h"
#include "app.h"
#include "graphicsview.h"
#include <cmath>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

/*G-Code*/
AppSettings::AppSettings()
{
    //    if (!m_settings)
    //        m_settings = this;
}

//void AppSettings::set(AppSettings* appSettings) { m_settings = appSettings; }
//AppSettings* AppSettings::ptr() { return m_settings; }

/*GUI*/
QColor& AppSettings::guiColor(int id) { return /*m_settings->*/ m_guiColor[id]; }
bool AppSettings::guiSmoothScSh() { return /*m_settings->*/ m_guiSmoothScSh; }
bool AppSettings::animSelection() { return /*m_settings->*/ m_animSelection; }

/*Clipper*/
int AppSettings::clpCircleSegments(double radius)
{
    const double length = /*m_settings->*/ m_clpMinCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = /*m_settings->*/ m_clpMinCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Markers*/
QPointF AppSettings::mkrHomeOffset() { return /*m_settings->*/ m_mrkHomeOffset; }
int AppSettings::mkrHomePos() { return /*m_settings->*/ m_mrkHomePos; }
QPointF AppSettings::mkrPinOffset() { return /*m_settings->*/ m_mrkPinOffset; }
QPointF AppSettings::mkrZeroOffset() { return /*m_settings->*/ m_mrkZeroOffset; }
int AppSettings::mkrZeroPos() { return /*m_settings->*/ m_mrkZeroPos; }

/*Other*/
double AppSettings::gridStep(double scale) { return /*m_settings->*/ m_inch ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool AppSettings::inch() { return /*m_settings->*/ m_inch; }
void AppSettings::setInch(bool val)
{ /*m_settings->*/
    m_inch = val;
}

QPointF AppSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod)
{
    if (mod & Qt::ALT || /*m_settings->*/ m_snap) {
        const double gs = AppSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pt / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return pt;
}

void AppSettings::setSnap(bool val)
{ /*m_settings->*/
    m_snap = val;
}

bool AppSettings::snap() { return /*m_settings->*/ m_snap; }
