// Звёздочка для роликовой цепи (профиль зуба по ISO 606, упрощённо).
// См. описание API в gear.js.

var params = {
    teeth:  { value: 18,   min: 5,   max: 300,  step: 1,    decimals: 0, description: "Teeth" },
    pitch:  { value: 12.7, min: 1,   max: 200,  step: 0.5,  decimals: 3, description: "Chain pitch, mm" },
    roller: { value: 7.75, min: 0.5, max: 100,  step: 0.25, decimals: 3, description: "Roller diameter, mm" },
    bore:   { value: 10,   min: 0,   max: 1000, step: 0.5,  decimals: 2, description: "Bore diameter, mm" },
    chamfer:{ value: 0.5,  min: 0,   max: 50,   step: 0.1,  decimals: 2, description: "Tip chamfer, mm" },
};

// Пересечение окружностей (c1, r1) и (c2, r2); возвращает массив точек (0..2).
function circleIntersect(c1, r1, c2, r2) {
    var dx = c2.x - c1.x, dy = c2.y - c1.y;
    var d = Math.sqrt(dx * dx + dy * dy);
    if (d < 1e-12 || d > r1 + r2 || d < Math.abs(r1 - r2)) return [];
    var a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    var h2 = r1 * r1 - a * a;
    var h = h2 > 0 ? Math.sqrt(h2) : 0;
    var mx = c1.x + a * dx / d, my = c1.y + a * dy / d;
    return [
        { x: mx + h * dy / d, y: my - h * dx / d },
        { x: mx - h * dy / d, y: my + h * dx / d },
    ];
}

function build(p, sh) {
    var z = Math.max(5, Math.round(p.teeth));
    var pt = p.pitch;
    var dr = p.roller;

    var D = pt / Math.sin(Math.PI / z);          // делительный диаметр
    var R = D / 2;
    var ri = 0.505 * dr + 0.069 * Math.pow(dr, 1 / 3);           // радиус впадины (посадки ролика)
    var alpha = sh.deg2rad(140 - 90 / z);                        // угол посадки ролика
    var re = 0.12 * dr * (z + 2);                                // радиус рабочего профиля
    var Da = D + pt * (1 - 1.6 / z) - dr;                        // диаметр вершин (минимальный)
    var Ra = Da / 2;
    var ch = Math.max(0, Math.min(p.chamfer, Ra * 0.5));         // фаска на вершине зуба (~45 градусов)
    var Rf = Ra - ch;                                            // радиус, до которого доходит фланг

    var pitchAng = 2 * Math.PI / z;
    var pl = null;
    var TB0 = null;

    for (var k = 0; k < z; k++) {
        var phi = k * pitchAng;                                  // направление на центр впадины
        var C = { x: R * Math.cos(phi), y: R * Math.sin(phi) };  // центр ролика
        // Концы дуги посадки: B -- по часовой от оси впадины, A -- против.
        var aB = phi + Math.PI + alpha / 2;
        var aA = phi + Math.PI - alpha / 2;
        var B = { x: C.x + ri * Math.cos(aB), y: C.y + ri * Math.sin(aB) };
        var A = { x: C.x + ri * Math.cos(aA), y: C.y + ri * Math.sin(aA) };

        // Центр рабочей дуги: на луче A -> C, за C, на расстоянии re от A.
        function flankCenter(P, aP) { return { x: P.x - re * Math.cos(aP), y: P.y - re * Math.sin(aP) }; }
        var CfA = flankCenter(A, aA);
        var CfB = flankCenter(B, aB);

        // Точка выхода рабочей дуги на окружность Rf (вершины минус фаска): та, что дальше по обходу.
        function tipPoint(Cf, ref, forward) {
            var pts = circleIntersect(Cf, re, { x: 0, y: 0 }, Rf);
            if (pts.length === 0) return null;
            var best = null, bestAng = forward ? -Infinity : Infinity;
            var refAng = Math.atan2(ref.y, ref.x);
            for (var i = 0; i < pts.length; i++) {
                var da = Math.atan2(pts[i].y, pts[i].x) - refAng;
                while (da > Math.PI) da -= 2 * Math.PI;
                while (da < -Math.PI) da += 2 * Math.PI;
                if (forward ? da > bestAng : da < bestAng) { bestAng = da; best = pts[i]; }
            }
            return best;
        }
        var TA = tipPoint(CfA, A, true);    // выход восходящего фланга (после A)
        var TB = tipPoint(CfB, B, false);   // выход нисходящего фланга (перед B)
        if (!TA || !TB) {
            // Вершина недостижима -- ограничиваем фланг углом 60 градусов.
            var swA = Math.PI / 3, swB = Math.PI / 3;
            TA = { x: CfA.x + re * Math.cos(aA - swA), y: CfA.y + re * Math.sin(aA - swA) };
            TB = { x: CfB.x + re * Math.cos(aB + swB), y: CfB.y + re * Math.sin(aB + swB) };
        }

        // Точки фаски на окружности вершин: сдвиг по дуге на ch к середине зуба.
        function chamferPoint(T, dir) {
            var a = Math.atan2(T.y, T.x) + dir * ch / Ra;
            return { x: Ra * Math.cos(a), y: Ra * Math.sin(a) };
        }
        var TBc = ch > 0 ? chamferPoint(TB, -1) : TB;
        var TAc = ch > 0 ? chamferPoint(TA, +1) : TA;

        if (k === 0) {
            TB0 = TBc;
            pl = sh.begin(TBc.x, TBc.y);
        } else
            pl.arcToC(TBc.x, TBc.y, 0, 0, true);        // вершина зуба по окружности вершин

        if (ch > 0) pl.lineTo(TB.x, TB.y);              // фаска
        pl.arcToC(B.x, B.y, CfB.x, CfB.y, false);       // нисходящий фланг (вогнутый)
        pl.arcToC(A.x, A.y, C.x, C.y, false);           // посадка ролика
        pl.arcToC(TA.x, TA.y, CfA.x, CfA.y, false);     // восходящий фланг
        if (ch > 0) pl.lineTo(TAc.x, TAc.y);            // фаска
    }
    // Замыкание вершиной первого зуба: дуга по окружности вершин к первой точке.
    pl.arcToC(TB0.x, TB0.y, 0, 0, true);
    pl.close();

    if (p.bore > 0)
        sh.circle(0, 0, p.bore);
}
