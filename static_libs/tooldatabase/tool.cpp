// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "tool.h"
#include "datastream.h"
#include "settings.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QtMath>
#include <app.h>

int toolId = qRegisterMetaType<Tool>("Tool");

QDataStream& operator<<(QDataStream& stream, const Tool& tool) {
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
QDataStream& operator>>(QDataStream& stream, Tool& tool) {
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

QDebug operator<<(QDebug debug, const Tool& t) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "T(D " << t.m_diameter << ", ID " << t.m_id << ')';
    return debug;
}

Tool::Tool() { }

QString Tool::nameEnc() const {
    switch (m_type) {
    case Tool::Drill:
        return QString("D-D%1MM").arg(m_diameter);
    case Tool::EndMill:
        return QString("M-D%1MM").arg(m_diameter);
    case Tool::Engraver:
        return QString("V-D%1MMA%2DEG").arg(m_diameter).arg(m_angle);
    case Tool::Laser:
        return QString("L-D%1MM").arg(m_diameter);
    default:
        return {};
    }
}
QString Tool::name() const { return m_name; }
void Tool::setName(const QString& name) { m_hash = {}, m_name = name; }

QString Tool::note() const { return m_note; }
void Tool::setNote(const QString& note) { m_hash = {}, m_note = note; }

Tool::Type Tool::type() const { return m_type; }
void Tool::setType(int type) { m_hash = {}, m_type = static_cast<Type>(type); }

double Tool::angle() const { return m_angle; }
void Tool::setAngle(double angle) { m_hash = {}, m_angle = angle; }

double Tool::diameter() const { return m_diameter; }
void Tool::setDiameter(double diameter) { m_hash = {}, m_diameter = diameter, updatePath(); }

double Tool::feedRateMmS() const { return m_feedRate / 60.0; }
double Tool::feedRate() const { return m_feedRate; }

void Tool::setFeedRate(double feedRate) { m_hash = {}, m_feedRate = feedRate; }
double Tool::oneTurnCut() const { return m_oneTurnCut; }

void Tool::setOneTurnCut(double oneTurnCut) { m_hash = {}, m_oneTurnCut = oneTurnCut; }
double Tool::passDepth() const { return m_passDepth; }

void Tool::setPassDepth(double passDepth) { m_hash = {}, m_passDepth = passDepth; }
double Tool::plungeRate() const { return m_plungeRate; }

void Tool::setPlungeRate(double plungeRate) { m_hash = {}, m_plungeRate = plungeRate; }
double Tool::spindleSpeed() const { return m_spindleSpeed; }

void Tool::setSpindleSpeed(double spindleSpeed) { m_hash = {}, m_spindleSpeed = spindleSpeed; }
double Tool::stepover() const { return m_stepover; }

void Tool::setStepover(double stepover) { m_hash = {}, m_stepover = stepover; }
bool Tool::autoName() const { return m_autoName; }

void Tool::setAutoName(bool autoName) { m_hash = {}, m_autoName = autoName; }
int Tool::id() const { return m_id; }

void Tool::setId(int id) { m_hash = {}, m_id = id; }
double Tool::getDiameter(double depth) const {
    if (type() == Engraver && depth > 0.0 && angle() > 0.0 && angle() <= 90.0) {
        double a = qDegreesToRadians(90 - angle() / 2);
        double d = depth * cos(a) / sin(a);
        return d * 2 + diameter();
    }
    return diameter();
}

double Tool::getDepth() const {
    switch (m_type) {
    case Tool::Drill:
        return m_diameter * 0.5 * tan(qDegreesToRadians((180.0 - m_angle) * 0.5));
    case Tool::EndMill:
    case Tool::Engraver:
    default:
        return 0.0;
    }
}

void Tool::read(const QJsonObject& json) {
    m_angle = json["angle"].toDouble();
    m_autoName = json["autoName"].toBool();
    m_diameter = json["diameter"].toDouble();
    m_feedRate = json["feedRate"].toDouble();
    m_id = json["id"].toInt();
    m_name = json["name"].toString();
    m_note = json["note"].toString();
    m_oneTurnCut = json["oneTurnCut"].toDouble();
    m_passDepth = json["passDepth"].toDouble();
    m_plungeRate = json["plungeRate"].toDouble();
    m_spindleSpeed = json["spindleSpeed"].toInt();
    m_stepover = json["stepover"].toDouble();
    m_type = static_cast<Type>(json["type"].toInt());
}

void Tool::write(QJsonObject& json) const {
    json["angle"] = m_angle;
    json["autoName"] = m_autoName;
    json["diameter"] = m_diameter;
    json["feedRate"] = m_feedRate;
    json["id"] = m_id;
    json["name"] = m_name;
    json["note"] = m_note;
    json["oneTurnCut"] = m_oneTurnCut;
    json["passDepth"] = m_passDepth;
    json["plungeRate"] = m_plungeRate;
    json["spindleSpeed"] = m_spindleSpeed;
    json["stepover"] = m_stepover;
    json["type"] = m_type;
}

bool Tool::isValid() const {
    do {
        if (qFuzzyIsNull(m_diameter))
            break;
        if (m_type != Laser && qFuzzyIsNull(m_passDepth))
            break;
        if (m_type != Drill && qFuzzyIsNull(m_feedRate))
            break;
        if (m_type != Drill && qFuzzyIsNull(m_stepover))
            break;
        if (m_type != Laser && qFuzzyIsNull(m_plungeRate))
            break;
        return true;
    } while (0);
    return false;
}

QIcon Tool::icon() const {
    switch (m_type) {
    case Tool::Drill:
        return QIcon::fromTheme("drill");
    case Tool::EndMill:
        return QIcon::fromTheme("endmill");
    case Tool::Engraver:
        return QIcon::fromTheme("engraving");
    case Tool::Laser:
        return QIcon::fromTheme("laser");
    default:
        return QIcon();
    }
}

QString Tool::errorStr() const {
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

void Tool::errorMessageBox(QWidget* parent) const {
    QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr());
}

size_t Tool::hash() const {
    if (m_hash)
        return m_hash;

    QByteArray hashData;
    hashData.push_back(m_name.toLocal8Bit());
    hashData.push_back(m_note.toLocal8Bit());
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append(reinterpret_cast<const char*>(&arg), sizeof(T));
    };
    push_back(m_type);
    push_back(m_angle);
    push_back(m_diameter);
    push_back(m_feedRate);
    push_back(m_oneTurnCut);
    push_back(m_passDepth);
    push_back(m_plungeRate);
    push_back(m_spindleSpeed);
    push_back(m_stepover);
    push_back(m_autoName);
    push_back(m_id);
    m_hash = qHash(hashData);

    return m_hash;
}

size_t Tool::hash2() const {
    if (!m_hash) {
        hash();
    } else
        return m_hash2;

    QByteArray hashData;
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append(reinterpret_cast<const char*>(&arg), sizeof(T));
    };
    push_back(m_angle);
    push_back(m_diameter);
    push_back(m_stepover);
    push_back(m_passDepth);
    m_hash2 = qHash(hashData);
    return m_hash2;
}

QPainterPath Tool::path(const QPointF& pt) const { return m_path.translated(pt); }

void Tool::updatePath(double depth) {
    const double diameter = getDiameter(depth);
    const double lineKoeff = diameter * 0.7;
    m_path = QPainterPath();
    m_path.addEllipse({}, diameter * 0.5, diameter * 0.5);
    m_path.moveTo(QPointF(0.0, +lineKoeff));
    m_path.lineTo(QPointF(0.0, -lineKoeff));
    m_path.moveTo(QPointF(+lineKoeff, 0.0));
    m_path.lineTo(QPointF(-lineKoeff, 0.0));
}

///////////////////////////////////////////////////////
/// \brief ToolHolder::tools
///
ToolHolder::ToolHolder() { }

void ToolHolder::readTools() {
    QJsonDocument loadDoc;

    QFile file(App::settingsPath() + QStringLiteral("/tools.json"));

    if (!file.exists())
        file.setFileName(qApp->applicationDirPath() + "/tools.json"); // fallback path
    if (file.exists() && file.open(QIODevice::ReadOnly))
        loadDoc = QJsonDocument::fromJson(file.readAll());
    else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        file.setFileName(qApp->applicationDirPath() + QStringLiteral("/tools.dat"));
        if (file.exists() && file.open(QIODevice::ReadOnly)) {
            loadDoc = QJsonDocument::fromBinaryData(file.readAll());
        } else
#endif
        {
            qDebug() << file.errorString();
            return;
        }
    }
    readTools(loadDoc.object());
}

void ToolHolder::readTools(const QJsonObject& json) {
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.setId(toolObject["id"].toInt());
        tool.updatePath();
        m_tools.emplace(tool.id(), tool);
    }
}

void ToolHolder::writeTools(QJsonObject& json) {
    QJsonArray toolArray;
    for (auto& [id, tool] : m_tools) {
        QJsonObject toolObject;
        tool.write(toolObject);
        toolObject["id"] = id;
        toolArray.push_back(toolObject);
    }
    json["tools"] = toolArray;
}
