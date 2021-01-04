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
    if (!m_settings)
        m_settings = this;
}

void AppSettings::setApp(AppSettings* appSettings) { m_settings = appSettings; }
AppSettings* AppSettings::appSettings() { return m_settings; }

QString AppSettings::gcFileExtension() { return m_settings->m_gcFileExtension; }
QString AppSettings::gcFormat() { return m_settings->m_gcFormat; }
QString AppSettings::gcLaserConstOn() { return m_settings->m_gcLaserConstOn; }
QString AppSettings::gcLaserDynamOn() { return m_settings->m_gcLaserDynamOn; }

QString AppSettings::gcSpindleLaserOff() { return m_settings->m_gcSpindleLaserOff; }
QString AppSettings::gcSpindleOn() { return m_settings->m_gcSpindleOn; }

QString AppSettings::gcLaserStart() { return m_settings->m_gcLaserStart; }
QString AppSettings::gcLaserEnd() { return m_settings->m_gcLaserEnd; }

QString AppSettings::gcStart() { return m_settings->m_gcStart; }
QString AppSettings::gcEnd() { return m_settings->m_gcEnd; }

bool AppSettings::gcInfo() { return m_settings->m_gcInfo; }
bool AppSettings::gcSameFolder() { return m_settings->m_gcSameFolder; }

/*DXF*/
QString AppSettings::dxfDefaultFont() { return m_settings->m_dxfDefaultFont; }
bool AppSettings::dxfBoldFont() { return m_settings->m_dxfBoldFont; }
bool AppSettings::dxfItalicFont() { return m_settings->m_dxfItalicFont; }
bool AppSettings::dxfOverrideFonts() { return m_settings->m_dxfOverrideFonts; }

/*GUI*/
QColor& AppSettings::guiColor(Colors id) { return m_settings->m_guiColor[static_cast<int>(id)]; }
bool AppSettings::guiSmoothScSh() { return m_settings->m_guiSmoothScSh; }
bool AppSettings::animSelection() { return m_settings->m_animSelection; }

/*Gerber/G-Code*/
int AppSettings::gbrGcCircleSegments(double radius)
{
    const double length = m_settings->m_gbrGcMinCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_settings->m_gbrGcMinCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Gerber*/
bool AppSettings::gbrCleanPolygons() { return m_settings->m_gbrCleanPolygons; }
bool AppSettings::gbrSimplifyRegions() { return m_settings->m_gbrSimplifyRegions; }
bool AppSettings::gbrSkipDuplicates() { return m_settings->m_gbrSkipDuplicates; }

/*Markers*/
QPointF AppSettings::mkrHomeOffset() { return m_settings->m_mrkHomeOffset; }
int AppSettings::mkrHomePos() { return m_settings->m_mrkHomePos; }
QPointF AppSettings::mkrPinOffset() { return m_settings->m_mrkPinOffset; }
QPointF AppSettings::mkrZeroOffset() { return m_settings->m_mrkZeroOffset; }
int AppSettings::mkrZeroPos() { return m_settings->m_mrkZeroPos; }

/*Other*/
double AppSettings::gridStep(double scale) { return m_settings->m_inch ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool AppSettings::inch() { return m_settings->m_inch; }
void AppSettings::setInch(bool val) { m_settings->m_inch = val; }

QPointF AppSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod)
{
    if (mod & Qt::ALT || m_settings->m_snap) {
        const double gs = AppSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pt / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return pt;
}

void AppSettings::setSnap(bool val) { m_settings->m_snap = val; }

bool AppSettings::snap() { return m_settings->m_snap; }
