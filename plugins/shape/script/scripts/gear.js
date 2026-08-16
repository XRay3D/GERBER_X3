// Модульная прямозубая эвольвентная шестерня.
//
// ============================================================================
//  КОНТРАКТ СКРИПТА ФИГУРЫ
// ============================================================================
//
// Скрипт лежит в <папка GGEasy>/scripts/shapes/*.js и обязан объявить две вещи:
//
//   var params = { ... };            // объект параметров (значения по умолчанию)
//   function build(p, sh) { ... }    // построение геометрии
//
// params -- ключ = имя параметра, значение либо число, либо дескриптор:
//   { value:       число,     -- значение по умолчанию (обязательно)
//     min:         число,     -- нижняя граница спинбокса      (по умолчанию -1e6)
//     max:         число,     -- верхняя граница спинбокса     (по умолчанию +1e6)
//     step:        число,     -- шаг спинбокса                 (по умолчанию 1)
//     decimals:    целое,     -- знаков после запятой          (по умолчанию 3)
//     description: строка }   -- подпись строки в таблице редактора; если нет,
//                                показывается имя параметра (имя -- в тултипе)
//   Параметры только числовые (bool -- как 0/1). Порядок строк в таблице
//   = порядок объявления. Последние введённые значения запоминаются по имени
//   скрипта и подставляются новым фигурам на нём вместо value.
//
// build(p, sh) вызывается при каждом пересчёте фигуры (смена параметра,
// перемещение, правка файла скрипта):
//   p  -- объект с текущими значениями параметров: p.module, p.teeth, ...
//   sh -- API построения (ниже). Возвращать ничего не нужно: всё, что
//         добавлено через sh, и есть результат.
//   Исключение или ошибка в скрипте -> фигура пустая (крестик в центре),
//   текст ошибки со строкой показывается красным в редакторе.
//   Файл перечитывается автоматически при изменении на диске.
//
// ============================================================================
//  API sh -- построение полилиний
// ============================================================================
//
// Система координат локальная: центр фигуры -- (0, 0), ось X вправо, Y вверх,
// единицы -- мм. Готовая геометрия сдвигается плагином на ручку центра.
// Углы везде в ГРАДУСАХ, отсчёт от оси X против часовой стрелки.
// Направление дуги: ccw = true -- против часовой стрелки, false -- по часовой.
// Прогиб (bulge) -- как в DXF: bulge = tan(θ/4), где θ -- центральный угол
// дуги; знак задаёт сторону: > 0 -- против часовой, < 0 -- по часовой,
// 0 -- прямой отрезок; 1 -- полуокружность.
//
// Готовые примитивы (каждый вызов добавляет отдельную полилинию):
//
//   sh.line(x1, y1, x2, y2)
//       Отрезок из (x1, y1) в (x2, y2). Открытая полилиния из двух вершин.
//
//   sh.circle(cx, cy, diameter)
//       Окружность по центру и ДИАМЕТРУ. Замкнутая, обход против часовой.
//       diameter <= 0 -- игнорируется.
//
//   sh.rectangle(cx, cy, width, height, angleDeg = 0)
//       Прямоугольник по центру и размерам, при angleDeg != 0 повёрнут вокруг
//       своего центра. Замкнутый, обход против часовой.
//
//   sh.arc(cx, cy, radius, startDeg, sweepDeg)
//       Дуга по центру, радиусу, начальному углу и размаху. sweepDeg > 0 --
//       против часовой, < 0 -- по часовой; |sweepDeg| >= 360 даёт замкнутую
//       окружность, начатую с угла startDeg. Открытая полилиния.
//
//   sh.arcBulge(x1, y1, x2, y2, bulge)
//       Одиночная дуга из (x1, y1) в (x2, y2) через прогиб.
//
//   sh.arcC(x1, y1, x2, y2, cx, cy, ccw = true)
//       Одиночная дуга из (x1, y1) в (x2, y2) по центру (cx, cy) и направлению
//       обхода. Радиус берётся до первой точки; вторая должна лежать на той же
//       окружности (иначе дуга приблизительная).
//
//   sh.polyline(points, closed = false)
//       Полилиния из массива вершин. Вершина -- либо [x, y] / [x, y, bulge],
//       либо {x: .., y: .., bulge: ..}. bulge относится к сегменту, ВЫХОДЯЩЕМУ
//       из этой вершины (к следующей; у замкнутой -- из последней в первую).
//       Пример: sh.polyline([[0,0], [10,0,1], [10,10], [0,10]], true)
//               -- квадрат, у которого нижняя правая сторона выгнута
//               полуокружностью наружу.
//
// Построитель пути (удобно для сложных контуров, см. build() ниже):
//
//   var pl = sh.begin(x, y);      -- начать полилинию в точке (x, y)
//   pl.lineTo(x, y);              -- отрезок к точке
//   pl.arcTo(x, y, bulge);        -- дуга к точке через прогиб
//   pl.arcToC(x, y, cx, cy, ccw); -- дуга к точке по центру и направлению
//   pl.arcToR(x, y, +-r, large);  -- дуга к точке по радиусу: знак r -- сторона
//                                    (+ против часовой, - по часовой), large =
//                                    true -- большая дуга (> 180°); |r| меньше
//                                    полухорды поднимается до полуокружности
//   pl.close();                   -- замкнуть контур; если последняя точка
//                                    совпала с первой, дубль убирается, а её
//                                    дуга становится замыкающей
//   pl.end();                     -- оставить контур открытым (необязательно:
//                                    незавершённый путь всё равно попадёт в
//                                    результат)
//   Все методы, кроме close/end, возвращают построитель -- можно писать цепочкой:
//   sh.begin(0, 0).lineTo(10, 0).arcTo(10, 10, 0.5).lineTo(0, 10).close();
//
// Утилиты:
//
//   sh.bulge(x1, y1, x2, y2, cx, cy, ccw = true)
//       Прогиб дуги из первой точки во вторую по центру и направлению --
//       чтобы собирать массивы для sh.polyline().
//   sh.polar(r, deg)              -> {x, y}  точка на радиусе r под углом deg
//   sh.deg2rad(deg), sh.rad2deg(rad)         перевод углов
//
// Отладка: console.log(...) пишет в лог GGEasy.
//
// ============================================================================

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
        value: 20,
        min: 4,
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
    bore: {
        value: 6,
        min: 0,
        max: 1000,
        step: 0.5,
        decimals: 2,
        description: "Bore diameter, mm"
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
    var z = Math.max(4, Math.round(p.teeth));
    var alpha = sh.deg2rad(p.pressureAngle);
    var steps = Math.max(2, Math.round(p.involuteSteps));

    var rp = m * z / 2;            // делительный радиус
    var rb = rp * Math.cos(alpha); // основной радиус
    var ra = rp + m;               // радиус вершин
    var rf = rp - 1.25 * m;        // радиус впадин

    // Эвольвента: t -- параметр развёртки, r(t) = rb*sqrt(1+t^2), полярный угол inv(t) = t - atan(t).
    var tPitch = Math.tan(alpha);
    var invPitch = tPitch - Math.atan(tPitch);
    var halfPitch = Math.PI / (2 * z);       // половина углового шага
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
    // При малом z и большом угле давления зуб на радиусе вершин ra = rp+m
    // уже заострён (фланги сошлись раньше: inv(t)-invPitch = halfPitch), и
    // дуга вершины между «перехлестнувшимися» точками ушла бы на полный круг.
    // Поэтому вершину обрываем на меньшем из ra и радиуса острия.
    var tTipGeom = Math.sqrt((ra / rb) * (ra / rb) - 1);
    var tTipPointed = solveInv(halfPitch + invPitch);
    var tipPointed = tTipPointed < tTipGeom + 1e-6;
    var tTip = tipPointed ? tTipPointed : tTipGeom;
    if (tipPointed) ra = rb * Math.sqrt(1 + tTip * tTip);
    var tStart = rf > rb ? Math.sqrt((rf / rb) * (rf / rb) - 1) : 0;
    var delta = -(halfPitch + invPitch); // поворот эвольвенты: на делительном радиусе угол = -halfPitch

    // Точка фланга по параметру t; side = +1 (сторона зуба по часовой, угол растёт с радиусом) / -1 (зеркало)
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

    // Эволюта эвольвенты окружности -- сама базовая окружность: центр кривизны
    // в параметре t лежит на ней под углом side*(t + delta) + rot (без члена
    // -atan(t) из угла самой точки -- тот отвечает только за угол давления),
    // радиус кривизны rho(t) = rb*t -- длина размотанной нити. Дуга фланга от
    // t0 до t1 берёт центр на середине интервала: сходится к истинной
    // эвольвенте на порядок точнее хорды той же длины.
    function curvatureCenter(t, side, rot) {
        var a = side * (t + delta) + rot;
        return {
            x: rb * Math.cos(a),
            y: rb * Math.sin(a)
        };
    }
    function ccwOf(a, b, c) {
        return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x) > 0;
    }
    // p0 -- уже построенная точка (начало дуги, параметр t0); строит дугу до
    // параметра t1 и возвращает { p: конечная точка, t: t1 } для следующего шага.
    function evolventArc(pl, p0, t0, t1, side, rot) {
        var c = curvatureCenter((t0 + t1) / 2, side, rot);
        var p1 = flank(t1, side, rot);
        pl.arcToC(p1.x, p1.y, c.x, c.y, ccwOf(p0, p1, c));
        return p1;
    }

    var pitchAng = 2 * Math.PI / z;
    var pl = null;

    for (var k = 0; k < z; k++) {
        var rot = k * pitchAng;

        // Корень зуба (со стороны по часовой)
        var start = flank(tStart, +1, rot);
        if (k === 0) {
            if (rf < rb) {
                var s0 = {
                    x: rf * Math.cos(start.ang),
                    y: rf * Math.sin(start.ang)
                };
                pl = sh.begin(s0.x, s0.y);
                pl.lineTo(start.x, start.y);
            } else
                pl = sh.begin(start.x, start.y);
        } else {
            if (rf < rb) {
                var s1 = {
                    x: rf * Math.cos(start.ang),
                    y: rf * Math.sin(start.ang)
                };
                pl.arcToC(s1.x, s1.y, 0, 0, true); // дуга по впадине к началу зуба
                pl.lineTo(start.x, start.y);
            } else
                pl.arcToC(start.x, start.y, 0, 0, true);
        }

        // Восходящий фланг -- дугами по эволюте (см. evolventArc выше)
        var pPrev = start, tPrev = tStart;
        for (var i = 1; i <= steps; i++) {
            var t = tStart + (tTip - tStart) * i / steps;
            pPrev = evolventArc(pl, pPrev, tPrev, t, +1, rot);
            tPrev = t;
        }
        // Вершина: дуга по окружности вершин (при остром зубе фланги уже
        // сошлись в одну точку -- дуги нет)
        var tip2 = flank(tTip, -1, rot);
        if (!tipPointed)
            pl.arcToC(tip2.x, tip2.y, 0, 0, true);
        // Нисходящий фланг
        pPrev = tip2;
        tPrev = tTip;
        for (var j = steps - 1; j >= 0; j--) {
            var t2 = tStart + (tTip - tStart) * j / steps;
            pPrev = evolventArc(pl, pPrev, tPrev, t2, -1, rot);
            tPrev = t2;
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
