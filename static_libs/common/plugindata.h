#include "app.h"
#include <QJsonObject>

#pragma once

class PluginData {

public:
    PluginData() = default;
    const QJsonObject& info() const { return info_; }
    void setInfo(const QJsonObject& info) { info_ = info; }

    QString name() const { return info_[u"Name"_s].toString(); }

protected:
    QJsonObject info_;
};
