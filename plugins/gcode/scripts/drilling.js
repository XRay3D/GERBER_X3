// Drilling gcode generation (peck drilling).
// Requires: common_gcode.js

function generate(file) {
    var depths        = file.getDepths();
    var toolLength    = file.toolLength;
    var toolOneTurnCut = file.toolOneTurnCut;

    forEachTile(file, function(file, pathss) {
        var path = pathss[0][0]; // drill points are in the first Curve of the first Curvess
        for (var k = 0; k < path.length; k++) {
            var pt = path[k];
            file.startPath(pt.x, pt.y);
            for (var d = 0; d < depths.length; d++) {
                file.addLine(file.formatted([file.g1(), file.fmtZ(depths[d]), file.strPlungeFeed]));
                if (d < depths.length - 1) {
                    // Peck: retract between passes
                    if (toolLength > depths[d + 1])
                        file.addLine(file.formatted([file.g0(), file.fmtZ(depths[d + 1] + toolOneTurnCut * 4)]));
                    else
                        file.addLine(file.formatted([file.g0(), file.fmtZ(0)]));
                }
            }
            file.endPath();
        }
    });
}
