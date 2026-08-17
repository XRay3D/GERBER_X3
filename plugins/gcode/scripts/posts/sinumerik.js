// Postprocessor: Siemens Sinumerik
// See README.md in this folder for the contract.

var post = {
    name: "Siemens Sinumerik",
    description: "Siemens Sinumerik 802D/828D/840D ISO dialect. ; comments, N line numbers, G71 metric, I/J arcs (incremental centre), M3/M5. Template -- check against your machine.",
    extension: "mpf",
    comment: ";",
    arcs: "ij",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: { start: 10, step: 10 },

    header: function(ctx) {
        var lines = ["G71 G17 G90 G40 G94", "G54"];
        if (!ctx.laser) lines.push("S" + ctx.spindleSpeed + " M3");
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M30"];
    },
    spindleOn:  function(ctx) { return "S" + ctx.spindleSpeed + " M3"; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
