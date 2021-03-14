// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_graphicobject.h"
#include "dxf_entity.h"
#include "dxf_file.h"

namespace Dxf {

QDataStream& operator<<(QDataStream& stream, const GraphicObject& go)
{
    stream << go.m_path;
    stream << go.m_paths;
    stream << go.m_entityId;

    stream << go.m_rotationAngle;
    stream << go.m_scaleX;
    stream << go.m_scaleY;
    stream << go.m_pos;

    return stream;
}

QDataStream& operator>>(QDataStream& stream, GraphicObject& go)
{
    stream >> go.m_path;
    stream >> go.m_paths;
    stream >> go.m_entityId;

    stream >> go.m_rotationAngle;
    stream >> go.m_scaleX;
    stream >> go.m_scaleY;
    stream >> go.m_pos;

    return stream;
}

size_t GraphicObject::entityId() const
{
    return m_entityId;
}

GraphicObject::GraphicObject() { }

GraphicObject::GraphicObject(int entityId, const Path& path, const Paths& paths)
    : m_entityId(entityId)
    , m_path(path)
    , m_paths(paths)
{
}

void GraphicObject::setRotation(double rotationAngle)
{
    m_rotationAngle = rotationAngle;
    RotatePath(m_path, m_rotationAngle /*, m_pos*/);
    for (auto& path : m_paths)
        RotatePath(path, m_rotationAngle /*, m_pos*/);
}

double GraphicObject::rotationAngle() const { return m_rotationAngle; }

void GraphicObject::setScale(double scaleX, double scaleY)
{
    m_scaleX = scaleX, m_scaleY = scaleY;
    auto scale = [](Path& path, double sx, double sy, const IntPoint& center = {}) {
        const bool fl = Area(path) < 0;
        for (IntPoint& pt : path) {
            const double dAangle = (M_PI * 2) - center.angleRadTo(pt);
            const double length = center.distTo(pt);
            pt = IntPoint(static_cast<cInt>(cos(dAangle) * length * sx), static_cast<cInt>(sin(dAangle) * length * sy));
            pt.X += center.X;
            pt.Y += center.Y;
        }
        if (fl != (Area(path) < 0))
            ReversePath(path);
    };

    scale(m_path, m_scaleX, m_scaleY, {} /*m_pos*/);
    for (auto& path : m_paths)
        scale(path, m_scaleX, m_scaleY, {} /*m_pos*/);
}

std::tuple<double, double> GraphicObject::scale() const { return { m_scaleX, m_scaleY }; }

double GraphicObject::scaleX() const { return m_scaleX; }

double GraphicObject::scaleY() const { return m_scaleY; }

void GraphicObject::setPos(QPointF pos)
{
    m_pos = pos;
    TranslatePath(m_path, m_pos);
    for (auto& path : m_paths)
        TranslatePath(path, m_pos);
}

QPointF GraphicObject::pos() const { return m_pos; }

const Entity* GraphicObject::entity() const { return m_file ? m_file->entities().at(m_entityId).get() : nullptr; }

//const File* GraphicObject::file() const { return m_gFile; }

const Path& GraphicObject::path() const { return m_path; }

const Paths& GraphicObject::paths() const { return m_paths; }

Path GraphicObject::line() const { return {}; }

Path GraphicObject::lineW() const { return {}; }

Path GraphicObject::polyLine() const { return {}; }

Paths GraphicObject::polyLineW() const { return {}; }

Path GraphicObject::elipse() const { return m_path; }

Paths GraphicObject::elipseW() const { return m_paths; }

Path GraphicObject::arc() const { return {}; }

Path GraphicObject::arcW() const { return {}; }

Path GraphicObject::polygon() const { return {}; }

Paths GraphicObject::polygonWholes() const { return {}; }

Path GraphicObject::hole() const { return {}; }

Paths GraphicObject::holes() const { return {}; }

bool GraphicObject::positive() const { return {}; }

bool GraphicObject::closed() const { return {}; }

Path& GraphicObject::rPath() { return m_path; }

Paths& GraphicObject::rPaths() { return m_paths; }

}
