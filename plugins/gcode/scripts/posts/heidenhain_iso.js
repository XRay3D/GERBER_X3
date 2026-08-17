// Postprocessor: Heidenhain ISO
// See README.md in this folder for the contract.

var post = {
    name: "Heidenhain ISO",
    description: "Heidenhain TNC in ISO mode (%name G71 * / N... G30 *). ; comments, N line numbers, R arcs. Template -- check against your machine.",
    extension: "i",
    comment: ";",
    arcs: "r",
    arcTolerance: 0.01,
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?R+Z?F?S?" },
        laser:   { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?R+Z?F?S?" },
    },
    lineNumbers: { start: 10, step: 10 },

    header: function(ctx) {
        var pgm = (ctx.programName || ctx.name).replace(/[^A-Za-z0-9]+/g, "_").toUpperCase();
        var lines = ["%" + pgm + " G71 *", "G30 G17 X+0 Y+0 Z-20 *", "G31 G90 X+100 Y+100 Z+0 *", "G17 G90 G94"];
        if (!ctx.laser) lines.push("S" + ctx.spindleSpeed + " M3");
        return lines;
    },
    footer: function(ctx) {
        return ["M5", "M30", "N99999 %" + (ctx.programName || ctx.name).replace(/[^A-Za-z0-9]+/g, "_").toUpperCase() + " G71 *"];
    },
    spindleOn:  function(ctx) { return "S" + ctx.spindleSpeed + " M3"; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
