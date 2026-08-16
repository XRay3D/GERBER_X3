// Profile gcode generation (milling spiral/zigzag + laser + bridges/tabs).
// Requires: common_gcode.js

function generate(file) {
    var depths  = file.getDepths();
    var bridges = file.ext && file.ext.hasBridges;
    forEachTile(file, function(file, pathss) {
        if (file.laser)
            laserTile(file, pathss);
        else if (bridges)
            bridgesTile(file, pathss, depths);
        else
            millingProfileTile(file, pathss, depths);
    });
}

// Profile with bridges (tabs): the contour stays ONE continuous pass; on
// passes below the tab top the cutter climbs over each bridge to the tab top
// and comes right back down -- XY feed never stops, no vertical plunges.
// Geometry (pieces, humps, cut discs) comes from C++ via file.ext (Profile::BridgesApi):
//   ext.chain(i, j)  -> [{id, bridge, perimeter}]
//   ext.split(id)    -> {up, flat, down}   (flat == -1 when the hump is a triangle)
//   ext.reverse(id)  -> id
//   ext.lines(id, perimeter, depth) -> gcode lines (perimeter > 0 && depth != 0: ramp)
function bridgesTile(file, pathss, depths) {
    var ext    = file.ext;
    var tabTop = ext.tabTop;
    var spiral = file.spiralRamp;

    for (var i = 0; i < pathss.length; i++) {
        var paths = pathss[i];
        for (var j = 0; j < paths.length; j++) {
            var path  = paths[j];
            var chain = ext.chain(i, j);
            if (chain.length === 0) continue;

            // Path entirely under a bridge (contour smaller than the cut disc):
            // never go below the tab top, otherwise it's an ordinary piece.
            var passes = depths;
            if (chain.length === 1 && chain[0].bridge) {
                chain[0].bridge = false;
                passes = [];
                for (var d = 0; d < depths.length; d++) {
                    var z = Math.max(depths[d], tabTop);
                    if (passes.length === 0 || passes[passes.length - 1] !== z)
                        passes.push(z);
                }
            }

            // Open path is walked back and forth (zigzag): plunging where the
            // tool already stands is cheaper.
            var zigzag = !path.closed;
            var reversedChain = null;
            if (zigzag) {
                reversedChain = [];
                for (var k = chain.length - 1; k >= 0; k--)
                    reversedChain.push({ id: ext.reverse(chain[k].id), bridge: chain[k].bridge, perimeter: chain[k].perimeter });
            }

            // One pass: ordinary pieces at depth, bridge pieces as a hump up to
            // the tab top. Down to the level along the first piece (ramp) or by
            // a vertical plunge.
            var millPass = function(chain, depth, ramp) {
                var first = true;
                for (var k = 0; k < chain.length; k++) {
                    var piece = chain[k];
                    if (piece.bridge && depth < tabTop) {
                        var hump = ext.split(piece.id);
                        appendLines(file, ext.lines(hump.up, ext.perimeter(hump.up), tabTop));
                        if (hump.flat >= 0)
                            appendLines(file, ext.lines(hump.flat, 0, 0));
                        appendLines(file, ext.lines(hump.down, ext.perimeter(hump.down), depth));
                    } else if (first && ramp) {
                        appendLines(file, ext.lines(piece.id, piece.perimeter, depth));
                    } else {
                        if (first && !ramp)
                            plungeTo(file, depth);
                        appendLines(file, ext.lines(piece.id, 0, 0));
                    }
                    first = false;
                }
            };

            file.startPath(path[0].x, path[0].y);
            var pass = 0;
            for (var d = 0; d < passes.length; d++, pass++)
                millPass(zigzag && (pass & 1) ? reversedChain : chain, passes[d], spiral);
            if (spiral) // clean up the ramp step, as in millingProfileTile
                millPass(zigzag && (pass & 1) ? reversedChain : chain, passes[passes.length - 1], false);
            file.endPath();
        }
    }
}
