/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QObject>

class QWidget;
class QIcon;

class Tool {
    friend QDataStream& operator<<(QDataStream& stream, const Tool& tool);
    friend QDataStream& operator>>(QDataStream& stream, Tool& tool);
    friend QDebug operator<<(QDebug debug, const Tool& t);

public:
    Tool();
    Tool(int);

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
    QString name() const;
    QString nameEnc() const;
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
    double feedRateMmS() const;
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

    double getDiameter(double depth) const;
    double getDepth() const;
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid() const;
    QIcon icon() const;
    QString errorStr() const;
    void errorMessageBox(QWidget* parent = nullptr) const;
    uint hash() const;

private:
    mutable int m_hash = 0;
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
    uint m_id;
};

class ToolHolder {
    static QMap<int, Tool> m_tools;
    friend class ToolItem;

public:
    static const Tool& tool(int id);
    static const QMap<int, Tool> &tools();
    static void readTools();
    static void readTools(const QJsonObject& json);
    static void writeTools(QJsonObject& json);
};

Q_DECLARE_METATYPE(Tool)
