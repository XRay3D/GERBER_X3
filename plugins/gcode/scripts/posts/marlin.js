// Postprocessor: Marlin (3D printer)
// See README.md in this folder for the contract.

var post = {
    name: "Marlin (3D printer)",
    description: "Marlin firmware. G2/G3 need ARC_SUPPORT (on by default). Laser: M3/M4 with LASER_FEATURE, S is 0-255 by default -- set the tool spindle speed accordingly.",
    extension: "gcode",
    comment: ";",
    arcs: "ij",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: false,

    header: function(ctx) {
        var lines = ["G21", "G90", "M82"];
        if (ctx.laser) lines.push("M5"); else lines.push("M3 S" + ctx.spindleSpeed);
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M84"];
    },
    spindleOn:  function(ctx) { return "M3 S" + ctx.spindleSpeed; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
