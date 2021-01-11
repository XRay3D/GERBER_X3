/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
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
    // m_depth
    static double depth() { return m_depth; }
    static void setDepth(double depth) { m_depth = depth; }

    double getDiameter(double depth) const;
    double getDepth() const;

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
    bool isValid() const;
    QIcon icon() const;
    QString errorStr() const;
    void errorMessageBox(QWidget* parent = nullptr) const;
    size_t hash() const;

    QPainterPath path(const QPointF& pt = {}) const;
    void updatePath(double depth = 0.0);

private:
    QString m_name;
    QString m_note;

    double m_angle;
    double m_diameter;
    double m_feedRate;
    double m_oneTurnCut;
    double m_passDepth;
    double m_plungeRate;
    double m_spindleSpeed;
    double m_stepover;
    static inline double m_depth;

    uint m_id;

    mutable size_t m_hash = 0;

    Type m_type;

    QPainterPath m_path;

    bool m_autoName;
};

#if __cplusplus > 201709L
using Tools = std::map<int, Tool, std::greater<int>>;
#else
struct Tools : std::map<int, Tool, std::greater<int>> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif

class ToolHolder {
    friend class ToolItem;
    friend class FilePluginInterface;

    Tools m_tools;
    ToolHolder(const ToolHolder&) = delete;
    ToolHolder& operator=(const ToolHolder&) = delete;
    ToolHolder(ToolHolder&&) = delete;
    ToolHolder& operator=(ToolHolder&&) = delete;

public:
    ToolHolder();

    const Tool& tool(int id) { return m_tools.at(id); }
    const Tools& tools() { return m_tools; }
    void readTools();
    void readTools(const QJsonObject& json);
    void writeTools(QJsonObject& json);
};

Q_DECLARE_METATYPE(Tool)
