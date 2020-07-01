// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "settings.h"
#include <cmath>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

/*G-Code*/

QString GlobalSettings::m_gcFileExtension = { "tap" };
QString GlobalSettings::m_gcFormat{ "G?X?Y?Z?F?S?" };
QString GlobalSettings::m_gcLaserConstOn{ "M3" };
QString GlobalSettings::m_gcLaserDynamOn{ "M4" };
QString GlobalSettings::m_gcSpindleLaserOff{ "M5" };
QString GlobalSettings::m_gcSpindleOn{ "M3" };

QString GlobalSettings::m_gcStart{ "G21 G17 G90\nM3 S?" };
QString GlobalSettings::m_gcEnd{ "M5\nM30" };

QString GlobalSettings::m_gcLaserStart{ "G21 G17 G90" };
QString GlobalSettings::m_gcLaserEnd{ "M30" };

bool GlobalSettings::m_gcInfo{ false };
bool GlobalSettings::m_gcSameFolder{ true };

/*GUI*/
enum { gridColor = 100 };
QColor GlobalSettings::m_guiColor[static_cast<int>(Colors::Count)]{
    QColor(), //Background
    QColor(255, 255, 0, 120), //Pin
    QColor(Qt::gray), //CutArea
    QColor(gridColor, gridColor, gridColor, 50), //Grid1
    QColor(gridColor, gridColor, gridColor, 100), //Grid5
    QColor(gridColor, gridColor, gridColor, 200), //Grid10
    QColor(), //Hole
    QColor(0, 255, 0, 120), //Home
    QColor(Qt::black), //ToolPath
    QColor(255, 0, 0, 120), //Zero
    QColor(Qt::red) //G0
};

bool GlobalSettings::m_guiSmoothScSh{ false };

/*Gerber/G-Code*/
double GlobalSettings::m_gbrGcMinCircleSegmentLength{ 0.5 };
int GlobalSettings::m_gbrGcMinCircleSegments{ 36 };

/*Gerber*/
bool GlobalSettings::m_gbrCleanPolygons{ false };
bool GlobalSettings::m_gbrSimplifyRegions{ false };
bool GlobalSettings::m_gbrSkipDuplicates{ false };

/*Markers*/
QPointF GlobalSettings::m_mrkHomeOffset;
QPointF GlobalSettings::m_mrkPinOffset;
QPointF GlobalSettings::m_mrkZeroOffset;
int GlobalSettings::m_mrkHomePos{ Qt::BottomLeftCorner };
int GlobalSettings::m_mrkZeroPos{ Qt::BottomLeftCorner };

/*Other*/
bool GlobalSettings::m_inch = false;

GlobalSettings::GlobalSettings() {}

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
