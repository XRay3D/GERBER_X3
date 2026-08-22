// Common utilities shared by all GCode generation scripts.
// Loaded automatically before the plugin script (see GCode::File::runJsScript).
//
// The `file` object (GCode::GcFileProxy):
//   file.tool         - tool snapshot (diameter, feedRate, passDepth, ...)
//   file.properties   - project properties (GCode::Properties form) as one object:
//                       safeZ, clearence, plunge, boardThickness, copperThickness, glue,
//                       tailing{spacingX, spacingY, stepsX, stepsY},
//                       workRect{x, y, width, height}, home{x, y}, zero{x, y}, pins[]
//   file.laser, file.spiralRamp, file.notTile, file.toolDiameter, ...
//   file.getDepths(), file.getToolPaths(ox, oy), file.savePathLines(...),
//   file.startPath(x, y), file.endPath(), file.addLine(s), file.formatted([...]),
//   file.g0()..g3(), file.fmtX()..fmtF(), file.strFeed/strPlungeFeed/strSpindle,
//   file.comment(text), file.spindleOn(), file.spindleOff(), file.laserOn(dyn), file.laserOff()
//   file.ext          - plugin-specific extension object (Profile: bridges), may be undefined

// Append an array of gcode line strings to the file output.
function appendLines(file, lines) {
    for (var l = 0; l < lines.length; l++)
        file.addLine(lines[l]);
}

// Iterate over all tile positions and invoke callback(file, pathss, ox, oy)
// for each tile that has non-empty tool paths. Honours file.notTile.
function forEachTile(file, callback) {
    var props      = file.properties;
    var stepsX     = props.tailing.stepsX;
    var stepsY     = props.tailing.stepsY;
    var workWidth  = props.workRect.width;
    var workHeight = props.workRect.height;
    var spaceX     = props.tailing.spacingX;
    var spaceY     = props.tailing.spacingY;

    for (var xi = 0; xi < stepsX; xi++) {
        for (var yi = 0; yi < stepsY; yi++) {
            var ox     = (workWidth  + spaceX) * xi;
            var oy     = (workHeight + spaceY) * yi;
            var pathss = file.getToolPaths(ox, oy);
            if (pathss.length > 0)
                callback(file, pathss, ox, oy);
            if (file.notTile)
                return;
        }
    }
}

// Plunge to depth at the current XY (vertical feed).
function plungeTo(file, depth) {
    file.z = depth;
    file.addLine(file.formatted([file.g1(), file.fmtZ(depth), file.strPlungeFeed]));
}

// ---------------------------------------------------------------------------
// Shared tile strategies.

// Laser: every curve is one pass at full power, no depth.
function laserTile(file, pathss) {
    file.addLine(file.laserOn(true));
    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        for (var j = 0; j < paths.length; j++) {
            var path = paths[j];
            file.startPath(path[0].x, path[0].y);
            appendLines(file, file.savePathLines(i, j, false, 0, 0));
            file.endPath();
        }
    }
}

// Milling profile: closed contour -> spiral ramp (or plunge per level),
// open contour -> zigzag alternating direction, several segments -> step-down.
function millingProfileTile(file, pathss, depths) {
    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        if (paths.length === 1) {
            var path  = paths[0];
            var perim = path.perimeter;
            file.startPath(path[0].x, path[0].y);
            if (path.closed) {
                if (file.spiralRamp) {
                    // Spiral: ramp down through each depth pass, then one flat finishing pass
                    for (var d = 0; d < depths.length; d++)
                        appendLines(file, file.savePathLines(i, 0, false, perim, depths[d]));
                    appendLines(file, file.savePathLines(i, 0, false, 0, 0));
                } else {
                    // Plunge straight down at each step; no spiral step left to clean up
                    for (var d = 0; d < depths.length; d++) {
                        plungeTo(file, depths[d]);
                        appendLines(file, file.savePathLines(i, 0, false, 0, 0));
                    }
                }
            } else {
                if (file.spiralRamp) {
                    // Zigzag: alternate direction on each depth pass
                    for (var d = 0; d < depths.length; d++)
                        appendLines(file, file.savePathLines(i, 0, (d & 1) !== 0, perim, depths[d]));
                    appendLines(file, file.savePathLines(i, 0, (depths.length & 1) !== 0, 0, 0));
                } else {
                    // Still alternate: plunging where the tool already stands is cheaper
                    for (var d = 0; d < depths.length; d++) {
                        plungeTo(file, depths[d]);
                        appendLines(file, file.savePathLines(i, 0, (d & 1) !== 0, 0, 0));
                    }
                }
            }
            file.endPath();
        } else {
            // Multiple segments: step-down across all segments at each depth level
            file.startPath(paths[0][0].x, paths[0][0].y);
            for (var d = 0; d < depths.length; d++) {
                for (var j = 0; j < paths.length; j++) {
                    var pt = paths[j][0];
                    file.addLine(file.formatted([file.g0(), file.fmtX(pt.x), file.fmtY(pt.y)]));
                    plungeTo(file, depths[d]);
                    appendLines(file, file.savePathLines(i, j, false, 0, 0));
                    file.addLine(file.formatted([file.g0(), file.fmtZ(0)]));
                }
            }
            file.endPath();
        }
    }
}

// Milling pocket (offset loops): consecutive loops of the SAME pocket (i
// unchanged) are linked at depth, a jump longer than three tool diameters is
// a break -- lift and re-enter. Crossing to a different pathss[i] always
// lifts, even when its start point lands close by: separate i's are
// independently computed regions (e.g. two nearby drilled pads) with no
// guarantee their boundaries actually touch -- dragging the tool through
// whatever solid stock sits between them at full depth breaks it.
function millingPocketTile(file, pathss, depths) {
    var toolDiameter = file.toolDiameter;
    var prevPt = pathss[0][0][0];
    var prevI = 0;
    file.startPath(prevPt.x, prevPt.y);

    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        for (var d = 0; d < depths.length; d++) {
            for (var j = 0; j < paths.length; j++) {
                var pt = paths[j][0];
                var dx = pt.x - prevPt.x;
                var dy = pt.y - prevPt.y;
                if (i !== prevI || Math.sqrt(dx * dx + dy * dy) > toolDiameter * 3) {
                    file.endPath();
                    file.startPath(pt.x, pt.y);
                }
                prevPt = pt;
                prevI = i;
                file.addLine(file.formatted([file.g1(), file.fmtX(pt.x), file.fmtY(pt.y)]));
                plungeTo(file, depths[d]);
                appendLines(file, file.savePathLines(i, j, false, 0, 0));
            }
        }
    }
    file.endPath();
}

// Milling raster: every stroke is its own pass at every depth.
function millingRasterTile(file, pathss, depths) {
    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        for (var d = 0; d < depths.length; d++) {
            for (var j = 0; j < paths.length; j++) {
                file.startPath(paths[j][0].x, paths[j][0].y);
                plungeTo(file, depths[d]);
                appendLines(file, file.savePathLines(i, j, false, 0, 0));
                file.endPath();
            }
        }
    }
}

// Profile-like plugin (Thermal, CrossHatch, ...): laser or milling profile.
function profileGenerate(file) {
    var depths = file.getDepths();
    forEachTile(file, function(file, pathss) {
        if (file.laser)
            laserTile(file, pathss);
        else
            millingProfileTile(file, pathss, depths);
    });
}
