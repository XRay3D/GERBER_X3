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
#include "gc_jsproxy.h"
#include "gc_file.h"

#include "app.h"
#include "project.h"

#include "serial.h"
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>

namespace GCode {

GcFileProxy::GcFileProxy(File* file, QJSEngine* engine, QObject* parent)
    : QObject{parent}
    , file_{file}
    , engine_{engine} {
}

QJSValue GcFileProxy::toolObject() const {
    // Tool::write обходит поля рефлексией (см. tool.cpp), ключи -- имена самих
    // членов. Наружу уходит снимок: писать в него из скрипта смысла нет.
    QJsonObject json;
    file_->gcp.tool().write(json);
    return engine_->toScriptValue(json.toVariantMap());
}

bool GcFileProxy::laser() const { return file_->toolType == Tool::Laser; }
bool GcFileProxy::spiralRamp() const { return file_->gcp.spiralRamp(); }
double GcFileProxy::toolDiameter() const { return file_->gcp.getToolDiameter(); }
double GcFileProxy::toolLength() const { return file_->gcp.tool().lenght(); }
double GcFileProxy::toolOneTurnCut() const { return file_->gcp.tool().oneTurnCut(); }
double GcFileProxy::feedRate() const { return file_->feedRate; }
double GcFileProxy::plungeRate() const { return file_->plungeRate; }
int GcFileProxy::spindleSpeed() const { return file_->spindleSpeed; }
QString GcFileProxy::strFeed() const { return file_->strFeed; }
QString GcFileProxy::strPlungeFeed() const { return file_->strPlungeFeed; }
QString GcFileProxy::strSpindle() const { return file_->strSpindle; }
bool GcFileProxy::inside() const { return !file_->gcp.params.contains(GCode::Params::Side) || file_->gcp.side() != GCode::Outer; }
bool GcFileProxy::climb() const { return !file_->gcp.params.contains(GCode::Params::Convent) || !file_->gcp.convent(); }
bool GcFileProxy::leftHand() const { return file_->gcp.leftHand(); }
double GcFileProxy::threadPitch() const { return file_->gcp.tool().passDepth(); }
double GcFileProxy::threadHoleDiam() const { return file_->gcp.tool().holeDiam(); }
bool GcFileProxy::circle() const { return file_->gcp.circle(); }
bool GcFileProxy::chamfer() const { return file_->gcp.chamfer(); }
int GcFileProxy::starts() const { return file_->gcp.starts(); }
QJSValue GcFileProxy::properties() const { return propertiesValue(*engine_); }
bool GcFileProxy::notTile() const { return file_->gcp.params.contains(Params::NotTile); }
QString GcFileProxy::name() const { return file_->shortName(); }
QString GcFileProxy::programName() const { return file_->programName_; }

namespace {

// Serial пишет QPointF массивом [x, y], QRectF -- [x, y, w, h]. Скрипту
// удобнее объекты с именованными полями.
QVariant asPoint(const QVariant& v) {
    const auto list = v.toList();
    if(list.size() != 2) return v;
    return QVariantMap{
        {u"x"_s, list[0]},
        {u"y"_s, list[1]}
    };
}
QVariant asRect(const QVariant& v) {
    const auto list = v.toList();
    if(list.size() != 4) return v;
    return QVariantMap{
        {u"x"_s,      list[0]},
        {u"y"_s,      list[1]},
        {u"width"_s,  list[2]},
        {u"height"_s, list[3]}
    };
}

} // namespace

QJSValue GcFileProxy::propertiesValue(QJSEngine& engine) {
    Serial::Writer sb;
    Serial::write(sb, App::project().header());
    std::string_view view;
    if(auto err = sb.view().get(view); err) {
        qWarning() << "GCode JS: project header:" << simdjson::error_message(err);
        return engine.newObject();
    }
    QVariantMap map = QJsonDocument::fromJson(QByteArray{view.data(), static_cast<qsizetype>(view.size())}).object().toVariantMap();
    for(const QString& key: {u"home"_s, u"zero"_s})
        if(map.contains(key)) map[key] = asPoint(map[key]);
    for(const QString& key: {u"workRect"_s, u"viewRect"_s})
        if(map.contains(key)) map[key] = asRect(map[key]);
    if(map.contains(u"pins"_s)) {
        QVariantList pins = map[u"pins"_s].toList();
        for(QVariant& pin: pins) pin = asPoint(pin);
        map[u"pins"_s] = pins;
    }
    return engine.toScriptValue(map);
}
double GcFileProxy::zVal() const { return file_->z_; }
void GcFileProxy::setZ(double z) { file_->z_ = z; }

QJSValue GcFileProxy::getToolPaths(double ox, double oy) {
    offset_ = {ox, oy};
    cachedPathss_ = file_->mirrorAndOffsetCurves(offset_);

    QJSValue outerArr = engine_->newArray(static_cast<uint>(cachedPathss_.size()));
    for(uint i{}; i < cachedPathss_.size(); ++i) {
        const Geo::Polylines& paths = cachedPathss_[i];
        QJSValue pathsArr = engine_->newArray(static_cast<uint>(paths.size()));
        for(uint j{}; j < paths.size(); ++j) {
            const Geo::Polyline& curve = paths[j];
            QJSValue curveArr = engine_->newArray(static_cast<uint>(curve.size()));
            curveArr.setProperty(u"closed"_s, curve.isClosed());
            curveArr.setProperty(u"perimeter"_s, curve.perimeter());
            for(uint k{}; k < curve.size(); ++k) {
                const Geo::Vertex& v = curve[k];
                QJSValue vtx = engine_->newObject();
                vtx.setProperty(u"x"_s, v.x());
                vtx.setProperty(u"y"_s, v.y());
                vtx.setProperty(u"bulge"_s, v.bulge);
                // type/cx/cy описывают сегмент, ПРИХОДЯЩИЙ в вершину -- так их
                // и видели скрипты, когда дуга хранилась центром на конечной
                // вершине; менять контракт из-за смены внутреннего вида нельзя
                auto arc = k ? Geo::arcOf(curve[k - 1], curve[k], curve[k - 1].bulge) : std::nullopt;
                vtx.setProperty(u"type"_s, arc ? int(arc->dir()) : 0);
                vtx.setProperty(u"cx"_s, arc ? arc->center.x() : 0.0);
                vtx.setProperty(u"cy"_s, arc ? arc->center.y() : 0.0);
                curveArr.setProperty(k, vtx);
            }
            pathsArr.setProperty(j, curveArr);
        }
        outerArr.setProperty(i, pathsArr);
    }
    return outerArr;
}

QJSValue GcFileProxy::getDepths() {
    auto depths = file_->getDepths();
    QJSValue arr = engine_->newArray(static_cast<uint>(depths.size()));
    for(uint i{}; i < depths.size(); ++i)
        arr.setProperty(i, depths[i]);
    return arr;
}

void GcFileProxy::startPath(double x, double y) {
    file_->startPath({x, y});
}

void GcFileProxy::endPath() {
    file_->endPath();
}

void GcFileProxy::addLine(const QString& line) {
    file_->lines_.emplace_back(line);
}

QString GcFileProxy::comment(const QString& text) { return File::comment(text); }
QString GcFileProxy::spindleOn() { return App::gcSettings().post().spindleOn(file_->postContext()); }
QString GcFileProxy::spindleOff() { return App::gcSettings().post().spindleOff(file_->postContext()); }
QString GcFileProxy::laserOn(bool dynamic) { return App::gcSettings().post().laserOn(file_->postContext(), dynamic); }
QString GcFileProxy::laserOff() { return App::gcSettings().post().laserOff(file_->postContext()); }

QJSValue GcFileProxy::savePathLines(int pi, int ci, bool rev, double perimeter, double depth) {
    if(pi < 0 || pi >= static_cast<int>(cachedPathss_.size())) return {};
    const Geo::Polylines& paths = cachedPathss_[static_cast<size_t>(pi)];
    if(ci < 0 || ci >= static_cast<int>(paths.size())) return {};

    const Geo::Polyline& orig = paths[static_cast<size_t>(ci)];
    return linesFor(rev ? orig.reversed() : orig, perimeter, depth);
}

QJSValue GcFileProxy::linesFor(const Geo::Polyline& curve, double perimeter, double depth) {
    auto lines = file_->savePath(curve, perimeter, depth);
    QJSValue arr = engine_->newArray(static_cast<uint>(lines.size()));
    for(uint i{}; i < lines.size(); ++i)
        arr.setProperty(i, lines[i]);
    return arr;
}

QString GcFileProxy::formatted(const QJSValue& parts) {
    std::vector<QString> data;
    const uint len = parts.property(u"length"_s).toUInt();
    data.reserve(len);
    for(uint i{}; i < len; ++i)
        data.emplace_back(parts.property(i).toString());
    return file_->formatedText(data);
}

QString GcFileProxy::g0() { return file_->g0(); }
QString GcFileProxy::g1() { return file_->g1(); }
QString GcFileProxy::g2() { return file_->g2(); }
QString GcFileProxy::g3() { return file_->g3(); }

QString GcFileProxy::fmtX(double v) { return File::x(v); }
QString GcFileProxy::fmtY(double v) { return File::y(v); }
QString GcFileProxy::fmtZ(double v) { return File::z(v); }
QString GcFileProxy::fmtI(double v) { return File::i(v); }
QString GcFileProxy::fmtJ(double v) { return File::j(v); }
QString GcFileProxy::fmtS(int v) { return File::speed(static_cast<double>(v)); }
QString GcFileProxy::fmtF(double v) { return u'F' + File::formatValue(v); }

} // namespace GCode
