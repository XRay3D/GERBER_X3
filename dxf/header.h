#pragma once

#include <QMap>
#include <QVariant>

namespace Dxf {
struct Header {
    Header();
    QMap<QString, QMap<int, QVariant>> data;
};
}
