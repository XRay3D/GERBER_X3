// Postprocessor: Klipper (3D printer)
// See README.md in this folder for the contract.

var post = {
    name: "Klipper (3D printer)",
    description: "Klipper firmware. Arcs are output as G1 chords (G2/G3 require [gcode_arcs] in printer.cfg). Spindle/laser via M3/M4/M5 macros -- define them in printer.cfg (e.g. SET_PIN).",
    extension: "gcode",
    comment: ";",
    arcs: "linear",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: false,

    header: function(ctx) {
        var lines = ["G21", "G90"];
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
