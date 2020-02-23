#include "settings.h"
#include <cmath>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

/*G-Code*/
QString Settings::m_gcEnd = "M30";
QString Settings::m_gcFileExtension = "tap";
QString Settings::m_gcFormat = "G?X?Y?Z?F?S?";
QString Settings::m_gcLaserConstOn { "M3" };
QString Settings::m_gcLaserDynamOn { "M4" };
QString Settings::m_gcSpindleLaserOff { "M5" };
QString Settings::m_gcSpindleOn { "M3" };
QString Settings::m_gcStart = "G21 G17 G90";
bool Settings::m_gcInfo = false;
bool Settings::m_gcSameFolder { true };

/*GUI*/
constexpr int gridColor = 100;
QColor Settings::m_guiColor[static_cast<int>(Colors::Count)] {
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
bool Settings::m_guiSmoothScSh = false;

/*Gerber/G-Code*/
double Settings::m_gbrGcMinCircleSegmentLength = 0.5;
int Settings::m_gbrGcMinCircleSegments = 36;

/*Gerber*/
bool Settings::m_gbrCleanPolygons = false;
bool Settings::m_gbrSimplifyRegions = false;
bool Settings::m_gbrSkipDuplicates = false;

/*Markers*/
QPointF Settings::m_mrkHomeOffset;
QPointF Settings::m_mrkPinOffset;
QPointF Settings::m_mrkZeroOffset;
int Settings::m_mrkHomePos = Qt::BottomLeftCorner;
int Settings::m_mrkZeroPos = Qt::BottomLeftCorner;

/*Other*/
bool Settings::m_inch = false;

Settings::Settings() {}

/*G-Code*/
QString Settings::gcEnd() { return m_gcEnd; }
QString Settings::gcFileExtension() { return m_gcFileExtension; }
QString Settings::gcFormat() { return m_gcFormat; }
bool Settings::gcInfo() { return m_gcInfo; }
QString Settings::gcLaserConstOn() { return m_gcLaserConstOn; }
QString Settings::gcLaserDynamOn() { return m_gcLaserDynamOn; }
bool Settings::gcSameFolder() { return m_gcSameFolder; }
QString Settings::gcSpindleLaserOff() { return m_gcSpindleLaserOff; }
QString Settings::gcSpindleOn() { return m_gcSpindleOn; }
QString Settings::gcStart() { return m_gcStart; }

/*GUI*/
QColor& Settings::guiColor(Colors id) { return m_guiColor[static_cast<int>(id)]; }
bool Settings::guiSmoothScSh() { return m_guiSmoothScSh; }

/*Gerber/G-Code*/
int Settings::gbrGcCircleSegments(double radius)
{
    const double length = m_gbrGcMinCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_gbrGcMinCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Gerber*/
bool Settings::gbrCleanPolygons() { return m_gbrCleanPolygons; }
bool Settings::gbrSimplifyRegions() { return m_gbrSimplifyRegions; }
bool Settings::gbrSkipDuplicates() { return m_gbrSkipDuplicates; }

/*Markers*/
QPointF Settings::mkrHomeOffset() { return m_mrkHomeOffset; }
int Settings::mkrHomePos() { return m_mrkHomePos; }
QPointF Settings::mkrPinOffset() { return m_mrkPinOffset; }
QPointF Settings::mkrZeroOffset() { return m_mrkZeroOffset; }
int Settings::mkrZeroPos() { return m_mrkZeroPos; }

/*Other*/
double Settings::gridStep(double scale) { return m_inch ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool Settings::inch() { return m_inch; }
void Settings::setInch(bool val) { m_inch = val; }
