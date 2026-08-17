// Внутренняя (кольцевая) шестерня -- зубья нарезаны на внутренней стороне
// обода и смотрят к центру. См. подробный контракт скрипта и API sh/pl в
// шапке gear.js -- здесь только то, чем эта шестерня отличается.
//
// Зуб внутренней шестерни -- это ВПАДИНА внешней шестерни того же модуля и
// числа зубьев (профиль вогнутый, "reentrant"): фланги -- те же эвольвенты
// того же основного радиуса rb, что и в gear.js, но материал лежит между
// флангом side=-1 внешнего зуба k и флангом side=+1 внешнего зуба k+1, а не
// между двумя флангами одного зуба. Поэтому зуб широкий у корня (у обода) и
// сужается к вершине (к центру) -- ровно наоборот тому, что было бы, возьми
// мы контур внешнего зуба как есть.
//
// Меняются и радиусы, на которых профиль обрывается:
//   ra (вершина зуба, обращена к центру) = rp - m    -- МЕНЬШЕ делительного
//   rf (корень зуба, у самого обода)     = rp + 1.25m -- БОЛЬШЕ делительного
// Вдоль фланга параметр эвольвенты t от корня к вершине УБЫВАЕТ; при
// z < 2/(1-cos(alpha)) (~34 при 20°) вершина проваливается под основную
// окружность -- там эвольвенты нет, и её достраивает радиальный отрезок до
// окружности вершин.
//
// Впадина между зубьями -- внешний зуб до радиуса rf; при большом угле
// давления он там уже острый (ширина впадины <= 0). Тогда корень обрывается
// на радиусе острия (фланги сходятся в точку), иначе контур самопересекается.
//
// Обод: по рекомендации для кольцевых шестерён толщина обода за корнем
// зубьев -- не меньше ~4 высот зуба (4 * 2.25m), иначе кольцо ведёт при
// обработке. Параметр rimWidth оставлен свободным, по умолчанию -- 4h.

var params = {
    module: {
        value: 2,
        min: 0.1,
        max: 50,
        step: 0.5,
        decimals: 2,
        description: "Module, mm"
    },
    teeth: {
        value: 40,
        min: 8,
        max: 500,
        step: 1,
        decimals: 0,
        description: "Teeth"
    },
    pressureAngle: {
        value: 20,
        min: 10,
        max: 30,
        step: 0.5,
        decimals: 1,
        description: "Pressure angle, deg"
    },
    rimWidth: {
        value: 18,
        min: 0.5,
        max: 200,
        step: 0.5,
        decimals: 2,
        description: "Rim width beyond root, mm (>= 4 tooth heights)"
    },
    involuteSteps: {
        value: 8,
        min: 2,
        max: 64,
        step: 1,
        decimals: 0,
        description: "Involute segments"
    }
};

function build(p, sh) {
    var m = p.module;
    var z = Math.max(8, Math.round(p.teeth));
    var alpha = sh.deg2rad(p.pressureAngle);
    var steps = Math.max(2, Math.round(p.involuteSteps));

    var rp = m * z / 2;              // делительный радиус
    var rb = rp * Math.cos(alpha);   // основной радиус (тот же, что и у внешней шестерни)
    var ra = rp - m;                 // вершина зуба -- у центра (МЕНЬШЕ делительного)
    var rf = rp + 1.25 * m;          // корень зуба -- у обода (БОЛЬШЕ делительного)

    var halfPitch0 = Math.PI / (2 * z);
    var tPitch = Math.tan(alpha);
    // Обратная эвольвентная функция: t, при котором inv(t) = t - atan(t) = v (Ньютон).
    function solveInv(v) {
        var t = Math.cbrt(3 * v);
        for (var n = 0; n < 30; n++) {
            var f = t - Math.atan(t) - v;
            var d = t * t / (1 + t * t);
            if (d < 1e-12) break;
            var dt = f / d;
            t -= dt;
            if (Math.abs(dt) < 1e-14) break;
        }
        return t;
    }
    var invPitch = tPitch - Math.atan(tPitch);
    // Впадина между зубьями -- это внешний зуб, продлённый до rf = rp+1.25m.
    // При большом угле давления и/или большом z он на этом радиусе уже
    // заострён: фланги сходятся раньше, чем доходят до rf (ширина впадины
    // inv(t)-invPitch = halfPitch), и контур самопересекается. Поэтому корень
    // обрываем на меньшем из rf и радиуса острия.
    var tRootGeom = Math.sqrt((rf / rb) * (rf / rb) - 1); // корень всегда выше базовой окружности
    var tSpaceClose = solveInv(halfPitch0 + invPitch);   // где впадина схлопывается в остриё
    // Допуск: почти нулевой зазор тоже считаем острием -- иначе дуга корня
    // между двумя численно совпавшими точками может уйти на полный круг.
    var rootPointed = tSpaceClose < tRootGeom + 1e-6;
    var tRoot = rootPointed ? tSpaceClose : tRootGeom;
    if (rootPointed) rf = rb * Math.sqrt(1 + tRoot * tRoot);
    var tTip = ra > rb ? Math.sqrt((ra / rb) * (ra / rb) - 1) : 0; // вершина может провалиться под неё
    var halfPitch = halfPitch0;
    var delta = -(halfPitch + invPitch);

    // Точка фланга по параметру t; side = +1 / -1 -- см. gear.js
    function flank(t, side, rot) {
        var r = rb * Math.sqrt(1 + t * t);
        var ang = side * (t - Math.atan(t) + delta) + rot;
        return {
            x: r * Math.cos(ang),
            y: r * Math.sin(ang),
            r: r,
            ang: ang
        };
    }
    // Эволюта эвольвенты -- сама базовая окружность (см. gear.js)
    function curvatureCenter(t, side, rot) {
        var a = side * (t + delta) + rot;
        return { x: rb * Math.cos(a), y: rb * Math.sin(a) };
    }
    function ccwOf(a, b, c) {
        return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x) > 0;
    }
    function evolventArc(pl, p0, t0, t1, side, rot) {
        var c = curvatureCenter((t0 + t1) / 2, side, rot);
        var p1 = flank(t1, side, rot);
        pl.arcToC(p1.x, p1.y, c.x, c.y, ccwOf(p0, p1, c));
        return p1;
    }

    var pitchAng = 2 * Math.PI / z;
    var pl = null;

    // Зуб k занимает впадину внешней шестерни между её зубьями k и k+1:
    // левый (по обходу против часовой) фланг -- side=-1 зуба k, правый --
    // side=+1 зуба k+1 (rot + pitchAng). Обход контура против часовой:
    // корень (rf) -> по флангу к вершине (ra) -> дуга вершины -> по второму
    // флангу к корню -> дуга корня к следующему зубу.
    for (var k = 0; k < z; k++) {
        var rot = k * pitchAng;
        var rotNext = rot + pitchAng;

        // Корень (широкая окружность rf, у самого обода) -- всегда выше
        // базовой, особый случай не нужен (в отличие от внешней шестерни).
        var start = flank(tRoot, -1, rot);
        if (k === 0)
            pl = sh.begin(start.x, start.y);
        else if (!rootPointed) // при остром корне фланги уже сошлись в start
            pl.arcToC(start.x, start.y, 0, 0, true);

        // Первый фланг: от корня (rf, снаружи) к вершине (ra, у центра) --
        // t убывает, полярный угол растёт (обход против часовой).
        var pPrev = start, tPrev = tRoot;
        for (var i = 1; i <= steps; i++) {
            var t = tRoot + (tTip - tRoot) * i / steps;
            pPrev = evolventArc(pl, pPrev, tPrev, t, -1, rot);
            tPrev = t;
        }

        // Вершина (внутренняя, у центра). Если ra ниже базовой окружности --
        // эвольвента там уже не существует, достраиваем радиальными отрезками
        // от базовой окружности до окружности вершин по обе стороны зуба.
        var tip2 = flank(tTip, +1, rotNext);
        if (tTip === 0) {
            pl.lineTo(ra * Math.cos(pPrev.ang), ra * Math.sin(pPrev.ang));
            pl.arcToC(ra * Math.cos(tip2.ang), ra * Math.sin(tip2.ang), 0, 0, true);
            pl.lineTo(tip2.x, tip2.y);
        } else {
            pl.arcToC(tip2.x, tip2.y, 0, 0, true);
        }

        // Второй фланг: от вершины обратно к корню -- t растёт.
        var pPrev2 = tip2, tPrev2 = tTip;
        for (var j = 1; j <= steps; j++) {
            var t2 = tTip + (tRoot - tTip) * j / steps;
            pPrev2 = evolventArc(pl, pPrev2, tPrev2, t2, +1, rotNext);
            tPrev2 = t2;
        }
    }
    // Замыкающая дуга по корню к первой точке (rf всегда выше базовой -- просто);
    // при остром корне последняя точка уже совпала с первой -- close() уберёт дубль.
    if (!rootPointed) {
        var first = flank(tRoot, -1, 0);
        pl.arcToC(first.x, first.y, 0, 0, true);
    }
    pl.close();

    // Обод: окружность за корнем зубьев.
    sh.circle(0, 0, (rf + p.rimWidth) * 2);
}
