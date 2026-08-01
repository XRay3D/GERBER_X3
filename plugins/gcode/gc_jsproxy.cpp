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

#include <QJSEngine>

namespace GCode {

GcFileProxy::GcFileProxy(File* file, QJSEngine* engine, QObject* parent)
    : QObject{parent}
    , file_{file}
    , engine_{engine} {
}

bool GcFileProxy::laser() const { return file_->toolType == Tool::Laser; }
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
double GcFileProxy::safeZ() const { return App::project().safeZ(); }
double GcFileProxy::plunge() const { return App::project().plunge(); }
double GcFileProxy::clearance() const { return App::project().clearence(); }
int GcFileProxy::stepsX() const { return static_cast<int>(App::project().stepsX()); }
int GcFileProxy::stepsY() const { return static_cast<int>(App::project().stepsY()); }
double GcFileProxy::workWidth() const { return App::project().worckRect().width(); }
double GcFileProxy::workHeight() const { return App::project().worckRect().height(); }
double GcFileProxy::spaceX() const { return App::project().spaceX(); }
double GcFileProxy::spaceY() const { return App::project().spaceY(); }
double GcFileProxy::zVal() const { return file_->z_; }
void GcFileProxy::setZ(double z) { file_->z_ = z; }

QJSValue GcFileProxy::getToolPaths(double ox, double oy) {
    cachedPathss_ = file_->mirrorAndOffsetCurves({ox, oy});

    QJSValue outerArr = engine_->newArray(static_cast<uint>(cachedPathss_.size()));
    for(uint i{}; i < cachedPathss_.size(); ++i) {
        const Curves& paths = cachedPathss_[i];
        QJSValue pathsArr = engine_->newArray(static_cast<uint>(paths.size()));
        for(uint j{}; j < paths.size(); ++j) {
            const Curve& curve = paths[j];
            QJSValue curveArr = engine_->newArray(static_cast<uint>(curve.size()));
            curveArr.setProperty(u"closed"_s, curve.isClosed());
            curveArr.setProperty(u"perimetr"_s, curve.perimetr());
            for(uint k{}; k < curve.size(); ++k) {
                const geo::Vertex& v = curve[k];
                QJSValue vtx = engine_->newObject();
                vtx.setProperty(u"x"_s, v.pt.x());
                vtx.setProperty(u"y"_s, v.pt.y());
                vtx.setProperty(u"type"_s, int(v.type));
                vtx.setProperty(u"cx"_s, v.center.x());
                vtx.setProperty(u"cy"_s, v.center.y());
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

QJSValue GcFileProxy::savePathLines(int pi, int ci, bool rev, double perimetr, double depth) {
    if(pi < 0 || pi >= static_cast<int>(cachedPathss_.size())) return {};
    const Curves& paths = cachedPathss_[static_cast<size_t>(pi)];
    if(ci < 0 || ci >= static_cast<int>(paths.size())) return {};

    const Curve& orig = paths[static_cast<size_t>(ci)];
    auto lines = file_->savePath(rev ? orig.reversed() : orig, perimetr, depth);

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
    return file_->formated(data);
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
QString GcFileProxy::fmtF(double v) { return u'F' + File::format(v); }

} // namespace GCode
