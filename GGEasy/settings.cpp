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
#include "graphicsview.h"
#include <cmath>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

/*G-Code*/
QString GlobalSettings::gcFileExtension() { return m_gcFileExtension; }
QString GlobalSettings::gcFormat() { return m_gcFormat; }
QString GlobalSettings::gcLaserConstOn() { return m_gcLaserConstOn; }
QString GlobalSettings::gcLaserDynamOn() { return m_gcLaserDynamOn; }

QString GlobalSettings::gcSpindleLaserOff() { return m_gcSpindleLaserOff; }
QString GlobalSettings::gcSpindleOn() { return m_gcSpindleOn; }

QString GlobalSettings::gcLaserStart() { return m_gcLaserStart; }
QString GlobalSettings::gcLaserEnd() { return m_gcLaserEnd; }

QString GlobalSettings::gcStart() { return m_gcStart; }
QString GlobalSettings::gcEnd() { return m_gcEnd; }

bool GlobalSettings::gcInfo() { return m_gcInfo; }
bool GlobalSettings::gcSameFolder() { return m_gcSameFolder; }

/*GUI*/
QColor& GlobalSettings::guiColor(Colors id) { return m_guiColor[static_cast<int>(id)]; }
bool GlobalSettings::guiSmoothScSh() { return m_guiSmoothScSh; }

/*Gerber/G-Code*/
int GlobalSettings::gbrGcCircleSegments(double radius)
{
    const double length = m_gbrGcMinCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_gbrGcMinCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Gerber*/
bool GlobalSettings::gbrCleanPolygons() { return m_gbrCleanPolygons; }
bool GlobalSettings::gbrSimplifyRegions() { return m_gbrSimplifyRegions; }
bool GlobalSettings::gbrSkipDuplicates() { return m_gbrSkipDuplicates; }

/*Markers*/
QPointF GlobalSettings::mkrHomeOffset() { return m_mrkHomeOffset; }
int GlobalSettings::mkrHomePos() { return m_mrkHomePos; }
QPointF GlobalSettings::mkrPinOffset() { return m_mrkPinOffset; }
QPointF GlobalSettings::mkrZeroOffset() { return m_mrkZeroOffset; }
int GlobalSettings::mkrZeroPos() { return m_mrkZeroPos; }

/*Other*/
double GlobalSettings::gridStep(double scale) { return m_inch ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool GlobalSettings::inch() { return m_inch; }
void GlobalSettings::setInch(bool val) { m_inch = val; }

QPointF GlobalSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod)
{
    if (mod & Qt::ALT || m_snap) {
        const double gs = GlobalSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pt / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return pt;
}
