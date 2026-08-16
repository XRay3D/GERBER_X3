// Postprocessor: Mach4
// See README.md in this folder for the contract.

var post = {
    name: "Mach4",
    description: "Mach4 (ArtSoft). Same as Mach3: percent framing, () comments, N numbers, I/J arcs.",
    extension: "tap",
    comment: "()",
    arcs: "ij",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: { start: 10, step: 10 },

    header: function(ctx) {
        var lines = ["%", "G21 G17 G90 G40 G49 G80"];
        if (!ctx.laser) lines.push("M3 S" + ctx.spindleSpeed);
        return lines;
    },
    footer: function(ctx) {
        return ctx.laser ? ["M30", "%"] : ["M5", "M30", "%"];
    },
    spindleOn:  function(ctx) { return "M3 S" + ctx.spindleSpeed; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
