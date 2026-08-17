// Pocket (raster) gcode generation.
// Requires: common_gcode.js

function generate(file) {
    var depths = file.getDepths();
    forEachTile(file, function(file, pathss) {
        if (file.laser)
            laserTile(file, pathss);
        else
            millingRasterTile(file, pathss, depths);
    });
}
