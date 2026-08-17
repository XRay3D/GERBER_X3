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

    // Which end of the thread the cut starts at: the form stores its "Start
    // from" radio in Convent, and climb == !convent, so climb reads here as
    // "plunge at the top and work downwards" (see Threading::Form::computePaths).
    var fromTop = file.climb;

    // The arc direction is pure helix geometry, not a climb/conventional choice:
    // a right-hand thread runs counter-clockwise (G3) while ascending and
    // clockwise (G2) while descending, a left-hand one mirrors that. Internal and
    // external threads are indistinguishable here -- in both cases the cutter
    // traces the very same helix about the axis.
    var ccw = !fromTop;
    if (file.leftHand) ccw = !ccw;

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
                    cutInternal(file, cx, cy, holeD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch, fromTop);
                else
                    cutExternal(file, cx, cy, nomD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch, fromTop);
            }
        }
    });
}

// formatted() drops words that did not change, so a move that repeats the
// current state collapses to an empty string -- which must not reach the file
// as a blank line.
function emit(file, parts) {
    var line = file.formatted(parts);
    if (line.length) file.addLine(line);
}

// Full circle at (cx,cy)+r*angle(a0), climbing/descending from z to itself
// (a flat pass when zPrev === z, a helical turn otherwise).
function emitCircle(file, g, cx, cy, r, a0, z) {
    emit(file, [g,
        file.fmtX(cx + r * Math.cos(a0)), file.fmtY(cy + r * Math.sin(a0)), file.fmtZ(z),
        file.fmtI(-r * Math.cos(a0)), file.fmtJ(-r * Math.sin(a0)), file.strFeed]);
}

// Bore already exists (pre-drilled to holeD), so it's always safe to travel
// through the axis: rapid to center once, then step outward pass by pass.
function cutInternal(file, cx, cy, holeD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch, fromTop) {
    var g      = ccw ? file.g3() : file.g2();
    var twoPiN = 2 * Math.PI / starts;
    var zStart = fromTop ? 0 : -totalH;

    // Не startPath(): та вдобавок касается поверхности (G1 Z0), а при старте
    // снизу это лишний ход -- на рабочую высоту всё равно уводит первая же
    // строка прохода. Остальное -- её же выход на ось на высоте перехода.
    emit(file, [file.g0(), file.fmtX(cx), file.fmtY(cy), file.strSpindle]);
    emit(file, [file.g0(), file.fmtZ(file.properties.plunge)]);
    file.z = zStart;

    for (var i = 1; i <= steps; i++) {
        var r = holeD / 2 - toolR + i * stepReal;

        for (var s = 0; s < starts; s++) {
            var a0 = s * twoPiN;
            var sx = cx + r * Math.cos(a0), sy = cy + r * Math.sin(a0);

            emit(file, [file.g1(), file.fmtZ(zStart), file.strPlungeFeed]);
            emit(file, [file.g1(), file.fmtX(cx + 0.9 * r * Math.cos(a0)), file.fmtY(cy + 0.9 * r * Math.sin(a0)), file.strFeed]);
            emit(file, [file.g1(), file.fmtX(sx), file.fmtY(sy), file.fmtF(file.feedRate / 10)]);

            // The extra full circle always sits at the bottom of the thread, so
            // it goes before the helix when climbing out and after it when
            // cutting downwards.
            if (file.circle && !fromTop)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            for (var j = 1; j <= turns; j++)
                emitCircle(file, g, cx, cy, r, a0, fromTop ? -j * turnStep : -totalH + j * turnStep);

            if (file.circle && fromTop)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            // Back to the axis after every start: the next one begins a half
            // (1/N) turn away, and the bore is the only place free to cross.
            emit(file, [file.g0(), file.fmtX(cx), file.fmtY(cy)]);
        }
    }

    if (file.chamfer) {
        var zChamfer = -0.45 * pitch;
        var r1       = holeD / 2 - toolR + stepReal;
        emit(file, [file.g1(), file.fmtZ(zChamfer), file.strPlungeFeed]);
        emit(file, [file.g1(), file.fmtX(cx + 0.9 * r1), file.fmtY(cy), file.strFeed]);
        for (var i = 1; i <= steps; i++) {
            var r = holeD / 2 - toolR + i * stepReal;
            emit(file, [file.g1(), file.fmtX(cx + r), file.fmtY(cy), file.fmtF(file.feedRate / 15)]);
            emitCircle(file, g, cx, cy, r, 0, zChamfer);
        }
        emit(file, [file.g0(), file.fmtX(cx), file.fmtY(cy)]);
    }

    file.endPath();
}

// Stock is solid outside the thread, so every pass must retract clear of it
// before descending and approach/retreat sideways rather than through center.
function cutExternal(file, cx, cy, nomD, toolR, steps, stepReal, starts, turns, turnStep, totalH, ccw, pitch, fromTop) {
    var g         = ccw ? file.g3() : file.g2(); // same helix geometry as internal
    var twoPiN    = 2 * Math.PI / starts;
    var approachR = nomD / 2 + toolR + 1;
    var zStart    = fromTop ? 0 : -totalH;

    for (var i = 1; i <= steps; i++) {
        var r = nomD / 2 + toolR - i * stepReal;

        for (var s = 0; s < starts; s++) {
            var a0 = s * twoPiN;
            var ax = cx + approachR * Math.cos(a0), ay = cy + approachR * Math.sin(a0);
            var sx = cx + r * Math.cos(a0), sy = cy + r * Math.sin(a0);

            emit(file, [file.g0(), file.fmtZ(file.properties.clearence)]);
            emit(file, [file.g0(), file.fmtX(ax), file.fmtY(ay)]);
            emit(file, [file.g1(), file.fmtZ(zStart), file.strPlungeFeed]);
            emit(file, [file.g1(), file.fmtX(sx), file.fmtY(sy), file.fmtF(file.feedRate / 10)]);

            if (file.circle && !fromTop)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            for (var j = 1; j <= turns; j++)
                emitCircle(file, g, cx, cy, r, a0, fromTop ? -j * turnStep : -totalH + j * turnStep);

            if (file.circle && fromTop)
                emitCircle(file, g, cx, cy, r, a0, -totalH);

            emit(file, [file.g0(), file.fmtX(ax), file.fmtY(ay)]);
        }
    }
    emit(file, [file.g0(), file.fmtZ(file.properties.clearence)]);

    if (file.chamfer) {
        var zChamfer = -0.45 * pitch;
        var r1       = nomD / 2 + toolR - stepReal;
        emit(file, [file.g0(), file.fmtZ(file.properties.clearence)]);
        emit(file, [file.g1(), file.fmtX(cx + r1), file.fmtY(cy), file.fmtF(file.feedRate / 10)]);
        emit(file, [file.g1(), file.fmtZ(zChamfer), file.strPlungeFeed]);
        for (var i = 1; i <= steps; i++) {
            var r = nomD / 2 + toolR - i * stepReal;
            emit(file, [file.g1(), file.fmtX(cx + r), file.fmtY(cy), file.fmtF(file.feedRate / 10)]);
            emitCircle(file, g, cx, cy, r, 0, zChamfer);
        }
        emit(file, [file.g0(), file.fmtX(cx + approachR), file.fmtY(cy)]);
    }

    emit(file, [file.g0(), file.fmtZ(file.properties.clearence)]);
}
