#pragma once

#include "myclipper.h"

namespace Dxf {

class File;
struct Entity;

class GraphicObject {
    friend class File;
    friend class Parser;

    friend QDataStream& operator<<(QDataStream& stream, const GraphicObject& go)
    {
        stream << go.m_path;
        stream << go.m_paths;
        //stream << go.m_entity;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, GraphicObject& go)
    {
        stream >> go.m_path;
        stream >> go.m_paths;
        //stream >> go.m_entity;
        return stream;
    }

    const File* const m_gFile = nullptr;
    const Entity* const m_entity = nullptr;
    Path m_path;
    Paths m_paths;

    double m_rotationAngle = 0.0;
    double m_scaleX = 0.0;
    double m_scaleY = 0.0;
    QPointF m_pos;

public:
    GraphicObject(
        const File* file,
        const Entity* entity,
        const Path& path,
        const Paths& paths)
        : m_gFile(file)
        , m_entity(entity)
        , m_path(path)
        , m_paths(paths)
    {
    }
    void inline setRotation(double rotationAngle) { m_rotationAngle = rotationAngle; }
    double rotationAngle() const { return m_rotationAngle; }

    void inline setScale(double scaleX, double scaleY) { m_scaleX = scaleX, m_scaleY = scaleY; }
    double scaleX() { return m_scaleX; }
    double scaleY() { return m_scaleY; }

    void inline setPos(QPointF pos) { m_pos = pos; }
    QPointF pos() { return m_pos; }

    inline const File* file() const { return m_gFile; }
    inline const Entity* entity() const { return m_entity; }
    inline const Path& path() const { return m_path; }
    inline const Paths& paths() const { return m_paths; }
};

}
