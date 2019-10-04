#include "tool.h"
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <qmath.h>

int toolId = qRegisterMetaType<Tool>("Tool");

Tool::Tool()
    : m_name("Name")
    , m_type(EndMill)
    , m_angle(0.0)
    , m_diameter(1.0)
    , m_feedRate(1200.0)
    , m_oneTurnCut(0.1)
    , m_passDepth(2.0)
    , m_plungeRate(600.0)
    , m_spindleSpeed(12000.0)
    , m_stepover(0.5)
    , m_autoName(true)
    , m_id(0)
{
}

double Tool::getDiameter(double depth) const
{
    if (type() == Engraving && depth > 0.0 && angle() > 0.0 && angle() < 90.0) {
        double a = qDegreesToRadians(90 - angle() / 2);
        double d = depth * cos(a) / sin(a);
        return d * 2 + diameter();
    }
    return diameter();
}

double Tool::getDepth() const
{
    switch (m_type) {
    case Tool::Drill:
        return m_diameter * 0.5 * tan(qDegreesToRadians((180.0 - m_angle) * 0.5));
    case Tool::EndMill:
    case Tool::Engraving:
    default:
        return 0.0;
    }
}

void Tool::read(const QJsonObject& json)
{
    m_angle = json["angle"].toDouble();
    m_diameter = json["diameter"].toDouble();
    m_feedRate = json["feedRate"].toDouble();
    m_oneTurnCut = json["oneTurnCut"].toDouble();
    m_passDepth = json["passDepth"].toDouble();
    m_plungeRate = json["plungeRate"].toDouble();
    m_spindleSpeed = json["spindleSpeed"].toInt();
    m_stepover = json["stepover"].toDouble();
    m_name = json["name"].toString();
    m_note = json["note"].toString();
    m_type = static_cast<Type>(json["type"].toInt());
    m_autoName = json["autoName"].toBool();
}

void Tool::write(QJsonObject& json) const
{
    json["angle"] = m_angle;
    json["diameter"] = m_diameter;
    json["feedRate"] = m_feedRate;
    json["oneTurnCut"] = m_oneTurnCut;
    json["passDepth"] = m_passDepth;
    json["plungeRate"] = m_plungeRate;
    json["spindleSpeed"] = m_spindleSpeed;
    json["stepover"] = m_stepover;
    json["name"] = m_name;
    json["note"] = m_note;
    json["type"] = m_type;
    json["autoName"] = m_autoName;
}

bool Tool::isValid()
{
    do {
        if (qFuzzyIsNull(m_diameter))
            break;
        if (qFuzzyIsNull(m_passDepth))
            break;
        if (type() != Drill && qFuzzyIsNull(m_feedRate))
            break;
        if (type() != Drill && qFuzzyIsNull(m_stepover))
            break;
        if (qFuzzyIsNull(m_plungeRate))
            break;
        return true;
    } while (0);
    return false;
}

QIcon Tool::icon() const
{
    switch (m_type) {
    case Tool::Drill:
        return QIcon::fromTheme("drill");
    case Tool::EndMill:
        return QIcon::fromTheme("endmill");
    case Tool::Engraving:
        return QIcon::fromTheme("engraving");
    default:
        return QIcon();
    }
}

QString Tool::errorStr()
{
    QString errorString;
    if (qFuzzyIsNull(m_diameter))
        errorString += "Tool diameter = 0!\n";
    if (qFuzzyIsNull(m_passDepth)) {
        if (type() == Drill)
            errorString += "Pass = 0!\n";
        else
            errorString += "Depth = 0!\n";
    }
    if (qFuzzyIsNull(m_feedRate))
        errorString += "Feed rate = 0\n";
    if (qFuzzyIsNull(m_stepover))
        errorString += "Stepover = 0\n";
    if (qFuzzyIsNull(m_plungeRate))
        errorString += "Plunge rate = 0!\n";
    return errorString;
}

///////////////////////////////////////////////////////
/// \brief ToolHolder::tools
///
QMap<int, Tool> ToolHolder::tools;

void ToolHolder::readTools()
{
    QFile loadFile(QStringLiteral("../tools.dat"));
    do {
        if (loadFile.open(QIODevice::ReadOnly))
            break;
        loadFile.setFileName(QStringLiteral("tools.dat"));
        if (loadFile.open(QIODevice::ReadOnly))
            break;
        return;
    } while (1);

    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(loadFile.readAll()));
    QJsonArray toolArray = loadDoc.object()["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.setId(toolObject["id"].toInt());
        tools[tool.id()] = tool;
    }
}

void ToolHolder::readTools(const QJsonObject& json)
{
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.setId(toolObject["id"].toInt());
        tools[tool.id()] = tool;
    }
}

void ToolHolder::writeTools(QJsonObject& json)
{
    QJsonArray toolArray;
    QMap<int, Tool>::iterator i = tools.begin();
    while (i != tools.constEnd()) {
        QJsonObject toolObject;
        i.value().write(toolObject);
        toolObject["id"] = i.key();
        toolArray.append(toolObject);
        ++i;
    }
    json["tools"] = toolArray;
}
