#include "tool.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <qmath.h>
//#include <QCryptographicHash>
#include <QHash>

int toolId = qRegisterMetaType<Tool>("Tool");

QDataStream& operator<<(QDataStream& stream, const Tool& tool)
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
QDataStream& operator>>(QDataStream& stream, Tool& tool)
{
    stream >> tool.m_name;
    stream >> tool.m_note;
    stream >> tool.m_type;
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

Tool::Tool()
    : m_name(QObject::tr("Default"))
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

QString Tool::name() const { return m_name; }

void Tool::setName(const QString& name)
{
    m_hash = 0;
    m_name = name;
}

QString Tool::note() const { return m_note; }

void Tool::setNote(const QString& note)
{
    m_hash = 0;
    m_note = note;
}

Tool::Type Tool::type() const { return m_type; }

void Tool::setType(int type)
{
    m_hash = 0;
    m_type = static_cast<Type>(type);
}

double Tool::angle() const { return m_angle; }

void Tool::setAngle(double angle)
{
    m_hash = 0;
    m_angle = angle;
}

double Tool::diameter() const { return m_diameter; }

void Tool::setDiameter(double diameter)
{
    m_hash = 0;
    m_diameter = diameter;
}

double Tool::feedRate() const { return m_feedRate; }

void Tool::setFeedRate(double feedRate)
{
    m_hash = 0;
    m_feedRate = feedRate;
}

double Tool::oneTurnCut() const { return m_oneTurnCut; }

void Tool::setOneTurnCut(double oneTurnCut)
{
    m_hash = 0;
    m_oneTurnCut = oneTurnCut;
}

double Tool::passDepth() const { return m_passDepth; }

void Tool::setPassDepth(double passDepth)
{
    m_hash = 0;
    m_passDepth = passDepth;
}

double Tool::plungeRate() const { return m_plungeRate; }

void Tool::setPlungeRate(double plungeRate)
{
    m_hash = 0;
    m_plungeRate = plungeRate;
}

double Tool::spindleSpeed() const { return m_spindleSpeed; }

void Tool::setSpindleSpeed(double spindleSpeed)
{
    m_hash = 0;
    m_spindleSpeed = spindleSpeed;
}

double Tool::stepover() const { return m_stepover; }

void Tool::setStepover(double stepover)
{
    m_hash = 0;
    m_stepover = stepover;
}

bool Tool::autoName() const { return m_autoName; }

void Tool::setAutoName(bool autoName)
{
    m_hash = 0;
    m_autoName = autoName;
}

int Tool::id() const { return m_id; }

void Tool::setId(int id)
{
    m_hash = 0;
    m_id = id;
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

void Tool::errorMessageBox(QWidget* parent) { QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr()); }

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
    QFile loadFile(qApp->applicationDirPath() + QStringLiteral("/tools.dat"));
    if (!loadFile.open(QIODevice::ReadOnly))
        return;

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
