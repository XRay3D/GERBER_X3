/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_graphicobject.h"
#include "dxf_entity.h"
#include "dxf_file.h"

namespace Dxf {

QDataStream& operator<<(QDataStream& stream, const DxfGo& go) {
    stream << (GraphicObject&)go;
    stream << go.entityId_;
    stream << go.rotationAngle_;
    stream << go.scaleX_;
    stream << go.scaleY_;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, DxfGo& go) {
    stream >> (GraphicObject&)go;
    stream >> go.entityId_;
    stream >> go.rotationAngle_;
    stream >> go.scaleX_;
    stream >> go.scaleY_;
    return stream;
}

size_t DxfGo::entityId() const { return entityId_; }

DxfGo::DxfGo(int entityId, const Path& path, const Paths& paths)
    : entityId_{entityId} {
    fill = paths;
    ::GraphicObject::path = path;
}

void DxfGo::setRotation(double rotationAngle) {
    rotationAngle_ = rotationAngle;
    RotatePath(path, rotationAngle_ /*, pos_*/);
    for(auto& path: fill)
        RotatePath(path, rotationAngle_ /*, pos_*/);
}

double DxfGo::rotationAngle() const { return rotationAngle_; }

void DxfGo::setScale(double scaleX, double scaleY) {
    scaleX_ = scaleX, scaleY_ = scaleY;
    auto scale = [](Path& path, double sx, double sy, const Point& center = Point{}) {
        const bool fl = Area(path) < 0;
        for(Point& pt: path) {
            const double dAangle = (pi * 2) - angleRadTo(center, pt);
            const double length = distTo(center, pt);
            pt = Point{cos(dAangle) * length * sx, sin(dAangle) * length * sy};
            pt.x += center.x;
            pt.y += center.y;
        }
        if(fl != (Area(path) < 0))
            ReversePath(path);
    };

    scale(path, scaleX_, scaleY_ /*, pos_*/);
    for(auto& path: fill)
        scale(path, scaleX_, scaleY_ /*, pos_*/);
}

std::tuple<double, double> DxfGo::scale() const { return {scaleX_, scaleY_}; }

double DxfGo::scaleX() const { return scaleX_; }

double DxfGo::scaleY() const { return scaleY_; }

void DxfGo::setPos(QPointF pos) {
    ::GraphicObject::pos = ~pos;
    TranslatePath(::GraphicObject::path, ~pos);
    for(auto& path: fill)
        TranslatePath(path, ::GraphicObject::pos);
}

QPointF DxfGo::pos() const { return ~::GraphicObject::pos; }

const Entity* DxfGo::entity() const { return file_ ? file_->entities().at(entityId_).get() : nullptr; }

// const File* GraphicObject::file() const { return gFile_; }

// const Path& GraphicObject::path() const { return path; }

// const Paths& GraphicObject::paths() const { return fill; }

// Path GraphicObject::line() const { return {}; }

// Path GraphicObject::lineW() const { return {}; }

// Path GraphicObject::polyLine() const { return {}; }

// Paths GraphicObject::polyLineW() const { return {}; }

// Path GraphicObject::elipse() const { return path; }

// Paths GraphicObject::elipseW() const { return fill; }

// Path GraphicObject::arc() const { return {}; }

// Path GraphicObject::arcW() const { return {}; }

// Path GraphicObject::polygon() const { return {}; }

// Paths GraphicObject::polygonWholes() const { return {}; }

// Path GraphicObject::hole() const { return {}; }

// Paths GraphicObject::holes() const { return {}; }

// bool GraphicObject::positive() const { return {}; }

// bool GraphicObject::closed() const { return {}; }

// Path& GraphicObject::rPath() { return path; }

// Paths& GraphicObject::rPaths() { return fill; }

} // namespace Dxf
