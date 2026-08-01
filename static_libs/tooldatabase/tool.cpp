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
        tool.data.id,
        tool.data.type,
        tool.data.angle,
        tool.data.autoName,
        tool.data.diameter,
        tool.data.feedRate,
        tool.data.name,
        tool.data.note,
        tool.data.oneTurnCut,
        tool.data.passDepth,
        tool.data.plungeRate,
        tool.data.spindleSpeed,
        tool.data.stepover,
        tool.data.lenght);
}

QDataStream& operator>>(QDataStream& stream, Tool& tool) {
    return Block{stream}.read(
        tool.data.id,
        tool.data.type,
        tool.data.angle,
        tool.data.autoName,
        tool.data.diameter,
        tool.data.feedRate,
        tool.data.name,
        tool.data.note,
        tool.data.oneTurnCut,
        tool.data.passDepth,
        tool.data.plungeRate,
        tool.data.spindleSpeed,
        tool.data.stepover,
        tool.data.lenght);
}

QDebug operator<<(QDebug debug, const Tool& t) {
    QDebugStateSaver saver{debug};
    debug.nospace() << u"Tool(D "_s << t.data.diameter << u", ID "_s << +t.data.id << u", Ty "_s << t.data.type << ')';
    return debug;
}

QString Tool::nameEnc() const {
    switch(data.type) {
    case Tool::Drill     : return u"D-D%1MM"_s.arg(data.diameter);
    case Tool::EndMill   : return u"M-D%1MM"_s.arg(data.diameter);
    case Tool::Engraver  : return u"V-D%1MMA%2DEG"_s.arg(data.diameter).arg(data.angle);
    case Tool::Laser     : return u"L-D%1MM"_s.arg(data.diameter);
    case Tool::ThreadMill: return u"T-D%1MM"_s.arg(data.diameter);
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
    switch(data.type) {
    case Tool::Drill   : return data.diameter * 0.5 * tan(qDegreesToRadians((180.0 - data.angle) * 0.5));
    case Tool::EndMill :
    case Tool::Engraver:
    default            : return 0.0;
    }
}

void Tool::read(const QJsonObject& json) {
    for_each_field(data, [&json]<typename T>(T& field, std::string_view name) {
        field = json[QLatin1StringView{name}].toVariant().value<T>();
    });
}

void Tool::write(QJsonObject& json) const {
    for_each_field(data, [&json]<typename T>(const T& field, std::string_view name) {
        if constexpr(std::is_enum_v<T>)
            json[QLatin1StringView{name}] = static_cast<qint64>(field);
        else
            json[QLatin1StringView{name}] = field;
    });
}

bool Tool::isValid() const {
    do {
        if(qFuzzyIsNull(data.diameter)) break;
        if(data.type != Laser && qFuzzyIsNull(data.passDepth)) break;
        if(data.type != Drill && qFuzzyIsNull(data.feedRate)) break;
        if(data.type != Drill && qFuzzyIsNull(data.stepover)) break;
        if(data.type != Laser && qFuzzyIsNull(data.plungeRate)) break;
        return true;
    } while(0);
    return false;
}

QIcon Tool::icon() const {
    switch(data.type) {
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
    if(qFuzzyIsNull(data.diameter))
        errorString += u"Tool diameter = 0!\n"_s;
    if(qFuzzyIsNull(data.passDepth)) {
        if(type() == Drill)
            errorString += u"Pass = 0!\n"_s;
        else
            errorString += u"Depth = 0!\n"_s;
    }
    if(qFuzzyIsNull(data.feedRate))
        errorString += u"Feed rate = 0\n"_s;
    if(qFuzzyIsNull(data.stepover))
        errorString += u"Stepover = 0\n"_s;
    if(qFuzzyIsNull(data.plungeRate))
        errorString += u"Plunge rate = 0!\n"_s;
    return errorString;
}

void Tool::errorMessageBox(QWidget* parent) const {
    QMessageBox::warning(parent, QObject::tr("No valid tool...!!!"), errorStr());
}

size_t Tool::hash() const {
    if(hash_) return hash_;
    std::vector<char> hashData;
    hashData.append_range(data.name.toUtf8());
    hashData.append_range(data.note.toUtf8());
    auto push_back = [&hashData](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        hashData.append_range(std::bit_cast<std::array<char, sizeof(T)>>(arg));
    };
    push_back(data.type);
    push_back(data.angle);
    push_back(data.diameter);
    push_back(data.feedRate);
    push_back(data.oneTurnCut);
    push_back(data.passDepth);
    push_back(data.plungeRate);
    push_back(data.spindleSpeed);
    push_back(data.stepover);
    push_back(data.autoName);
    push_back(data.id);
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
    push_back(data.angle);
    push_back(data.diameter);
    push_back(data.stepover);
    push_back(data.passDepth);
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
