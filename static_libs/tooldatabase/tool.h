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

#include "utils.h"
#include <QObject>
#include <QPainterPath>

class QWidget;
class QIcon;
class QDataStream;

class Tool {
    friend QDataStream& operator<<(QDataStream& stream, const Tool& tool);
    friend QDataStream& operator>>(QDataStream& stream, Tool& tool);
    friend QDebug operator<<(QDebug debug, const Tool& t);

public:
    Tool() = default;
    Tool(double diameter)
        : diameter_{diameter} { }

    enum class ID : int32_t {
        // Folder = -1,
        Null   = 0,
        Folder = 0,
        Tool   = +1
    };

    enum Type {
        Drill,
        EndMill,
        Engraver,
        Laser,
        ThreadMill,
        Group = 100
    };

    // name
    QString nameEnc() const;
    QString name() const { return name_; }
    void setName(const QString& name) { hash_ = {}, name_ = name; }
    // note
    QString note() const { return note_; }
    void setNote(const QString& note) { hash_ = {}, note_ = note; }
    // type
    Type type() const;
    void setType(int type) { hash_ = {}, type_ = static_cast<Type>(type); }
    // angle
    double angle() const { return angle_; }
    void setAngle(double angle) { hash_ = {}, angle_ = angle; }
    // diameter
    double diameter() const { return diameter_; }
    void setDiameter(double diameter) { hash_ = {}, diameter_ = diameter, updatePath(); }
    // feedRate
    double feedRate_mmPerSec() const { return feedRate_ / 60.0; }
    double feedRate() const { return feedRate_; }
    void setFeedRate(double feedRate) { hash_ = {}, feedRate_ = feedRate; }
    // oneTurnCut
    double oneTurnCut() const { return oneTurnCut_; }
    void setOneTurnCut(double oneTurnCut) { hash_ = {}, oneTurnCut_ = oneTurnCut; }
    // passDepth
    double passDepth() const { return passDepth_; }
    void setPassDepth(double passDepth) { hash_ = {}, passDepth_ = passDepth; }
    // plungeRate
    double plungeRate() const { return plungeRate_; }
    void setPlungeRate(double plungeRate) { hash_ = {}, plungeRate_ = plungeRate; }
    // spindleSpeed
    double spindleSpeed() const { return spindleSpeed_; }
    void setSpindleSpeed(double spindleSpeed) { hash_ = {}, spindleSpeed_ = spindleSpeed; }
    // stepover
    double stepover() const { return stepover_; }
    void setStepover(double stepover) { hash_ = {}, stepover_ = stepover; }
    // autoName
    bool autoName() const { return autoName_; }
    void setAutoName(bool autoName) { hash_ = {}, autoName_ = autoName; }
    // lenght
    double lenght() const { return lenght_; }
    void setLenght(double lenght) { hash_ = {}, lenght_ = lenght; }
    // id
    ID id() const { return id_; }
    void setId(ID id) { hash_ = {}, id_ = id; }
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
    QString name_{QObject::tr("Default")};
    QString note_;

    double angle_{.0};
    double diameter_{1.};
    double feedRate_{100.};
    double oneTurnCut_{0.1};
    double passDepth_{2.}; // max thread pitch
    double plungeRate_{600.};
    double spindleSpeed_{12000.};
    double stepover_{0.5};
    double lenght_{1.}; //

    static inline double depth_;

    ID id_{ID::Null};

    mutable size_t hash_{};
    mutable size_t hash2_{};

    Type type_{EndMill};

    QPainterPath path_;

    bool autoName_{true};
};

using Tools = std::map<Tool::ID, Tool, std::greater<Tool::ID>>;

class ToolHolder {
    friend class ToolItem;
    friend class AbstractFilePlugin;

    Tools tools_;
    ToolHolder(const ToolHolder&)            = delete;
    ToolHolder& operator=(const ToolHolder&) = delete;
    ToolHolder(ToolHolder&&)                 = delete;
    ToolHolder& operator=(ToolHolder&&)      = delete;

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
