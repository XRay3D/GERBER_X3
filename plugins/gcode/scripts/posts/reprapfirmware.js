// Postprocessor: RepRapFirmware (Duet)
// See README.md in this folder for the contract.

var post = {
    name: "RepRapFirmware (Duet)",
    description: "RepRapFirmware 3 (Duet boards). Native G2/G3, M3/M4/M5 for spindle and laser (M452 laser mode).",
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
        var lines = ["G21", "G90", "G17"];
        if (ctx.laser) lines.push("M5"); else lines.push("M3 S" + ctx.spindleSpeed);
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M0"];
    },
    spindleOn:  function(ctx) { return "M3 S" + ctx.spindleSpeed; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
