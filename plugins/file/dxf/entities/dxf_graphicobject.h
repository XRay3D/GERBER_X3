/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "datastream.h"
//#include "dxf_entity.h"
#include "plugintypes.h"
#include <myclipper.h>

namespace Dxf {

class File;
struct Entity;

class GraphicObject final : public AbstrGraphicObject {
    friend class File;
    friend class Plugin;

    friend QDataStream& operator<<(QDataStream& stream, const GraphicObject& go);
    friend QDataStream& operator>>(QDataStream& stream, GraphicObject& go);

    int m_entityId {};
    Path m_path;
    Paths m_paths;
    File* m_file = nullptr;
    double m_rotationAngle {};
    double m_scaleX {};
    double m_scaleY {};
    QPointF m_pos;

public:
    GraphicObject();
    GraphicObject(int entityId, const Path& path, const Paths& paths);

    void setRotation(double rotationAngle);
    double rotationAngle() const;

    void setScale(double scaleX, double scaleY);
    std::tuple<double, double> scale() const;
    double scaleX() const;
    double scaleY() const;

    void setPos(QPointF pos);
    QPointF pos() const;

    const Entity* entity() const;
    size_t entityId() const;

    // AbstrGraphicObject interface

    const Path& path() const override;
    const Paths& paths() const override;

    Path line() const override;
    Path lineW() const override;
    Path polyLine() const override;
    Paths polyLineW() const override;
    Path elipse() const override;
    Paths elipseW() const override;
    Path arc() const override;
    Path arcW() const override;
    Path polygon() const override;
    Paths polygonWholes() const override;
    Path hole() const override;
    Paths holes() const override;
    bool positive() const override;
    bool closed() const override;
    Path& rPath() override;
    Paths& rPaths() override;
};

} // namespace Dxf
