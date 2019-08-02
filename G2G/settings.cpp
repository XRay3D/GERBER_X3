#include "settings.h"
#include <math.h>

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif
const int gridColor = 100;

QColor Settings::m_color[Colors::Count]{
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

int Settings::m_minCircleSegments = 36;

double Settings::m_minCircleSegmentLength = 0.5;

bool Settings::m_cleanPolygons = true;

bool Settings::m_skipDuplicates = false;

bool Settings::m_gcinfo = false;

QString Settings::m_startGCode = "G21 G17 G90";

QString Settings::m_endGCode = "M30";

QString Settings::m_GCode = "G?X?Y?Z?F?S?";

QPointF Settings::m_pinOffset;

QPointF Settings::m_homeOffset;

int Settings::m_homePos = 0;

Settings::Settings() {}

int Settings::circleSegments(double radius)
{
    const double length = m_minCircleSegmentLength; // 0.5; // mm
    const int destSteps = static_cast<int>(M_PI / asin((length * 0.5) / (radius)));
    int intSteps = m_minCircleSegments; //MinStepsPerCircle; //32 aka 10 degres
    while (intSteps < destSteps)
        intSteps <<= 1; // aka *= 2 // resize to desination 0.5 mm rib length
    return intSteps;
}

QColor& Settings::color(Colors id) { return m_color[static_cast<int>(id)]; }

bool Settings::cleanPolygons() { return m_cleanPolygons; }

bool Settings::skipDuplicates() { return m_skipDuplicates; }

QString Settings::startGCode() { return m_startGCode; }

QString Settings::endGCode() { return m_endGCode; }

QString Settings::gCodeFormat() { return m_GCode; }

QPointF Settings::homeOffset() { return m_homeOffset; }

int Settings::homePos() { return m_homePos; }

QPointF Settings::pinOffset()
{
    return m_pinOffset;
}

bool Settings::gcinfo()
{
    return m_gcinfo;
}
