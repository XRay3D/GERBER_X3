/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once
#include "gc_types.h"
#include <QList>
#include <QString>
#include <myclipper.h>

class Project;

namespace GCode {

class GCUtils {
    friend class ::Project;

public:
    GCUtils(const GCodeParams& gcp);

    inline double feedRate() { return m_feedRate; }
    inline double plungeRate() { return m_plungeRate; }
    inline int spindleSpeed() { return m_spindleSpeed; }
    inline int toolType() { return m_toolType; }

    void setFeedRate(double val) { m_feedRate = val; }
    void setPlungeRate(double val) { m_plungeRate = val; }
    void setSpindleSpeed(int val) { m_spindleSpeed = val; }
    void setToolType(int val) { m_toolType = val; }

    static QString getLastDir();
    static void setLastDir(QString dirPath);

private:
    double m_feedRate = 0.0;
    double m_plungeRate = 0.0;
    int m_spindleSpeed = 0;
    int m_toolType = 0;
    const GCodeParams& m_gcp; ////

protected:
    enum {
        AlwaysG,
        AlwaysX,
        AlwaysY,
        AlwaysZ,
        AlwaysF,
        AlwaysS,

        SpaceG,
        SpaceX,
        SpaceY,
        SpaceZ,
        SpaceF,
        SpaceS,

        Size
    };

    Paths m_g0path;

    static inline QString lastDir;
    static inline bool redirected;
    inline static const mvector<QChar> cmdList { 'G', 'X', 'Y', 'Z', 'F', 'S' };

    mvector<double> getDepths();

    bool formatFlags[Size];
    QString lastValues[6];
    Code m_gCode = GNull;

    inline QString g0() {
        m_gCode = G00;
        return "G0";
    }

    inline QString g1() {
        m_gCode = G01;
        return "G1";
    }

    mvector<QString> savePath(const QPolygonF& path, double spindleSpeed) {
        mvector<QString> lines;
        lines.reserve(path.size());
        bool skip = true;
        for (const QPointF& point : path) {
            if (skip)
                skip = false;
            else
                lines.push_back(formated({ g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed) }));
        }
        return lines;
    }

    QString formated(const mvector<QString>& data) {
        QString ret;
        for (const QString& str : data) {
            const int index = cmdList.indexOf(str.front().toUpper());
            if (index != -1) {
                if (formatFlags[AlwaysG + index] || lastValues[index] != str) {
                    lastValues[index] = str;
                    ret += str + (formatFlags[SpaceG + index] ? " " : "");
                }
            }
        }
        return ret.trimmed();
    }

    inline QString x(double val) { return 'X' + format(val); }
    inline QString y(double val) { return 'Y' + format(val); }
    inline QString z(double val) { return 'Z' + format(val); }
    inline QString feed(double val) { return 'F' + format(val); }
    inline QString speed(int val) { return 'S' + QString::number(val); }
    inline QString format(double val) {
        QString str(QString::number(val, 'g', (abs(val) < 1 ? 3 : (abs(val) < 10 ? 4 : (abs(val) < 100 ? 5 : 6)))));
        if (str.contains('e'))
            return QString::number(val, 'f', 3);
        return str;
    }
};
} // namespace GCode
