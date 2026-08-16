// Postprocessor: Haas
// See README.md in this folder for the contract.

var post = {
    name: "Haas",
    description: "Haas NGC (Fanuc-compatible). Percent-framed, O-number, () comments, N line numbers, G53 G0 Z0 return, G187 smoothing. Template -- check against your machine.",
    extension: "nc",
    comment: "()",
    arcs: "ij",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: { start: 10, step: 10 },

    header: function(ctx) {
        var lines = ["%", "O00001 (" + (ctx.programName || ctx.name).replace(/[()]/g, "") + ")",
                     "G21 G17 G40 G49 G80 G90 G94", "G54", "G187 P2"];
        if (!ctx.laser) lines.push("S" + ctx.spindleSpeed + " M3");
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M9", "G53 G0 Z0", "M30", "%"];
    },
    spindleOn:  function(ctx) { return "S" + ctx.spindleSpeed + " M3"; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
