/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "serial.h"
#include "utils.h"
#include <QObject>
#include <QPainterPath>

class QWidget;
class QIcon;

class Tool {
    friend QDebug operator<<(QDebug debug, const Tool& t);

public:
    enum class ID : int32_t {
        // Folder = -1,
        Null = 0,
        Folder = 0,
        Tool = +1
    };

    enum Type {
        Drill,
        EndMill,
        Engraver,
        Laser,
        ThreadMill,
        Group = 100
    };

private:
    struct {
        QString name{QObject::tr("Default")};
        QString note;
        double angle{.0};
        double diameter{1.};
        double feedRate{600.};
        double oneTurnCut{0.1};
        double passDepth{2.}; // max thread pitch
        double plungeRate{100.};
        double spindleSpeed{12000.};
        double stepover{0.5};
        double lenght{1.}; //
        ID id{ID::Null};
        Type type{EndMill};
        bool autoName{true};
    } data;

public:
    Tool() = default;
    Tool(double diameter) { data.diameter = diameter; }

    // name
    QString nameEnc() const;
    QString name() const { return data.name; }
    void setName(const QString& name) { hash_ = {}, data.name = name; }
    // note
    QString note() const { return data.note; }
    void setNote(const QString& note) { hash_ = {}, data.note = note; }
    // type
    Type type() const { return data.type; }
    void setType(int type) { hash_ = {}, data.type = static_cast<Type>(type); }
    // angle
    double angle() const { return data.angle; }
    void setAngle(double angle) { hash_ = {}, data.angle = angle; }
    // diameter
    double diameter() const { return data.diameter; }
    void setDiameter(double diameter) { hash_ = {}, data.diameter = diameter, updatePath(); }
    // feedRate
    double feedRate_mmPerSec() const { return data.feedRate / 60.0; }
    double feedRate() const { return data.feedRate; }
    void setFeedRate(double feedRate) { hash_ = {}, data.feedRate = feedRate; }
    // oneTurnCut
    double oneTurnCut() const { return data.oneTurnCut; }
    void setOneTurnCut(double oneTurnCut) { hash_ = {}, data.oneTurnCut = oneTurnCut; }
    // passDepth
    double passDepth() const { return data.passDepth; }
    void setPassDepth(double passDepth) { hash_ = {}, data.passDepth = passDepth; }
    // plungeRate
    double plungeRate() const { return data.plungeRate; }
    void setPlungeRate(double plungeRate) { hash_ = {}, data.plungeRate = plungeRate; }
    // spindleSpeed
    double spindleSpeed() const { return data.spindleSpeed; }
    void setSpindleSpeed(double spindleSpeed) { hash_ = {}, data.spindleSpeed = spindleSpeed; }
    // stepover
    double stepover() const { return data.stepover; }
    void setStepover(double stepover) { hash_ = {}, data.stepover = stepover; }
    // autoName
    bool autoName() const { return data.autoName; }
    void setAutoName(bool autoName) { hash_ = {}, data.autoName = autoName; }
    // lenght
    double lenght() const { return data.lenght; }
    void setLenght(double lenght) { hash_ = {}, data.lenght = lenght; }
    // Thread Hole Diam
    double holeDiam() const { return data.angle; }
    void setHoleDiam(double val) { hash_ = {}, data.angle = val; }
    // ID
    ID id() const { return data.id; }
    void setId(ID id) { hash_ = {}, data.id = id; }

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
    static inline double depth_;

    [[= Serial::skip]] mutable size_t hash_{};
    [[= Serial::skip]] mutable size_t hash2_{};

    [[= Serial::skip]] QPainterPath path_;
};

using Tools = std::map<Tool::ID, Tool, std::greater<Tool::ID>>;

class ToolHolder {
    friend class ToolItem;
    friend class AbstractFilePlugin;

    Tools tools_;
    ToolHolder(const ToolHolder&) = delete;
    ToolHolder& operator=(const ToolHolder&) = delete;
    ToolHolder(ToolHolder&&) = delete;
    ToolHolder& operator=(ToolHolder&&) = delete;

public:
    ToolHolder() = default;

    const Tool& tool(Tool::ID id) { return tools_.at(id); }
    const Tools& tools() { return tools_; }
    void readTools();
    void readTools(const QJsonObject& json);
    void writeTools(QJsonObject& json);
};

Q_DECLARE_METATYPE(Tool)
Q_DECLARE_METATYPE(Tool::Type)
Q_DECLARE_METATYPE(Tool::ID)
