// Pocket (raster) gcode generation.
// Requires: common_gcode.js

function generate(file) {
    var depths = file.getDepths();

    forEachTile(file, function(file, pathss) {
        for (var i = 0; i < pathss.length; i++) {
            var paths = pathss[i];
            for (var d = 0; d < depths.length; d++) {
                for (var j = 0; j < paths.length; j++) {
                    file.startPath(paths[j][0].x, paths[j][0].y);
                    file.z = depths[d];
                    file.addLine(file.formatted([file.g1(), file.fmtZ(depths[d]), file.strPlungeFeed]));
                    appendLines(file, file.savePathLines(i, j, false, 0, 0));
                    file.endPath();
                }
            }
        }
    });
}
