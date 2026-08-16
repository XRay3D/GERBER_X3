// Thread milling gcode generation (helical interpolation), matching the v2.0
// algorithm of the reference calculators at sergshobby.ru:
//   Internal threads (holes) - calc3.php
//   External threads (rods)  - calc4.php
//
// Each thread mark is a 3-vertex curve: {center, center + (M/2, 0), center + (0,
// holeD/2)}, so the hole/rod center, the nominal thread diameter M and the
// pre-drilled hole diameter all travel together per point (see
// Threading::Form::computePaths in thr_form.cpp).
//
// Requires: common_gcode.js

function generate(file) {
    var pitch   = file.threadPitch > 0 ? file.threadPitch : 1;        // P (single start)
    var toolR   = file.toolDiameter / 2;                               // R
    var stepMax = file.toolOneTurnCut > 0 ? file.toolOneTurnCut : 0.1; // mh
    var inside  = file.inside;
    var starts  = Math.max(1, Math.round(file.starts));                // N

    // Climb-milling convention (spindle always CW/M3): starting from an inside,
    // right-hand, climb cut using G3, each one of {outside, left-hand, conventional}
    // flips the helix direction once.
    var ccw = inside;
    if (file.leftHand) ccw = !ccw;
    if (!file.climb)   ccw = !ccw;

    var lead   = pitch * starts; // axial advance per revolution of the N-start helix set
    var depths = file.getDepths();
    var totalH = -depths[depths.length - 1]; // H

    var turns    = Math.max(1, Math.ceil(totalH / lead));
    var turnStep = totalH / turns;

    forEachTile(file, function(file, pathss) {
        for (var pi = 0; pi < pathss.length; pi++) {
            var curves = pathss[pi];
            for (var ci = 0; ci < curves.length; ci++) {
                var curve = curves[ci];
                if (curve.length < 2) continue;

                var cx    = curve[0].x, cy = curve[0].y;
                var nomD  = 2 * Math.hypot(curve[1].x - cx, curve[1].y - cy); // M
                var holeD = curve.length > 2
                    ? 2 * Math.hypot(curve[2].x - cx, curve[2].y - cy)        // m
                    : file.threadHoleDiam;

                var half     = Math.max(0, (nomD - holeD) / 2);
                var steps    = Math.max(1, Math.ceil(half / stepMax));
                var stepReal = half / steps;

                if (inside)
                    cutInternal(file, cx, cy, holeD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch);
                else
                    cutExternal(file, cx, cy, nomD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch);
            }
        }
    });
}

// Full circle at (cx,cy)+r*angle(a0), climbing/descending from z to itself
// (a flat pass when zPrev === z, a helical turn otherwise).
function emitCircle(file, g, cx, cy, r, a0, z) {
    file.addLine(file.formatted([g,
        file.fmtX(cx + r * Math.cos(a0)), file.fmtY(cy + r * Math.sin(a0)), file.fmtZ(z),
        file.fmtI(-r * Math.cos(a0)), file.fmtJ(-r * Math.sin(a0)), file.strFeed]));
}

// Bore already exists (pre-drilled to holeD), so it's always safe to travel
// through the axis: rapid to center once, then step outward pass by pass.
function cutInternal(file, cx, cy, holeD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch) {
    var g      = ccw ? file.g3() : file.g2();
    var twoPiN = 2 * Math.PI / starts;

    file.startPath(cx, cy);
    for (var i = 1; i <= steps; i++) {
        var r = holeD / 2 - toolR + i * stepReal;

        for (var s = 0; s < starts; s++) {
            var a0 = s * twoPiN;
            var sx = cx + r * Math.cos(a0), sy = cy + r * Math.sin(a0);

            file.addLine(file.formatted([file.g1(), file.fmtZ(-totalH), file.strPlungeFeed]));
            file.addLine(file.formatted([file.g1(), file.fmtX(cx + 0.9 * r * Math.cos(a0)), file.fmtY(cy + 0.9 * r * Math.sin(a0)), file.strFeed]));
            file.addLine(file.formatted([file.g1(), file.fmtX(sx), file.fmtY(sy), file.fmtF(file.feedRate / 10)]));

            if (file.circle)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            var z = -totalH;
            for (var j = 1; j <= turns; j++) {
                z = -totalH + j * turnStep;
                emitCircle(file, g, cx, cy, r, a0, z);
            }
        }
        file.addLine(file.formatted([file.g0(), file.fmtX(cx), file.fmtY(cy)]));
    }

    if (file.chamfer) {
        var zChamfer = -0.45 * pitch;
        var r1       = holeD / 2 - toolR + stepReal;
        file.addLine(file.formatted([file.g1(), file.fmtZ(zChamfer), file.strPlungeFeed]));
        file.addLine(file.formatted([file.g1(), file.fmtX(cx + 0.9 * r1), file.fmtY(cy), file.strFeed]));
        for (var i = 1; i <= steps; i++) {
            var r = holeD / 2 - toolR + i * stepReal;
            file.addLine(file.formatted([file.g1(), file.fmtX(cx + r), file.fmtY(cy), file.fmtF(file.feedRate / 15)]));
            emitCircle(file, g, cx, cy, r, 0, zChamfer);
        }
        file.addLine(file.formatted([file.g0(), file.fmtX(cx), file.fmtY(cy)]));
    }

    file.endPath();
}

// Stock is solid outside the thread, so every pass must retract clear of it
// before descending and approach/retreat sideways rather than through center.
function cutExternal(file, cx, cy, nomD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch) {
    var g         = ccw ? file.g2() : file.g3(); // opposite base direction of internal for the same climb sense
    var twoPiN    = 2 * Math.PI / starts;
    var approachR = nomD / 2 + toolR + 1;

    for (var i = 1; i <= steps; i++) {
        var r = nomD / 2 + toolR - i * stepReal;

        for (var s = 0; s < starts; s++) {
            var a0 = s * twoPiN;
            var ax = cx + approachR * Math.cos(a0), ay = cy + approachR * Math.sin(a0);
            var sx = cx + r * Math.cos(a0), sy = cy + r * Math.sin(a0);

            file.addLine(file.formatted([file.g0(), file.fmtZ(file.properties.clearence)]));
            file.addLine(file.formatted([file.g0(), file.fmtX(ax), file.fmtY(ay)]));
            file.addLine(file.formatted([file.g1(), file.fmtZ(-totalH), file.strPlungeFeed]));
            file.addLine(file.formatted([file.g1(), file.fmtX(sx), file.fmtY(sy), file.fmtF(file.feedRate / 10)]));

            if (file.circle)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            var z = -totalH;
            for (var j = 1; j <= turns; j++) {
                z = -totalH + j * turnStep;
                emitCircle(file, g, cx, cy, r, a0, z);
            }
            file.addLine(file.formatted([file.g0(), file.fmtX(ax), file.fmtY(ay)]));
        }
    }
    file.addLine(file.formatted([file.g0(), file.fmtZ(file.properties.clearence)]));

    if (file.chamfer) {
        var zChamfer = -0.45 * pitch;
        var r1       = nomD / 2 + toolR - stepReal;
        file.addLine(file.formatted([file.g0(), file.fmtZ(file.properties.clearence)]));
        file.addLine(file.formatted([file.g1(), file.fmtX(cx + r1), file.fmtY(cy), file.fmtF(file.feedRate / 10)]));
        file.addLine(file.formatted([file.g1(), file.fmtZ(zChamfer), file.strPlungeFeed]));
        for (var i = 1; i <= steps; i++) {
            var r = nomD / 2 + toolR - i * stepReal;
            file.addLine(file.formatted([file.g1(), file.fmtX(cx + r), file.fmtY(cy), file.fmtF(file.feedRate / 10)]));
            emitCircle(file, g, cx, cy, r, 0, zChamfer);
        }
        file.addLine(file.formatted([file.g0(), file.fmtX(cx + approachR), file.fmtY(cy)]));
    }

    file.addLine(file.formatted([file.g0(), file.fmtZ(file.properties.clearence)]));
}
