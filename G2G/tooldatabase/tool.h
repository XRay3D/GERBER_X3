#ifndef TOOL_H
#define TOOL_H

#include "icons.h"
#include <QJsonObject>
#include <QMap>
#include <QMessageBox>
#include <QObject>

class Tool {
    friend QDataStream& operator<<(QDataStream& stream, const Tool& tool)
    {
        stream << tool.m_name;
        stream << tool.m_note;
        stream << tool.m_type;
        stream << tool.m_angle;
        stream << tool.m_diameter;
        stream << tool.m_feedRate;
        stream << tool.m_oneTurnCut;
        stream << tool.m_passDepth;
        stream << tool.m_plungeRate;
        stream << tool.m_spindleSpeed;
        stream << tool.m_stepover;
        stream << tool.m_autoName;
        stream << tool.m_id;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Tool& tool)
    {
        stream >> tool.m_name;
        stream >> tool.m_note;
        stream >> (int&)(tool.m_type);
        stream >> tool.m_angle;
        stream >> tool.m_diameter;
        stream >> tool.m_feedRate;
        stream >> tool.m_oneTurnCut;
        stream >> tool.m_passDepth;
        stream >> tool.m_plungeRate;
        stream >> tool.m_spindleSpeed;
        stream >> tool.m_stepover;
        stream >> tool.m_autoName;
        stream >> tool.m_id;
        return stream;
    }

public:
    Tool();

    enum Type {
        Drill,
        EndMill,
        Engraving,
        Group
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
        StepoverPercent,
    };

    // name
    inline QString name() const { return m_name; }
    inline void setName(const QString& name) { m_name = name; }
    // note
    inline QString note() const { return m_note; }
    inline void setNote(const QString& note) { m_note = note; }
    // type
    inline Type type() const { return m_type; }
    inline void setType(int type) { m_type = static_cast<Type>(type); }
    // angle
    inline double angle() const { return m_angle; }
    inline void setAngle(double angle) { m_angle = angle; }
    // diameter
    inline double diameter() const { return m_diameter; }
    inline void setDiameter(double diameter) { m_diameter = diameter; }
    // feedRate
    inline double feedRate() const { return m_feedRate; }
    inline void setFeedRate(double feedRate) { m_feedRate = feedRate; }
    // oneTurnCut
    inline double oneTurnCut() const { return m_oneTurnCut; }
    inline void setOneTurnCut(double oneTurnCut) { m_oneTurnCut = oneTurnCut; }
    // passDepth
    inline double passDepth() const { return m_passDepth; }
    inline void setPassDepth(double passDepth) { m_passDepth = passDepth; }
    // plungeRate
    inline double plungeRate() const { return m_plungeRate; }
    inline void setPlungeRate(double plungeRate) { m_plungeRate = plungeRate; }
    // spindleSpeed
    inline double spindleSpeed() const { return m_spindleSpeed; }
    inline void setSpindleSpeed(double spindleSpeed) { m_spindleSpeed = spindleSpeed; }
    // stepover
    inline double stepover() const { return m_stepover; }
    inline void setStepover(double stepover) { m_stepover = stepover; }
    // autoName
    inline bool autoName() const { return m_autoName; }
    inline void setAutoName(bool autoName) { m_autoName = autoName; }
    // id
    inline int id() const { return m_id; }
    inline void setId(int id) { m_id = id; }

    double getDiameter(double depth) const;
    double getDepth() const;
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid();
    QIcon icon() const;
    QString errorStr();
    void errorMessageBox(QWidget* parent = nullptr) { QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr()); }

private:
    QString m_name;
    QString m_note;
    Type m_type;
    double m_angle;
    double m_diameter;
    double m_feedRate;
    double m_oneTurnCut;
    double m_passDepth;
    double m_plungeRate;
    double m_spindleSpeed;
    double m_stepover;
    bool m_autoName;
    int m_id;
};

class ToolHolder {
public:
    ToolHolder();

    static void readTools();
    static void readTools(const QJsonObject& json);
    static void writeTools(QJsonObject& json);
    static QMap<int, Tool> tools;
};

Q_DECLARE_METATYPE(Tool)

#endif // TOOL_H
