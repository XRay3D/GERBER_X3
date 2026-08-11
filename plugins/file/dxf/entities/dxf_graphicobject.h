/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

// #include "dxf_entity.h"
#include "plugintypes.h"

namespace Dxf {

class File;
struct Entity;

class DxfGo final : public ::GraphicObject {
    friend class File;
    friend class Plugin;

    int entityId_{};
    // Path path_;
    File* file_ = nullptr; // восстанавливает File::createGi
    double rotationAngle_{};
    double scaleX_{1.0};
    double scaleY_{1.0};
    // QPointF pos_;

public:
    DxfGo() = default;
    DxfGo(int entityId, Geo::Polyline&& path, Geo::Polygons&& paths = {});

    void setRotation(double rotationAngle);
    double rotationAngle() const;

    void setScale(double scaleX, double scaleY);
    std::tuple<double, double> scale() const;
    double scaleX() const;
    double scaleY() const;

    // void setPos(QPointF pos);
    // QPointF pos() const;

    const Entity* entity() const;
    size_t entityId() const;

#if 0
    // GraphicObject interface
    Path arc() const /*override*/;
    Path arcW() const /*override*/;
    Path elipse() const /*override*/;
    Path hole() const /*override*/;
    Path line() const /*override*/;
    Path lineW() const /*override*/;
    Path polyLine() const /*override*/;
    Path polygon() const /*override*/;
    Path& rPath() /*override*/;
    Paths elipseW() const /*override*/;
    Paths holes() const /*override*/;
    Paths polyLineW() const /*override*/;
    Paths polygonWholes() const /*override*/;
    Paths& rPaths() /*override*/;
    bool closed() const /*override*/;
    bool positive() const /*override*/;
    const Path& path() const /*override*/;
    const Paths& paths() const /*override*/;
#endif
};

} // namespace Dxf
