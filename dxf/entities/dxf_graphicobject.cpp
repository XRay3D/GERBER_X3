#include "dxf_graphicobject.h"
namespace Dxf {
GraphicObject::GraphicObject(const File* file, const Entity* entity, const Path& path, const Paths& paths)
    : m_gFile(file)
    , m_entity(entity)
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

void GraphicObject::setScale(double scaleX, double scaleY)
{
    m_scaleX = scaleX, m_scaleY = scaleY;
    auto scale = [](Path& path, double sx, double sy, const Point64& center = {}) {
        const bool fl = Area(path) < 0;
        for (Point64& pt : path) {
            const double dAangle = (M_PI * 2) - center.angleRadTo(pt);
            const double length = center.distTo(pt);
            pt = Point64(static_cast<cInt>(cos(dAangle) * length * sx), static_cast<cInt>(sin(dAangle) * length * sy));
            pt.X += center.X;
            pt.Y += center.Y;
        }
        if (fl != (Area(path) < 0))
            ReversePath(path);
    };

    scale(m_path, m_scaleX, m_scaleY, m_pos);
    for (auto& path : m_paths)
        scale(path, m_scaleX, m_scaleY, m_pos);
}

void GraphicObject::setPos(QPointF pos)
{
    m_pos = pos;
    TranslatePath(m_path, m_pos);
    for (auto& path : m_paths)
        TranslatePath(path, m_pos);
}

}
