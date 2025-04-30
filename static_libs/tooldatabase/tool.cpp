/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
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
    return Block{stream}.write(
        tool.id_,
        tool.type_,
        tool.angle_,
        tool.autoName_,
        tool.diameter_,
        tool.feedRate_,
        tool.name_,
        tool.note_,
        tool.oneTurnCut_,
        tool.passDepth_,
        tool.plungeRate_,
        tool.spindleSpeed_,
        tool.stepover_,
        tool.lenght_);
}

QDataStream& operator>>(QDataStream& stream, Tool& tool) {
    return Block{stream}.read(
        tool.id_,
        tool.type_,
        tool.angle_,
        tool.autoName_,
        tool.diameter_,
        tool.feedRate_,
        tool.name_,
        tool.note_,
        tool.oneTurnCut_,
        tool.passDepth_,
        tool.plungeRate_,
        tool.spindleSpeed_,
        tool.stepover_,
        tool.lenght_);
}

QDebug operator<<(QDebug debug, const Tool& t) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Tool(D " << t.diameter_ << ", ID " << t.id_ << ')';
    return debug;
}

QString Tool::nameEnc() const {
    switch(type_) {
    case Tool::Drill:
        return QString("D-D%1MM").arg(diameter_);
    case Tool::EndMill:
        return QString("M-D%1MM").arg(diameter_);
    case Tool::Engraver:
        return QString("V-D%1MMA%2DEG").arg(diameter_).arg(angle_);
    case Tool::Laser:
        return QString("L-D%1MM").arg(diameter_);
    case Tool::ThreadMill:
        return QString("T-D%1MM").arg(diameter_);
    default:
        return {};
    }
}
QString Tool::name() const { return name_; }
void Tool::setName(const QString& name) { hash_ = {}, name_ = name; }

QString Tool::note() const { return note_; }
void Tool::setNote(const QString& note) { hash_ = {}, note_ = note; }

Tool::Type Tool::type() const { return type_; }
void Tool::setType(int type) { hash_ = {}, type_ = static_cast<Type>(type); }

double Tool::angle() const { return angle_; }
void Tool::setAngle(double angle) { hash_ = {}, angle_ = angle; }

double Tool::diameter() const { return diameter_; }
void Tool::setDiameter(double diameter) { hash_ = {}, diameter_ = diameter, updatePath(); }

double Tool::feedRate_mmPerSec() const { return feedRate_ / 60.0; }
double Tool::feedRate() const { return feedRate_; }
void Tool::setFeedRate(double feedRate) { hash_ = {}, feedRate_ = feedRate; }

double Tool::oneTurnCut() const { return oneTurnCut_; }
void Tool::setOneTurnCut(double oneTurnCut) { hash_ = {}, oneTurnCut_ = oneTurnCut; }

double Tool::passDepth() const { return passDepth_; }
void Tool::setPassDepth(double passDepth) { hash_ = {}, passDepth_ = passDepth; }

double Tool::plungeRate() const { return plungeRate_; }
void Tool::setPlungeRate(double plungeRate) { hash_ = {}, plungeRate_ = plungeRate; }

double Tool::spindleSpeed() const { return spindleSpeed_; }
void Tool::setSpindleSpeed(double spindleSpeed) { hash_ = {}, spindleSpeed_ = spindleSpeed; }

double Tool::stepover() const { return stepover_; }
void Tool::setStepover(double stepover) { hash_ = {}, stepover_ = stepover; }

bool Tool::autoName() const { return autoName_; }
void Tool::setAutoName(bool autoName) { hash_ = {}, autoName_ = autoName; }

double Tool::lenght() const { return lenght_; }
void Tool::setLenght(double lenght) { hash_ = {}, lenght_ = lenght; }

int Tool::id() const { return id_; }

void Tool::setId(int32_t id) { hash_ = {}, id_ = id; }
double Tool::getDiameter(double depth) const {
    if(type() == Engraver && depth > 0.0 && angle() > 0.0 && angle() <= 90.0) {
        double a = qDegreesToRadians(90 - angle() / 2);
        double d = depth * cos(a) / sin(a);
        return d * 2 + diameter();
    }
    return diameter();
}

double Tool::getDepth() const {
    switch(type_) {
    case Tool::Drill:
        return diameter_ * 0.5 * tan(qDegreesToRadians((180.0 - angle_) * 0.5));
    case Tool::EndMill:
    case Tool::Engraver:
    default:
        return 0.0;
    }
}

void Tool::read(const QJsonObject& json) {
    angle_ = json["angle"].toDouble();
    autoName_ = json["autoName"].toBool();
    diameter_ = json["diameter"].toDouble();
    feedRate_ = json["feedRate"].toDouble();
    id_ = json["id"].toInt();
    name_ = json["name"].toString();
    note_ = json["note"].toString();
    oneTurnCut_ = json["oneTurnCut"].toDouble();
    passDepth_ = json["passDepth"].toDouble();
    plungeRate_ = json["plungeRate"].toDouble();
    spindleSpeed_ = json["spindleSpeed"].toInt();
    stepover_ = json["stepover"].toDouble();
    lenght_ = json["lenght"].toDouble(10);

    type_ = static_cast<Type>(json["type"].toInt());
}

void Tool::write(QJsonObject& json) const {
    json["angle"] = angle_;
    json["autoName"] = autoName_;
    json["diameter"] = diameter_;
    json["feedRate"] = feedRate_;
    json["id"] = id_;
    json["name"] = name_;
    json["note"] = note_;
    json["oneTurnCut"] = oneTurnCut_;
    json["passDepth"] = passDepth_;
    json["plungeRate"] = plungeRate_;
    json["spindleSpeed"] = spindleSpeed_;
    json["stepover"] = stepover_;
    json["type"] = type_;
    json["lenght"] = lenght_;
}

bool Tool::isValid() const {
    do {
        if(qFuzzyIsNull(diameter_))
            break;
        if(type_ != Laser && qFuzzyIsNull(passDepth_))
            break;
        if(type_ != Drill && qFuzzyIsNull(feedRate_))
            break;
        if(type_ != Drill && qFuzzyIsNull(stepover_))
            break;
        if(type_ != Laser && qFuzzyIsNull(plungeRate_))
            break;
        return true;
    } while(0);
    return false;
}

QIcon Tool::icon() const {
    switch(type_) {
    case Tool::Drill:
        return QIcon::fromTheme("drill");
    case Tool::EndMill:
        return QIcon::fromTheme("endmill");
    case Tool::Engraver:
        return QIcon::fromTheme("engraving");
    case Tool::Laser:
        return QIcon::fromTheme("laser");
    case Tool::ThreadMill:
        return QIcon::fromTheme("thread_mill");
    default:
        return QIcon();
    }
}

QString Tool::errorStr() const {
    QString errorString;
    if(qFuzzyIsNull(diameter_))
        errorString += "Tool diameter = 0!\n";
    if(qFuzzyIsNull(passDepth_)) {
        if(type() == Drill)
            errorString += "Pass = 0!\n";
        else
            errorString += "Depth = 0!\n";
    }
    if(qFuzzyIsNull(feedRate_))
        errorString += "Feed rate = 0\n";
    if(qFuzzyIsNull(stepover_))
        errorString += "Stepover = 0\n";
    if(qFuzzyIsNull(plungeRate_))
        errorString += "Plunge rate = 0!\n";
    return errorString;
}

void Tool::errorMessageBox(QWidget* parent) const {
    QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr());
}

size_t Tool::hash() const {
    if(hash_)
        return hash_;

    QByteArray hashData;
    hashData.push_back(name_.toUtf8());
    hashData.push_back(note_.toUtf8());
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append(reinterpret_cast<const char*>(&arg), sizeof(T));
    };
    push_back(type_);
    push_back(angle_);
    push_back(diameter_);
    push_back(feedRate_);
    push_back(oneTurnCut_);
    push_back(passDepth_);
    push_back(plungeRate_);
    push_back(spindleSpeed_);
    push_back(stepover_);
    push_back(autoName_);
    push_back(id_);
    hash_ = qHash(hashData);

    return hash_;
}

size_t Tool::hash2() const {
    if(!hash_)
        hash();
    else
        return hash2_;

    QByteArray hashData;
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append(reinterpret_cast<const char*>(&arg), sizeof(T));
    };
    push_back(angle_);
    push_back(diameter_);
    push_back(stepover_);
    push_back(passDepth_);
    hash2_ = qHash(hashData);
    return hash2_;
}

QPainterPath Tool::path(const QPointF& pt) const { return path_.translated(pt); }

void Tool::updatePath(double depth) {
    const double diameter = getDiameter(depth);
    const double lineKoeff = diameter * 0.7;
    path_ = QPainterPath();
    path_.addEllipse({}, diameter * 0.5, diameter * 0.5);
    path_.moveTo(QPointF(0.0, +lineKoeff));
    path_.lineTo(QPointF(0.0, -lineKoeff));
    path_.moveTo(QPointF(+lineKoeff, 0.0));
    path_.lineTo(QPointF(-lineKoeff, 0.0));
}

///////////////////////////////////////////////////////
/// \brief ToolHolder::tools
///
void ToolHolder::readTools() {
    QJsonDocument loadDoc;

    QFile file(App::settingsPath() + u"/tools.json"_s);

    if(!file.exists())
        file.setFileName(qApp->applicationDirPath() + "/tools.json"); // fallback path
    if(file.exists() && file.open(QIODevice::ReadOnly))
        loadDoc = QJsonDocument::fromJson(file.readAll());
    else {
        qDebug() << file.errorString();
        return;
    }
    readTools(loadDoc.object());
}

void ToolHolder::readTools(const QJsonObject& json) {
    QJsonArray toolArray = json["tools"].toArray();
    for(int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.setId(toolObject["id"].toInt());
        tool.updatePath();
        tools_.try_emplace(tool.id(), tool);
    }
}

void ToolHolder::writeTools(QJsonObject& json) {
    QJsonArray toolArray;
    for(auto& [id, tool]: tools_) {
        QJsonObject toolObject;
        tool.write(toolObject);
        toolObject["id"] = id;
        toolArray.push_back(toolObject);
    }
    json["tools"] = QJsonValue{toolArray};
}
