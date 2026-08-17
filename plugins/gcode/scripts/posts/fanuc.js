// Postprocessor: Fanuc
// See README.md in this folder for the contract.

var post = {
    name: "Fanuc",
    description: "Fanuc-style ISO (0i/30i and compatibles). Percent-framed, O-number, () comments, N line numbers, G91 G28 Z0 return. Template -- check against your machine.",
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
        var lines = ["%", "O0001 (" + (ctx.programName || ctx.name).replace(/[()]/g, "") + ")",
                     "G21 G17 G40 G49 G80 G90 G94", "G54"];
        if (!ctx.laser) lines.push("S" + ctx.spindleSpeed + " M3");
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M9", "G91 G28 Z0", "G90", "M30", "%"];
    },
    spindleOn:  function(ctx) { return "S" + ctx.spindleSpeed + " M3"; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
