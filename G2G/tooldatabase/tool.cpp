#include "tool.h"
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <qmath.h>
//#include <QCryptographicHash>
#include <QHash>

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

uint Tool::hash() const
{
    if (m_hash)
        return m_hash;

    QByteArray hashData;
    hashData.append(m_name.toLocal8Bit());
    hashData.append(m_note.toLocal8Bit());
    hashData.append(reinterpret_cast<const char*>(&m_type), sizeof(Type));
    hashData.append(reinterpret_cast<const char*>(&m_angle), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_diameter), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_feedRate), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_oneTurnCut), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_passDepth), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_plungeRate), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_spindleSpeed), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_stepover), sizeof(double));
    hashData.append(reinterpret_cast<const char*>(&m_autoName), sizeof(bool));
    hashData.append(reinterpret_cast<const char*>(&m_id), sizeof(int));
    m_hash = qHash(hashData);

    //    QCryptographicHash calcHash(QCryptographicHash::Sha1);
    //    calcHash.addData(m_name.toLocal8Bit());
    //    calcHash.addData(m_note.toLocal8Bit());
    //    calcHash.addData(reinterpret_cast<const char*>(&m_type), sizeof(Type));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_angle), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_diameter), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_feedRate), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_oneTurnCut), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_passDepth), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_plungeRate), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_spindleSpeed), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_stepover), sizeof(double));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_autoName), sizeof(bool));
    //    calcHash.addData(reinterpret_cast<const char*>(&m_id), sizeof(int));
    //    m_hash = *reinterpret_cast<int*>(calcHash.result().data());
    //    qDebug() << calcHash.result() << m_hash << qHash(calcHash.result());

    //        if (m_hash)
    //            return m_hash;
    //        for (const QChar& h : m_name) {
    //            m_hash ^= h.digitValue();
    //        }
    //        for (const QChar& h : m_note) {
    //            m_hash ^= h.digitValue();
    //        }
    //        m_hash ^= m_type;
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_angle);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_diameter);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_feedRate);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_oneTurnCut);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_passDepth);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_plungeRate);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_spindleSpeed);
    //        m_hash ^= *reinterpret_cast<const uint64_t*>(&m_stepover);
    //        m_hash ^= m_id;
    return m_hash;
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
