// Модульная прямозубая эвольвентная шестерня.
//
// Контракт скрипта фигуры:
//   var params = { name: число | {value, min, max, step, decimals, description}, ... }
//   description -- подпись строки в таблице редактора (иначе показывается имя)
//   function build(p, sh) { ... }   // p -- текущие значения, sh -- API построения
//
// API sh (все координаты локальные, центр фигуры -- (0, 0)):
//   sh.line(x1, y1, x2, y2)                    отрезок
//   sh.circle(cx, cy, diameter)                окружность
//   sh.rectangle(cx, cy, w, h, angleDeg?)      прямоугольник по центру (с поворотом)
//   sh.arc(cx, cy, r, startDeg, sweepDeg)      дуга по центру (размах > 0 -- против часовой)
//   sh.arcBulge(x1, y1, x2, y2, bulge)         дуга через прогиб (bulge = tan(θ/4))
//   sh.arcC(x1, y1, x2, y2, cx, cy, ccw)       дуга по центру и направлению
//   sh.polyline([[x, y, bulge?], ...], closed) полилиния (или [{x, y, bulge}, ...])
//   sh.begin(x, y)                             построитель: .lineTo(x, y) .arcTo(x, y, bulge)
//                                              .arcToC(x, y, cx, cy, ccw) .close() .end()
//   sh.bulge(x1, y1, x2, y2, cx, cy, ccw)      прогиб дуги для polyline()
//   sh.polar(r, deg) -> {x, y}, sh.deg2rad(), sh.rad2deg()

var params = {
    module:        { value: 2,  min: 0.1, max: 50,   step: 0.5,  decimals: 2, description: "Module, mm" },
    teeth:         { value: 20, min: 4,   max: 500,  step: 1,    decimals: 0, description: "Teeth" },
    pressureAngle: { value: 20, min: 10,  max: 30,   step: 0.5,  decimals: 1, description: "Pressure angle, deg" },
    bore:          { value: 6,  min: 0,   max: 1000, step: 0.5,  decimals: 2, description: "Bore diameter, mm" },
    involuteSteps: { value: 8,  min: 2,   max: 64,   step: 1,    decimals: 0, description: "Involute segments" },
};

function build(p, sh) {
    var m = p.module;
    var z = Math.max(4, Math.round(p.teeth));
    var alpha = sh.deg2rad(p.pressureAngle);
    var steps = Math.max(2, Math.round(p.involuteSteps));

    var rp = m * z / 2;              // делительный радиус
    var rb = rp * Math.cos(alpha);   // основной радиус
    var ra = rp + m;                 // радиус вершин
    var rf = rp - 1.25 * m;          // радиус впадин

    // Эвольвента: t -- параметр развёртки, r(t) = rb*sqrt(1+t^2), полярный угол inv(t) = t - atan(t).
    var tPitch = Math.tan(alpha);
    var invPitch = tPitch - Math.atan(tPitch);
    var tTip = Math.sqrt((ra / rb) * (ra / rb) - 1);
    var tStart = rf > rb ? Math.sqrt((rf / rb) * (rf / rb) - 1) : 0;
    var halfPitch = Math.PI / (2 * z);           // половина углового шага
    var delta = -(halfPitch + invPitch);         // поворот эвольвенты: на делительном радиусе угол = -halfPitch

    // Точка фланга по параметру t; side = +1 (сторона зуба по часовой, угол растёт с радиусом) / -1 (зеркало)
    function flank(t, side, rot) {
        var r = rb * Math.sqrt(1 + t * t);
        var ang = side * (t - Math.atan(t) + delta) + rot;
        return { x: r * Math.cos(ang), y: r * Math.sin(ang), r: r, ang: ang };
    }

    var pitchAng = 2 * Math.PI / z;
    var pl = null;

    for (var k = 0; k < z; k++) {
        var rot = k * pitchAng;

        // Корень зуба (со стороны по часовой)
        var start = flank(tStart, +1, rot);
        if (k === 0) {
            if (rf < rb) {
                var s0 = { x: rf * Math.cos(start.ang), y: rf * Math.sin(start.ang) };
                pl = sh.begin(s0.x, s0.y);
                pl.lineTo(start.x, start.y);
            } else
                pl = sh.begin(start.x, start.y);
        } else {
            if (rf < rb) {
                var s1 = { x: rf * Math.cos(start.ang), y: rf * Math.sin(start.ang) };
                pl.arcToC(s1.x, s1.y, 0, 0, true);   // дуга по впадине к началу зуба
                pl.lineTo(start.x, start.y);
            } else
                pl.arcToC(start.x, start.y, 0, 0, true);
        }

        // Восходящий фланг
        for (var i = 1; i <= steps; i++) {
            var t = tStart + (tTip - tStart) * i / steps;
            var q = flank(t, +1, rot);
            pl.lineTo(q.x, q.y);
        }
        // Вершина: дуга по окружности вершин
        var tip2 = flank(tTip, -1, rot);
        pl.arcToC(tip2.x, tip2.y, 0, 0, true);
        // Нисходящий фланг
        for (var j = steps - 1; j >= 0; j--) {
            var t2 = tStart + (tTip - tStart) * j / steps;
            var q2 = flank(t2, -1, rot);
            pl.lineTo(q2.x, q2.y);
        }
        if (rf < rb) {
            var e = flank(tStart, -1, rot);
            pl.lineTo(rf * Math.cos(e.ang), rf * Math.sin(e.ang));
        }
    }
    // Замыкающая дуга по впадине к первой точке
    var first = flank(tStart, +1, 0);
    var firstAng = first.ang;
    var r0 = rf < rb ? rf : first.r;
    pl.arcToC(r0 * Math.cos(firstAng), r0 * Math.sin(firstAng), 0, 0, true);
    pl.close();

    if (p.bore > 0)
        sh.circle(0, 0, p.bore);
}
