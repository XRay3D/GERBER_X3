// Profile gcode generation (milling spiral/zigzag + laser).
// Requires: common_gcode.js

function generate(file) {
    var depths = file.getDepths();
    forEachTile(file, function(file, pathss) {
        if (file.laser)
            laserTile(file, pathss);
        else
            millingTile(file, pathss, depths);
    });
}

function laserTile(file, pathss) {
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

function millingTile(file, pathss, depths) {
    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        if (paths.length === 1) {
            var path  = paths[0];
            var perim = path.perimetr;
            file.startPath(path[0].x, path[0].y);
            if (path.closed) {
                // Spiral: ramp down through each depth pass, then one flat finishing pass
                for (var d = 0; d < depths.length; d++)
                    appendLines(file, file.savePathLines(i, 0, false, perim, depths[d]));
                appendLines(file, file.savePathLines(i, 0, false, 0, 0));
            } else {
                // Zigzag: alternate direction on each depth pass
                for (var d = 0; d < depths.length; d++)
                    appendLines(file, file.savePathLines(i, 0, (d & 1) !== 0, perim, depths[d]));
                appendLines(file, file.savePathLines(i, 0, (depths.length & 1) !== 0, 0, 0));
            }
            file.endPath();
        } else {
            // Multiple segments: step-down across all segments at each depth level
            file.startPath(paths[0][0].x, paths[0][0].y);
            for (var d = 0; d < depths.length; d++) {
                for (var j = 0; j < paths.length; j++) {
                    var pt = paths[j][0];
                    file.addLine(file.formatted([file.g0(), file.fmtX(pt.x), file.fmtY(pt.y)]));
                    file.z = depths[d];
                    file.addLine(file.formatted([file.g1(), file.fmtZ(depths[d]), file.strPlungeFeed]));
                    appendLines(file, file.savePathLines(i, j, false, 0, 0));
                    file.addLine(file.formatted([file.g0(), file.fmtZ(0)]));
                }
            }
            file.endPath();
        }
    }
}
