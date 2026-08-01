// Common utilities shared by all GCode generation scripts.

// Append an array of gcode line strings to the file output.
function appendLines(file, lines) {
    for (var l = 0; l < lines.length; l++)
        file.addLine(lines[l]);
}

// Iterate over all tile positions and invoke callback(file, pathss, ox, oy)
// for each tile that has non-empty tool paths.
function forEachTile(file, callback) {
    var stepsX     = file.stepsX;
    var stepsY     = file.stepsY;
    var workWidth  = file.workWidth;
    var workHeight = file.workHeight;
    var spaceX     = file.spaceX;
    var spaceY     = file.spaceY;

    for (var xi = 0; xi < stepsX; xi++) {
        for (var yi = 0; yi < stepsY; yi++) {
            var ox     = (workWidth  + spaceX) * xi;
            var oy     = (workHeight + spaceY) * yi;
            var pathss = file.getToolPaths(ox, oy);
            if (pathss.length > 0)
                callback(file, pathss, ox, oy);
        }
    }
}
