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
    void setName(const QString& val) { hash_ = {}, name_ = val; }
    // note
    QString note() const { return note_; }
    void setNote(const QString& val) { hash_ = {}, note_ = val; }
    // type
    Type type() const { return type_; }
    void setType(int val) { hash_ = {}, type_ = static_cast<Type>(val); }
    // angle
    double angle() const { return angle_; }
    void setAngle(double val) { hash_ = {}, angle_ = val; }
    // Thread Hole Diam
    double holeDiam() const { return angle_; }
    void setHoleDiam(double val) { hash_ = {}, angle_ = val; }
    // diameter
    double diameter() const { return diameter_; }
    void setDiameter(double val) { hash_ = {}, diameter_ = val, updatePath(); }
    // feedRate
    double feedRate_mmPerSec() const { return feedRate_ / 60.0; }
    double feedRate() const { return feedRate_; }
    void setFeedRate(double val) { hash_ = {}, feedRate_ = val; }
    // oneTurnCut
    double oneTurnCut() const { return oneTurnCut_; }
    void setOneTurnCut(double val) { hash_ = {}, oneTurnCut_ = val; }
    // passDepth
    double passDepth() const { return passDepth_; }
    void setPassDepth(double val) { hash_ = {}, passDepth_ = val; }
    // plungeRate
    double plungeRate() const { return plungeRate_; }
    void setPlungeRate(double val) { hash_ = {}, plungeRate_ = val; }
    // spindleSpeed
    double spindleSpeed() const { return spindleSpeed_; }
    void setSpindleSpeed(double val) { hash_ = {}, spindleSpeed_ = val; }
    // stepover
    double stepover() const { return stepover_; }
    void setStepover(double val) { hash_ = {}, stepover_ = val; }
    // autoName
    bool autoName() const { return autoName_; }
    void setAutoName(bool val) { hash_ = {}, autoName_ = val; }
    // lenght
    double lenght() const { return lenght_; }
    void setLenght(double val) { hash_ = {}, lenght_ = val; }
    // id
    int id() const { return id_; }
    void setId(int id) { hash_ = {}, id_ = id; }
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

    int32_t id_{-1};

    mutable size_t hash_{};
    mutable size_t hash2_{};

    Type type_{EndMill};

    QPainterPath path_;

    bool autoName_{true};
};

using Tools = std::map<int, Tool, std::greater<int>>;

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

    const Tool& tool(int32_t id) { return tools_.at(id); }
    const Tools& tools() { return tools_; }
    void readTools();
    void readTools(const QJsonObject& json);
    void writeTools(QJsonObject& json);
};

Q_DECLARE_METATYPE(Tool)
Q_DECLARE_METATYPE(Tool::Type)
