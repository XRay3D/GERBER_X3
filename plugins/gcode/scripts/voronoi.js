// Voronoi gcode generation: several path groups are milled as a pocket
// (linked loops), a single group -- as a profile.
// Requires: common_gcode.js

function generate(file) {
    var depths = file.getDepths();
    forEachTile(file, function(file, pathss) {
        if (file.laser)
            laserTile(file, pathss);
        else if (pathss.length > 1)
            millingPocketTile(file, pathss, depths);
        else
            millingProfileTile(file, pathss, depths);
    });
}
