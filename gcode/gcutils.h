#pragma once

#ifndef GCUTILS_H
#define GCUTILS_H

namespace GCode {
class GCUtils {
public:
    GCUtils();

    inline double feedRate() { return m_feedRate; }
    inline double plungeRate() { return m_plungeRate; }
    inline int spindleSpeed() { return m_spindleSpeed; }
    inline int toolType() { return m_toolType; }

    void setFeedRate(double val) { m_feedRate = val; }
    void setPlungeRate(double val) { m_plungeRate = val; }
    void setSpindleSpeed(int val) { m_spindleSpeed = val; }
    void setToolType(int val) { m_toolType = val; }

private:
    double m_feedRate = 0.0;
    double m_plungeRate = 0.0;
    int m_spindleSpeed = 0;
    int m_toolType = 0;
};
}

#endif // GCUTILS_H
