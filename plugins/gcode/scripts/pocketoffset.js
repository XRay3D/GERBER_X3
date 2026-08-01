// Pocket (offset) gcode generation.
// Requires: common_gcode.js

function generate(file) {
    var depths       = file.getDepths();
    var toolDiameter = file.toolDiameter;

    forEachTile(file, function(file, pathss) {
        var prevPt = pathss[0][0][0];
        file.startPath(prevPt.x, prevPt.y);

        for (var i = 0; i < pathss.length; i++) {
            var paths = pathss[i];
            for (var d = 0; d < depths.length; d++) {
                for (var j = 0; j < paths.length; j++) {
                    var pt = paths[j][0];
                    var dx = pt.x - prevPt.x;
                    var dy = pt.y - prevPt.y;
                    if (Math.sqrt(dx * dx + dy * dy) > toolDiameter * 2) {
                        file.endPath();
                        file.startPath(pt.x, pt.y);
                    }
                    prevPt = pt;
                    file.addLine(file.formatted([file.g1(), file.fmtX(pt.x), file.fmtY(pt.y)]));
                    file.z = depths[d];
                    file.addLine(file.formatted([file.g1(), file.fmtZ(depths[d]), file.strPlungeFeed]));
                    appendLines(file, file.savePathLines(i, j, false, 0, 0));
                }
            }
        }
        file.endPath();
    });
}
