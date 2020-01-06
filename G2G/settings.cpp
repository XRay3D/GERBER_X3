#include "settings.h"
#include <math.h>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif
const int gridColor = 100;

QColor Settings::m_color[static_cast<int>(Colors::Count)] {
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

QPointF Settings::m_homeOffset;
QPointF Settings::m_zeroOffset;
QPointF Settings::m_pinOffset;
QString Settings::m_GCode = "G?X?Y?Z?F?S?";
QString Settings::m_endGCode = "M30";
QString Settings::m_startGCode = "G21 G17 G90";
bool Settings::m_cleanPolygons = true;
bool Settings::m_gcinfo = false;
bool Settings::m_inch = false; //true;
bool Settings::m_skipDuplicates = true;
bool Settings::m_smoothScSh = false; //true;
bool Settings::m_simplifyRegions = true;
double Settings::m_minCircleSegmentLength = 0.5;
int Settings::m_homePos = Qt::BottomLeftCorner;
int Settings::m_zeroPos = Qt::BottomLeftCorner;
int Settings::m_minCircleSegments = 36;

Settings::Settings() {}

int Settings::circleSegments(double radius)
{
    const double length = m_minCircleSegmentLength; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_minCircleSegments;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

QColor& Settings::color(Colors id) { return m_color[static_cast<int>(id)]; }
QPointF Settings::homeOffset() { return m_homeOffset; }
QPointF Settings::pinOffset() { return m_pinOffset; }
QPointF Settings::zeroOffset() { return m_zeroOffset; }
QString Settings::endGCode() { return m_endGCode; }
QString Settings::gCodeFormat() { return m_GCode; }
QString Settings::startGCode() { return m_startGCode; }

bool Settings::cleanPolygons() { return m_cleanPolygons; }
bool Settings::gcinfo() { return m_gcinfo; }
bool Settings::inch() { return m_inch; }
bool Settings::skipDuplicates() { return m_skipDuplicates; }
bool Settings::simplifyRegions() { return m_simplifyRegions; }

int Settings::homePos() { return m_homePos; }
int Settings::zeroPos() { return m_zeroPos; }
void Settings::setInch(bool val) { m_inch = val; }

double Settings::gridStep(double scale)
{
    return m_inch
        ? pow(10.0, ceil(log10(30 / scale))) * .254
        : pow(10.0, ceil(log10(8.0 / scale))); // 0.1;
}

bool Settings::smoothScSh() { return m_smoothScSh; }
