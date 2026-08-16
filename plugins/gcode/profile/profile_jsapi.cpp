/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "profile_jsapi.h"
#include "profile.h"
#include "profile_bridges.h"

#include "gc_jsproxy.h"

#include <QJSEngine>

namespace Profile {

BridgesApi::BridgesApi(File* file, GCode::GcFileProxy* proxy)
    : QObject{}
    , file_{file}
    , proxy_{proxy} { }

bool BridgesApi::hasBridges() const { return file_->hasBridges(); }
double BridgesApi::tabTop() const { return file_->bridgeTabTop(); }

int BridgesApi::add(Geo::Polyline&& path) {
    pool_.push_back(std::move(path));
    return static_cast<int>(pool_.size()) - 1;
}

const Geo::Polygons& BridgesApi::region() {
    const QPointF offset = proxy_->offset();
    if(!regionOffset_ || *regionOffset_ != offset) {
        regionOffset_ = offset;
        region_ = file_->bridgeRegion(offset);
    }
    return region_;
}

QJSValue BridgesApi::chain(int pathssIdx, int pathIdx) {
    QJSEngine& engine = *proxy_->engine();
    const auto& pathss = proxy_->cachedPathss();
    if(pathssIdx < 0 || pathssIdx >= static_cast<int>(pathss.size())) return engine.newArray(0);
    const auto& paths = pathss[static_cast<size_t>(pathssIdx)];
    if(pathIdx < 0 || pathIdx >= static_cast<int>(paths.size())) return engine.newArray(0);

    std::vector<Piece> pieces = chainPieces(paths[static_cast<size_t>(pathIdx)], region());
    QJSValue arr = engine.newArray(static_cast<uint>(pieces.size()));
    for(uint i{}; i < pieces.size(); ++i) {
        QJSValue obj = engine.newObject();
        obj.setProperty(u"bridge"_s, pieces[i].bridge);
        obj.setProperty(u"perimeter"_s, pieces[i].path.perimeter());
        obj.setProperty(u"id"_s, add(std::move(pieces[i].path)));
        arr.setProperty(i, obj);
    }
    return arr;
}

QJSValue BridgesApi::split(int id) {
    QJSEngine& engine = *proxy_->engine();
    QJSValue obj = engine.newObject();
    if(id < 0 || id >= static_cast<int>(pool_.size())) return obj;
    auto [up, flat, down] = splitBridge(pool_[static_cast<size_t>(id)], file_->params().getToolDiameter());
    obj.setProperty(u"up"_s, add(std::move(up)));
    obj.setProperty(u"flat"_s, flat.empty() ? -1 : add(std::move(flat)));
    obj.setProperty(u"down"_s, add(std::move(down)));
    return obj;
}

int BridgesApi::reverse(int id) {
    if(id < 0 || id >= static_cast<int>(pool_.size())) return -1;
    return add(pool_[static_cast<size_t>(id)].reversed());
}

double BridgesApi::perimeter(int id) const {
    if(id < 0 || id >= static_cast<int>(pool_.size())) return 0.0;
    return pool_[static_cast<size_t>(id)].perimeter();
}

QJSValue BridgesApi::lines(int id, double perimeter, double depth) {
    if(id < 0 || id >= static_cast<int>(pool_.size())) return proxy_->engine()->newArray(0);
    return proxy_->linesFor(pool_[static_cast<size_t>(id)], perimeter, depth);
}

} // namespace Profile
