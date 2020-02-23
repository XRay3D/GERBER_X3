#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QPointF>

enum class Colors : int {
    Background,
    Pin,
    CutArea,
    Grid1,
    Grid5,
    Grid10,
    Hole,
    Home,
    ToolPath,
    Zero,
    G0,
    Count
};

namespace HomePosition {
enum {
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight,
    AlwaysZero
};
}

class Settings {
public:
    Settings();

    /*G-Code*/
    static QString gcEnd();
    static QString gcFileExtension();
    static QString gcFormat();
    static bool gcInfo();
    static QString gcLaserConstOn();
    static QString gcLaserDynamOn();
    static bool gcSameFolder();
    static QString gcSpindleLaserOff();
    static QString gcSpindleOn();
    static QString gcStart();

    /*GUI*/
    static QColor& guiColor(Colors id);
    static bool guiSmoothScSh();

    /*Gerber/G-Code*/
    static int gbrGcCircleSegments(double radius);

    /*Gerber*/
    static bool gbrCleanPolygons();
    static bool gbrSimplifyRegions();
    static bool gbrSkipDuplicates();

    static QPointF mkrHomeOffset();
    static int mkrHomePos();
    static QPointF mkrPinOffset();
    static QPointF mkrZeroOffset();
    static int mkrZeroPos();
    /*Other*/
    static double gridStep(double scale);
    static bool inch();
    static void setInch(bool val);

protected:
    /*G-Code*/
    static QString m_gcEnd;
    static QString m_gcFileExtension;
    static QString m_gcFormat;
    static bool m_gcInfo;
    static QString m_gcLaserConstOn;
    static QString m_gcLaserDynamOn;
    static bool m_gcSameFolder;
    static QString m_gcSpindleLaserOff;
    static QString m_gcSpindleOn;
    static QString m_gcStart;

    /*GUI*/
    static QColor m_guiColor[static_cast<int>(Colors::Count)];
    static bool m_guiSmoothScSh;

    /*Gerber/G-Code*/
    static double m_gbrGcMinCircleSegmentLength;
    static int m_gbrGcMinCircleSegments;

    /*Gerber*/
    static bool m_gbrCleanPolygons;
    static bool m_gbrSimplifyRegions;
    static bool m_gbrSkipDuplicates;

    /*Markers*/
    static QPointF m_mrkHomeOffset;
    static int m_mrkHomePos;
    static QPointF m_mrkPinOffset;
    static QPointF m_mrkZeroOffset;
    static int m_mrkZeroPos;

    /*Other*/
    static bool m_inch;
};

#endif // SETTINGS_H
