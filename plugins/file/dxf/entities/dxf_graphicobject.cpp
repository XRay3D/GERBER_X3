// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_graphicobject.h"
#include "dxf_entity.h"
#include "dxf_file.h"

namespace Dxf {

QDataStream& operator<<(QDataStream& stream, const GraphicObject& go) {
    stream << go.path_;
    stream << go.paths_;
    stream << go.entityId_;

    stream << go.rotationAngle_;
    stream << go.scaleX_;
    stream << go.scaleY_;
    stream << go.pos_;

    return stream;
}

QDataStream& operator>>(QDataStream& stream, GraphicObject& go) {
    stream >> go.path_;
    stream >> go.paths_;
    stream >> go.entityId_;

    stream >> go.rotationAngle_;
    stream >> go.scaleX_;
    stream >> go.scaleY_;
    stream >> go.pos_;

    return stream;
}

size_t GraphicObject::entityId() const { return entityId_; }

GraphicObject::GraphicObject()
    : AbstrGraphicObject {{}} { }

GraphicObject::GraphicObject(int entityId, const Path& path, const Paths& paths)
    : AbstrGraphicObject(paths)
    , entityId_(entityId)
    , path_(path) {
}

void GraphicObject::setRotation(double rotationAngle) {
    rotationAngle_ = rotationAngle;
    RotatePath(path_, rotationAngle_ /*, pos_*/);
    for (auto& path : paths_)
        RotatePath(path, rotationAngle_ /*, pos_*/);
}

double GraphicObject::rotationAngle() const { return rotationAngle_; }

void GraphicObject::setScale(double scaleX, double scaleY) {
    scaleX_ = scaleX, scaleY_ = scaleY;
    auto scale = [](Path& path, double sx, double sy, const IntPoint& center = {}) {
        const bool fl = Area(path) < 0;
        for (IntPoint& pt : path) {
            const double dAangle = (pi * 2) - center.angleRadTo(pt);
            const double length = center.distTo(pt);
            pt = IntPoint(static_cast<cInt>(cos(dAangle) * length * sx), static_cast<cInt>(sin(dAangle) * length * sy));
            pt.X += center.X;
            pt.Y += center.Y;
        }
        if (fl != (Area(path) < 0))
            ReversePath(path);
    };

    scale(path_, scaleX_, scaleY_, {} /*pos_*/);
    for (auto& path : paths_)
        scale(path, scaleX_, scaleY_, {} /*pos_*/);
}

std::tuple<double, double> GraphicObject::scale() const { return {scaleX_, scaleY_}; }

double GraphicObject::scaleX() const { return scaleX_; }

double GraphicObject::scaleY() const { return scaleY_; }

void GraphicObject::setPos(QPointF pos) {
    pos_ = pos;
    TranslatePath(path_, pos_);
    for (auto& path : paths_)
        TranslatePath(path, pos_);
}

QPointF GraphicObject::pos() const { return pos_; }

const Entity* GraphicObject::entity() const { return file_ ? file_->entities().at(entityId_).get() : nullptr; }

// const File* GraphicObject::file() const { return gFile_; }

const Path& GraphicObject::path() const { return path_; }

const Paths& GraphicObject::paths() const { return paths_; }

Path GraphicObject::line() const { return {}; }

Path GraphicObject::lineW() const { return {}; }

Path GraphicObject::polyLine() const { return {}; }

Paths GraphicObject::polyLineW() const { return {}; }

Path GraphicObject::elipse() const { return path_; }

Paths GraphicObject::elipseW() const { return paths_; }

Path GraphicObject::arc() const { return {}; }

Path GraphicObject::arcW() const { return {}; }

Path GraphicObject::polygon() const { return {}; }

Paths GraphicObject::polygonWholes() const { return {}; }

Path GraphicObject::hole() const { return {}; }

Paths GraphicObject::holes() const { return {}; }

bool GraphicObject::positive() const { return {}; }

bool GraphicObject::closed() const { return {}; }

Path& GraphicObject::rPath() { return path_; }

Paths& GraphicObject::rPaths() { return paths_; }

} // namespace Dxf
