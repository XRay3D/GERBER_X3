#include "app.h"
#include <QJsonObject>

#pragma once

class PluginData {

public:
    PluginData() { App app; }
    const QJsonObject& info() const { return info_; }
    void setInfo(const QJsonObject& info) { info_ = info; }

protected:
    QJsonObject info_;
};
