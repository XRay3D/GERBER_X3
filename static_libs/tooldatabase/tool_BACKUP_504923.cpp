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
    QDebugStateSaver saver{debug};
    debug.nospace() << u"Tool(D "_s << t.diameter_ << u", ID "_s << +t.id_ << u", Ty "_s << t.type_ << ')';
    return debug;
}

QString Tool::nameEnc() const {
    switch(type_) {
    case Tool::Drill     : return u"D-D%1MM"_s.arg(diameter_);
    case Tool::EndMill   : return u"M-D%1MM"_s.arg(diameter_);
    case Tool::Engraver  : return u"V-D%1MMA%2DEG"_s.arg(diameter_).arg(angle_);
    case Tool::Laser     : return u"L-D%1MM"_s.arg(diameter_);
    case Tool::ThreadMill: return u"T-D%1MM"_s.arg(diameter_);
    default              : return {};
    }
}

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
    case Tool::Drill   : return diameter_ * 0.5 * tan(qDegreesToRadians((180.0 - angle_) * 0.5));
    case Tool::EndMill :
    case Tool::Engraver:
    default            : return 0.0;
    }
}

void Tool::read(const QJsonObject& json) {
    angle_ = json[u"angle"_s].toDouble();
    autoName_ = json[u"autoName"_s].toBool();
    diameter_ = json[u"diameter"_s].toDouble();
    feedRate_ = json[u"feedRate"_s].toDouble();
    id_ = static_cast<ID>(json[u"id"_s].toInt());
    name_ = json[u"name"_s].toString();
    note_ = json[u"note"_s].toString();
    oneTurnCut_ = json[u"oneTurnCut"_s].toDouble();
    passDepth_ = json[u"passDepth"_s].toDouble();
    plungeRate_ = json[u"plungeRate"_s].toDouble();
    spindleSpeed_ = json[u"spindleSpeed"_s].toInt();
    stepover_ = json[u"stepover"_s].toDouble();
    lenght_ = json[u"lenght"_s].toDouble(10);

    type_ = static_cast<Type>(json[u"type"_s].toInt());
}

void Tool::write(QJsonObject& json) const {
    json[u"angle"_s] = angle_;
    json[u"autoName"_s] = autoName_;
    json[u"diameter"_s] = diameter_;
    json[u"feedRate"_s] = feedRate_;
    json[u"id"_s] = +id_;
    json[u"name"_s] = name_;
    json[u"note"_s] = note_;
    json[u"oneTurnCut"_s] = oneTurnCut_;
    json[u"passDepth"_s] = passDepth_;
    json[u"plungeRate"_s] = plungeRate_;
    json[u"spindleSpeed"_s] = spindleSpeed_;
    json[u"stepover"_s] = stepover_;
    json[u"type"_s] = type_;
    json[u"lenght"_s] = lenght_;
}

bool Tool::isValid() const {
    do {
        if(qFuzzyIsNull(diameter_)) break;
        if(type_ != Laser && qFuzzyIsNull(passDepth_)) break;
        if(type_ != Drill && qFuzzyIsNull(feedRate_)) break;
        if(type_ != Drill && qFuzzyIsNull(stepover_)) break;
        if(type_ != Laser && qFuzzyIsNull(plungeRate_)) break;
        return true;
    } while(0);
    return false;
}

QIcon Tool::icon() const {
    switch(type_) {
    case Tool::Drill     : return QIcon::fromTheme(u"drill"_s);
    case Tool::EndMill   : return QIcon::fromTheme(u"endmill"_s);
    case Tool::Engraver  : return QIcon::fromTheme(u"engraving"_s);
    case Tool::Laser     : return QIcon::fromTheme(u"laser"_s);
    case Tool::ThreadMill: return QIcon::fromTheme(u"thread_mill"_s);
    default              : return QIcon();
    }
}

QString Tool::errorStr() const {
    QString errorString;
    if(qFuzzyIsNull(diameter_))
        errorString += u"Tool diameter = 0!\n"_s;
    if(qFuzzyIsNull(passDepth_)) {
        if(type() == Drill)
            errorString += u"Pass = 0!\n"_s;
        else
            errorString += u"Depth = 0!\n"_s;
    }
    if(qFuzzyIsNull(feedRate_))
        errorString += u"Feed rate = 0\n"_s;
    if(qFuzzyIsNull(stepover_))
        errorString += u"Stepover = 0\n"_s;
    if(qFuzzyIsNull(plungeRate_))
        errorString += u"Plunge rate = 0!\n"_s;
    return errorString;
}

void Tool::errorMessageBox(QWidget* parent) const {
    QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr());
}

size_t Tool::hash() const {
    if(hash_) return hash_;
    std::vector<char> hashData;
    hashData.append_range(name_.toUtf8());
    hashData.append_range(note_.toUtf8());
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append_range(std::bit_cast<std::array<char, sizeof(T)>>(arg));
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
    return hash_ = qHash(hashData);
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
        file.setFileName(qApp->applicationDirPath() + u"/tools.json"_s); // fallback path
    if(file.exists() && file.open(QIODevice::ReadOnly))
        loadDoc = QJsonDocument::fromJson(file.readAll());
    else {
        qDebug() << file.errorString();
        return;
    }
    readTools(loadDoc.object());
}

void ToolHolder::readTools(const QJsonObject& json) {
    QJsonArray toolArray = json[u"tools"_s].toArray();
    for(int treeIndex{}; treeIndex < toolArray.size(); ++treeIndex) {
        Tool tool;
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool.read(toolObject);
        tool.setId(static_cast<Tool::ID>(toolObject[u"id"_s].toInt()));
        tool.updatePath();
        tools_.try_emplace(tool.id(), tool);
    }
}

void ToolHolder::writeTools(QJsonObject& json) {
    QJsonArray toolArray;
    for(auto& [id, tool]: tools_) {
        QJsonObject toolObject;
        tool.write(toolObject);
        toolObject[u"id"_s] = +id;
        toolArray.push_back(toolObject);
    }
    json[u"tools"_s] = QJsonValue{toolArray};
}
