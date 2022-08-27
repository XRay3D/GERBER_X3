/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include <QObject>
#include <QPainterPath>

class QWidget;
class QIcon;

class Tool {
    friend QDataStream& operator<<(QDataStream& stream, const Tool& tool);
    friend QDataStream& operator>>(QDataStream& stream, Tool& tool);
    friend QDebug operator<<(QDebug debug, const Tool& t);

public:
    Tool();
    Tool(int)
        : diameter_(0) { }

    enum Type {
        Drill,
        EndMill,
        Engraver,
        Laser,
        Group = 100
    };

    enum {
        Angle,
        Diameter,
        FeedRate,
        OneTurnCut,
        PassDepth,
        PlungeRate,
        SpindleSpeed,
        Stepover,
        OneTurnCutPercent,
        StepoverPercent
    };

    // name
    QString nameEnc() const;
    QString name() const;
    void setName(const QString& name);
    // note
    QString note() const;
    void setNote(const QString& note);
    // type
    Type type() const;
    void setType(int type);
    // angle
    double angle() const;
    void setAngle(double angle);
    // diameter
    double diameter() const;
    void setDiameter(double diameter);
    // feedRate
    double feedRate_mm_s() const;
    double feedRate() const;
    void setFeedRate(double feedRate);
    // oneTurnCut
    double oneTurnCut() const;
    void setOneTurnCut(double oneTurnCut);
    // passDepth
    double passDepth() const;
    void setPassDepth(double passDepth);
    // plungeRate
    double plungeRate() const;
    void setPlungeRate(double plungeRate);
    // spindleSpeed
    double spindleSpeed() const;
    void setSpindleSpeed(double spindleSpeed);
    // stepover
    double stepover() const;
    void setStepover(double stepover);
    // autoName
    bool autoName() const;
    void setAutoName(bool autoName);
    // id
    int id() const;
    void setId(int id);
    // depth_
    static double depth() { return depth_; }
    static void setDepth(double depth) { depth_ = depth; }

    double getDiameter(double depth) const;
    double getDepth() const;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid() const;
    QIcon icon() const;
    QString errorStr() const;
    void errorMessageBox(QWidget* parent = nullptr) const;
    size_t hash() const;
    size_t hash2() const;

    QPainterPath path(const QPointF& pt = {}) const;
    void updatePath(double depth = 0.0);

private:
    QString name_ {QObject::tr("Default")};
    QString note_;

    double angle_ {};
    double diameter_ {1};
    double feedRate_ {100};
    double oneTurnCut_ {0.1};
    double passDepth_ {2};
    double plungeRate_ {600};
    double spindleSpeed_ {12000};
    double stepover_ {0.5};
    static inline double depth_;

    int id_ {-1};

    mutable size_t hash_ = 0;
    mutable size_t hash2_ = 0;

    Type type_ {EndMill};

    QPainterPath path_;

    bool autoName_ {true};
};

using Tools = std::map<int, Tool, std::greater<int>>;

class ToolHolder {
    friend class ToolItem;
    friend class FilePlugin;

    Tools tools_;
    ToolHolder(const ToolHolder&) = delete;
    ToolHolder& operator=(const ToolHolder&) = delete;
    ToolHolder(ToolHolder&&) = delete;
    ToolHolder& operator=(ToolHolder&&) = delete;

public:
    ToolHolder();

    const Tool& tool(int id) { return tools_.at(id); }
    const Tools& tools() { return tools_; }
    void readTools();
    void readTools(const QJsonObject& json);
    void writeTools(QJsonObject& json);
};

Q_DECLARE_METATYPE(Tool)
