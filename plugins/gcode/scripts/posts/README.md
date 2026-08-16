# G-code postprocessors

A postprocessor describes the *dialect* of a CNC controller: how words are
written, how arcs are expressed, what the program header/footer and the
spindle/laser codes look like. It does **not** decide what to cut -- that is the
job of the plugin scripts (`scripts/<plugin>.js`) and the C++ core; the post only
supplies the words.

Every `scripts/posts/<id>.js` file defines a global object `post`. The file name
without `.js` is the post id (stored in the settings), `post.name` is what the
combobox in *Settings -> G-Code -> Postprocessor* shows. The list is rebuilt from
the folder each time the settings dialog opens, so dropping a new file in is
enough. `generic.js` is the default.

```js
var post = {
    name: "grbl 1.1",                  // shown in the combobox
    description: "…",                  // shown under the combobox
    extension: "nc",                   // output file extension (without the dot)
    comment: ";",                      // ";" or "()"
    arcs: "ij",                        // "ij" | "r" | "linear"
    arcTolerance: 0.01,                // chord deviation for "linear", mm

    // Word format, one string per line kind: linear (G0/G1) and arc (G2/G3),
    // separately for milling and laser (laser inherits milling when omitted).
    //   ?  write the word only if its value changed
    //   +  always write the word
    //   a space after a word puts a space into the output
    //   a letter that is missing is never written
    // Letters: G X Y Z I J R S F.
    format: {
        milling: { linear: "G?X?Y?Z?F?S?", arc: "G?X?Y?I+J+Z?F?S?" },
        laser:   { linear: "G?X?Y?F?S?",   arc: "G?X?Y?I+J+F?S?" },
    },

    lineNumbers: false,                // or { start: 10, step: 10 } -> "N10 …"

    // Text fields may be a string, an array of strings, or a function(ctx)
    // returning either. Empty -> nothing is written.
    header:     function(ctx) { return ["G21 G17 G90", "M3 S" + ctx.spindleSpeed]; },
    footer:     ["M5", "M30"],
    spindleOn:  function(ctx) { return "M3 S" + ctx.spindleSpeed; },
    spindleOff: "M5",
    laserOn:    function(ctx, dynamic) { return dynamic ? "M4" : "M3"; },
    laserOff:   "M5",
};
```

`ctx` is `{laser, name, programName, spindleSpeed, feedRate, plungeRate, tool, properties}`;
`tool` is the tool snapshot, `properties` -- the project properties (safeZ,
clearence, plunge, workRect, tailing, home, zero, …).

What the core always writes itself, regardless of the post: the rapid to the
safe height right after the header, the coordinate moves of the tool path, and
the rapid to the safe height (milling) or laser-off + rapid home (laser) right
before the footer.

Arcs: `"ij"` writes `G2/G3 X Y I J` (I/J relative to the arc start, a full
circle as one block); `"r"` writes `G2/G3 X Y R` (arcs of 180° and more are
split in two); `"linear"` replaces every arc with G1 chords within
`arcTolerance`.

Limitation: `threading.js` writes helical `G2/G3 … I J` blocks directly and is
not adapted by the `arcs` setting.

Industrial controller posts (Fanuc, Haas, Sinumerik, Heidenhain ISO) are
reasonable templates, not certified posts -- check the header/footer against
your machine before running.
