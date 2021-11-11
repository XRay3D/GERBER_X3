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
QColor& AppSettings::guiColor(int id) { return m_guiColor[id]; }
bool AppSettings::animSelection() { return m_animSelection; }
bool AppSettings::guiSmoothScSh() { return m_guiSmoothScSh; }
bool AppSettings::scaleHZMarkers() { return m_scaleHZMarkers; }
bool AppSettings::scalePinMarkers() { return m_scalePinMarkers; }
int AppSettings::theme() { return m_theme; }

/*Clipper*/
int AppSettings::clpCircleSegments(double radius)
{
    const double length = m_clpMinCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_clpMinCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Markers*/
QPointF AppSettings::mkrHomeOffset() { return m_mrkHomeOffset; }
int AppSettings::mkrHomePos() { return m_mrkHomePos; }
QPointF AppSettings::mkrPinOffset() { return m_mrkPinOffset; }
QPointF AppSettings::mkrZeroOffset() { return m_mrkZeroOffset; }
int AppSettings::mkrZeroPos() { return m_mrkZeroPos; }

/*Other*/
double AppSettings::gridStep(double scale) { return m_inch ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool AppSettings::inch() { return m_inch; }
void AppSettings::setInch(bool val) { m_inch = val; }

QPointF AppSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod)
{
    if ((mod & Qt::ALT) || m_snap) {
        const double gs = AppSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pt / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return pt;
}

void AppSettings::setSnap(bool val)
{
    m_snap = val;
}

bool AppSettings::snap() { return m_snap; }
