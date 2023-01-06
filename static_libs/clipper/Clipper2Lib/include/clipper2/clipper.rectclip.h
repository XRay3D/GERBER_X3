/*******************************************************************************
 * Author    :  Angus Johnson                                                   *
 * Date      :  26 October 2022                                                 *
 * Website   :  http://www.angusj.com                                           *
 * Copyright :  Angus Johnson 2010-2022                                         *
 * Purpose   :  FAST rectangular clipping                                       *
 * License   :  http://www.boost.org/LICENSE_1_0.txt                            *
 *******************************************************************************/

#ifndef CLIPPER_RECTCLIP_H
#define CLIPPER_RECTCLIP_H

#include "clipper.core.h"
#include "clipper.h"
#include <cstdlib>
#include <vector>

namespace Clipper2Lib {

enum class Location { Left,
    Top,
    Right,
    Bottom,
    Inside };

class RectClip {
protected:
    const RectI rect_;
    const PointI mp_;
    const PathI rectPath_;
    PathI result_;
    std::vector<Location> start_locs_;

    void GetNextLocation(const PathI& path,
        Location& loc, int& i, int highI);
    void AddCorner(Location prev, Location curr);
    void AddCorner(Location& loc, bool isClockwise);

public:
    RectClip(const RectI& rect)
        : rect_(rect)
        , mp_(rect.MidPoint())
        , rectPath_(rect.AsPath()) { }
    PathI Execute(const PathI& path);
};

class RectClipLines : public RectClip {
public:
    RectClipLines(const RectI& rect)
        : RectClip(rect) {};
    PathsI Execute(const PathI& path);
};

} // namespace Clipper2Lib
#endif // CLIPPER_RECTCLIP_H
