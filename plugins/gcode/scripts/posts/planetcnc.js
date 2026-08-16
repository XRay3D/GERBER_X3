// Postprocessor: PlanetCNC
// See README.md in this folder for the contract.

var post = {
    name: "PlanetCNC",
    description: "PlanetCNC TNG. Parenthesised comments, M3/M5 spindle, M3/M4 laser, I/J arcs.",
    extension: "nc",
    comment: "()",
    arcs: "ij",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
    },
    lineNumbers: false,

    header: function(ctx) {
        var lines = ["G21 G17 G90 G40 G49 G94"];
        if (!ctx.laser) lines.push("M3 S" + ctx.spindleSpeed);
        return lines;
    },
    footer: function(ctx) {
        return ctx.laser ? ["M30"] : ["M5", "M30"];
    },
    spindleOn:  function(ctx) { return "M3 S" + ctx.spindleSpeed; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
