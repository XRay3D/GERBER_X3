/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "file.h"
#include "gc_types.h"
#include <QList>
#include <QString>

class Project;

namespace GCode {

class GCFile : public FileInterface {
    friend class ::Project;

public:
    GCFile(GCodeParams&& gcp);
    GCFile() {};

    inline double feedRate() { return feedRate_; }
    inline double plungeRate() { return plungeRate_; }
    inline int spindleSpeed() { return spindleSpeed_; }
    inline int toolType() { return toolType_; }

    void setFeedRate(double val) { feedRate_ = val; }
    void setPlungeRate(double val) { plungeRate_ = val; }
    void setSpindleSpeed(int val) { spindleSpeed_ = val; }
    void setToolType(int val) { toolType_ = val; }

    static QString getLastDir();
    static void setLastDir(QString dirPath);

private:
    double feedRate_ = 0.0;
    double plungeRate_ = 0.0;
    int spindleSpeed_ = 0;
    int toolType_ = 0;

protected:
    /*const*/ GCodeParams gcp_; ////
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

    Paths g0path_;

    static inline QString lastDir;
    static inline bool redirected;
    inline static const mvector<QChar> cmdList {'G', 'X', 'Y', 'Z', 'F', 'S'};

    mvector<double> getDepths();

    bool formatFlags[Size];
    QString lastValues[6];
    Code gCode_ = GNull;

    inline QString g0() {
        gCode_ = G00;
        return "G0";
    }

    inline QString g1() {
        gCode_ = G01;
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
                lines.emplace_back(formated({g1(), x(point.x()), y(point.y()), feed(feedRate()), speed(spindleSpeed)}));
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

    virtual void pathsToGCode() { }
};

} // namespace GCode
