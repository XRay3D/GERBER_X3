/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "g2_parser.h"

#include <QRegularExpression>
#include <cmath>

namespace Gerber2 {

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double deg2rad = pi / 180.0;

double toDouble(QStringView s, double def = 0.0) {
    bool ok{};
    double v = s.toDouble(&ok);
    return ok ? v : def;
}

// -----------------------------------------------------------------------------
// Разбиение исходного текста на команды.
// Расширенная команда — «%…%», словарная — «…*».
struct Command {
    bool extended{};
    QStringList words; // для расширенной — все слова между %…%, иначе одно слово
};

std::vector<Command> tokenize(const QString& src) {
    std::vector<Command> cmds;
    const int n = src.size();
    int i = 0;
    auto isSkip = [](QChar c) { return c == u'\n' || c == u'\r'; };
    while(i < n) {
        while(i < n && (isSkip(src[i]) || src[i].isSpace()) && src[i] != u'%') ++i;
        if(i >= n) break;
        if(src[i] == u'%') {
            int end = src.indexOf(u'%', ++i);
            if(end < 0) end = n;
            QString body = src.mid(i, end - i);
            body.remove(u'\n').remove(u'\r');
            Command c{.extended = true};
            for(auto&& w: QStringView{body}.split(u'*', Qt::SkipEmptyParts))
                c.words << w.toString();
            if(!c.words.isEmpty()) cmds.push_back(std::move(c));
            i = end + 1;
        } else {
            int end = src.indexOf(u'*', i);
            if(end < 0) break;
            QString word = src.mid(i, end - i);
            word.remove(u'\n').remove(u'\r');
            if(!word.isEmpty()) cmds.push_back({.extended = false, .words = {word}});
            i = end + 1;
        }
    }
    return cmds;
}

// -----------------------------------------------------------------------------
// Вычислитель арифметических выражений макросов (4.5.4).
class ExprEval {
    QStringView s;
    int pos{};
    const std::map<int, double>& vars;

    void skip() {
        while(pos < s.size() && s[pos].isSpace()) ++pos;
    }
    double primary() {
        skip();
        if(pos >= s.size()) return 0.0;
        if(s[pos] == u'(') {
            ++pos;
            double v = expr();
            skip();
            if(pos < s.size() && s[pos] == u')') ++pos;
            return v;
        }
        if(s[pos] == u'+') return ++pos, primary();
        if(s[pos] == u'-') return ++pos, -primary();
        if(s[pos] == u'$') {
            ++pos;
            int start = pos;
            while(pos < s.size() && s[pos].isDigit()) ++pos;
            int idx = s.sliced(start, pos - start).toInt();
            auto it = vars.find(idx);
            return it == vars.end() ? 0.0 : it->second; // неопределённые = 0
        }
        int start = pos;
        while(pos < s.size() && (s[pos].isDigit() || s[pos] == u'.')) ++pos;
        return toDouble(s.sliced(start, pos - start));
    }
    double term() {
        double v = primary();
        for(;;) {
            skip();
            if(pos < s.size() && (s[pos] == u'x' || s[pos] == u'X'))
                ++pos, v *= primary();
            else if(pos < s.size() && s[pos] == u'/')
                ++pos, v /= primary();
            else
                return v;
        }
    }

public:
    ExprEval(QStringView str, const std::map<int, double>& v)
        : s{str}
        , vars{v} { }
    double expr() {
        double v = term();
        for(;;) {
            skip();
            if(pos < s.size() && s[pos] == u'+')
                ++pos, v += term();
            else if(pos < s.size() && s[pos] == u'-')
                ++pos, v -= term();
            else
                return v;
        }
    }
};

double eval(QStringView e, const std::map<int, double>& vars) {
    return ExprEval{e, vars}.expr();
}

// -----------------------------------------------------------------------------
// Геометрические помощники (только curve.h).

Curves circle(double dia, QPointF c = {}) {
    if(dia <= 0.0) return {};
    return {CircleCurve(dia, c)};
}

Curves rect(double w, double h, QPointF c = {}) {
    if(w <= 0.0 || h <= 0.0) return {};
    return {RectangleCurve(w, h, c)};
}

// Прямоугольник, заданный отрезком и шириной (примитив 20 / трассы).
Curves thickLine(QPointF p1, QPointF p2, double width) {
    QLineF l{p1, p2};
    if(qFuzzyIsNull(l.length()) || width <= 0.0) return {};
    QPointF n = QPointF{-l.dy(), l.dx()} / l.length() * (width * 0.5);
    Curve c{
        geo::Vertex{p1 + n},
        geo::Vertex{p2 + n},
        geo::Vertex{p2 - n},
        geo::Vertex{p1 - n},
        geo::Vertex{p1 + n},
    };
    return {std::move(c)};
}

// Правильный многоугольник по описанной окружности.
Curves regularPolygon(double dia, int vertices, double rotDeg, QPointF c = {}) {
    if(dia <= 0.0 || vertices < 3) return {};
    Curve curve;
    const double r = dia * 0.5;
    for(int i = 0; i <= vertices; ++i) {
        double a = (rotDeg + 360.0 * i / vertices) * deg2rad;
        curve.emplace_back(QPointF{c.x() + r * std::cos(a), c.y() + r * std::sin(a)});
    }
    return {std::move(curve)};
}

Curves obround(double w, double h, QPointF c = {}) {
    if(w <= 0.0 || h <= 0.0) return {};
    if(qFuzzyCompare(w, h)) return circle(w, c);
    const double d = std::min(w, h);
    const double len = std::max(w, h) - d;
    QPointF a = c, b = c;
    if(w > h)
        a.rx() -= len * 0.5, b.rx() += len * 0.5;
    else
        a.ry() -= len * 0.5, b.ry() += len * 0.5;
    Curves line{
        Curve{geo::Vertex{a}, geo::Vertex{b}}
    };
    // Inflate трактует delta как диаметр (радиус = delta/2)
    return Inflate(line, d, JoinType::Round, EndType::Round);
}

Curves subtractHole(Curves body, double holeDia) {
    if(holeDia <= 0.0 || body.empty()) return body;
    return BoolOp.Difference(body, circle(holeDia), FillRule::NonZero);
}

// -----------------------------------------------------------------------------

class Parser {
public:
    ParseResult res;

    explicit Parser(const QString& src) { run(src); }

private:
    State st;
    Objects top;
    std::vector<Objects*> sinks{&top};
    Objects* sink() { return sinks.back(); }

    // регион
    Curves regionContours;
    Curve regionCurve;

    // блок-апертура: код + собственное хранилище объектов блока
    std::vector<std::pair<int, std::unique_ptr<Objects>>> abStack;

    // step & repeat
    bool inSr{};
    Objects srObjects;
    int srX = 1, srY = 1;
    double srI{}, srJ{};

    void warn(const QString& w) {
        if(res.warnings.size() < 64 && !res.warnings.contains(w)) res.warnings << w;
    }

    // ---------------------------------------------------------------- команды
    void run(const QString& src) {
        for(const Command& cmd: tokenize(src)) {
            if(cmd.extended)
                extended(cmd.words);
            else
                word(cmd.words.front());
        }
        if(res.format.xDec == 0) res.error = QObject::tr("Missing FS command");
        res.objects = std::move(top);
    }

    void extended(const QStringList& words) {
        const QString& head = words.front();
        auto is = [&](const char* p) { return head.startsWith(QLatin1String{p}); };
        if(is("FS")) return parseFS(head);
        if(is("MO")) return parseMO(head);
        if(is("AM")) return parseAM(words);
        if(is("AD")) return parseAD(head);
        if(is("LP")) {
            st.polarity = head.mid(2).startsWith(u'C') ? Polarity::Clear : Polarity::Dark;
            return;
        }
        if(is("LM")) {
            QStringView m = QStringView{head}.mid(2);
            st.tr.mirror = m.startsWith(u"XY") ? Mirror::XY
                : m.startsWith(u'X')           ? Mirror::X
                : m.startsWith(u'Y')           ? Mirror::Y
                                               : Mirror::None;
            return;
        }
        if(is("LR")) return void(st.tr.rotation = toDouble(QStringView{head}.mid(2)));
        if(is("LS")) return void(st.tr.scale = toDouble(QStringView{head}.mid(2), 1.0));
        if(is("AB")) return parseAB(head);
        if(is("SR")) return parseSR(head);
        if(is("TF") || is("TA") || is("TO") || is("TD")) return; // атрибуты не влияют на изображение
        if(is("IN") || is("LN") || is("IP") || is("AS") || is("IR") || is("MI") || is("OF") || is("SF")) {
            warn(QObject::tr("Deprecated command ignored: %1").arg(head.left(2)));
            return;
        }
        warn(QObject::tr("Unknown command: %1").arg(head.left(8)));
    }

    void word(const QString& w) {
        if(w.startsWith(u"G04")) return;
        if(w.startsWith(u"M0")) return; // M00/M01/M02
        if(w == u"G36") {
            st.region = true;
            regionContours.clear();
            regionCurve.clear();
            return;
        }
        if(w == u"G37") return closeRegion();
        if(w == u"G01" || w == u"G1") return void(st.plot = PlotMode::Linear);
        if(w == u"G02" || w == u"G2") return void(st.plot = PlotMode::Cw);
        if(w == u"G03" || w == u"G3") return void(st.plot = PlotMode::Ccw);
        if(w == u"G74") return void(st.multiQuadrant = false);
        if(w == u"G75") return void(st.multiQuadrant = true);
        if(w == u"G70") return void(res.format.unit = Unit::Inches);
        if(w == u"G71") return void(res.format.unit = Unit::Millimeters);
        if(w == u"G90") return void(res.format.notation = Notation::Absolute);
        if(w == u"G91") return void(res.format.notation = Notation::Incremental);
        if(w.startsWith(u"G54") || w.startsWith(u"G55")) { // устар., без эффекта
            if(w.size() > 3) return operation(w.mid(3));
            return;
        }
        // Устаревшая форма «G01X…D01» — режим и операция в одном слове (8.3.1).
        if(w.size() > 3 && w.startsWith(u'G')) {
            if(w.startsWith(u"G01"))
                st.plot = PlotMode::Linear;
            else if(w.startsWith(u"G02"))
                st.plot = PlotMode::Cw;
            else if(w.startsWith(u"G03"))
                st.plot = PlotMode::Ccw;
            else
                return warn(QObject::tr("Unknown command: %1").arg(w.left(8)));
            return operation(w.mid(3));
        }
        operation(w);
    }

    // ------------------------------------------------------- FS / MO / AD / AM
    void parseFS(const QString& w) {
        static const QRegularExpression re{uR"(^FS([LT])?([AI])?X(\d)(\d)Y(\d)(\d))"_s};
        auto m = re.match(w);
        if(!m.hasMatch()) {
            res.error = QObject::tr("Bad FS command: %1").arg(w);
            return;
        }
        auto& f = res.format;
        f.zeros = m.captured(1) == u"T" ? Zeros::OmitTrailing : Zeros::OmitLeading;
        f.notation = m.captured(2) == u"I" ? Notation::Incremental : Notation::Absolute;
        f.xInt = m.captured(3).toInt();
        f.xDec = m.captured(4).toInt();
        f.yInt = m.captured(5).toInt();
        f.yDec = m.captured(6).toInt();
    }

    void parseMO(const QString& w) {
        res.format.unit = QStringView{w}.mid(2).startsWith(u"IN") ? Unit::Inches : Unit::Millimeters;
    }

    void parseAM(const QStringList& words) {
        Macro macro;
        macro.name = words.front().mid(2);
        macro.body = words | v::drop(1) | r::to<std::vector<QString>>();
        res.macros[macro.name] = std::move(macro);
    }

    void parseAD(const QString& w) {
        static const QRegularExpression re{uR"(^ADD(\d+)([^,]+)(?:,(.*))?$)"_s};
        auto m = re.match(w);
        if(!m.hasMatch()) return warn(QObject::tr("Bad AD command: %1").arg(w));

        const int code = m.captured(1).toInt();
        const QString tmpl = m.captured(2);

        // Параметры шаблона: raw — как в файле, p — пересчитанные в мм.
        // Углы и счётчики вершин длинами не являются, поэтому берутся из raw.
        const std::vector<double> raw = m.hasCaptured(3)
            ? QStringView{m.capturedView(3)}.split(u'X', Qt::SkipEmptyParts)
                | v::transform([](QStringView s) { return toDouble(s); })
                | r::to<std::vector>()
            : std::vector<double>{};
        const auto p = raw
            | v::transform([&](double v) { return res.format.lenToMm(v); })
            | r::to<std::vector>();

        auto at = [&](size_t i, double def = 0.0) { return i < p.size() ? p[i] : def; };
        auto atRaw = [&](size_t i, double def = 0.0) { return i < raw.size() ? raw[i] : def; };
        auto ap = std::make_shared<Aperture>();
        ap->source = u'%' + w + u"*%"_s;

        if(tmpl == u"C") {
            ap->body = circle(at(0));
            ap->holeDia = at(1);
        } else if(tmpl == u"R") {
            ap->body = rect(at(0), at(1));
            ap->holeDia = at(2);
        } else if(tmpl == u"O") {
            ap->body = obround(at(0), at(1));
            ap->holeDia = at(2);
        } else if(tmpl == u"P") {
            ap->body = regularPolygon(at(0), int(std::lround(atRaw(1))), atRaw(2));
            ap->holeDia = at(3);
        } else if(auto it = res.macros.find(tmpl); it != res.macros.end()) {
            ap->body = buildMacro(it->second, raw);
        } else {
            return warn(QObject::tr("Unknown aperture template: %1").arg(tmpl));
        }
        ap->body = subtractHole(std::move(ap->body), ap->holeDia);
        res.apertures[code] = std::move(ap);
    }

    // -------------------------------------------------------------- AB и SR
    void parseAB(const QString& w) {
        if(w.size() <= 2) { // %AB*% — закрытие
            if(abStack.empty()) return warn(QObject::tr("Unmatched AB close"));
            auto [code, storage] = std::move(abStack.back());
            abStack.pop_back();
            if(!sinks.empty() && sinks.back() == storage.get()) sinks.pop_back();
            auto ap = std::make_shared<Aperture>();
            ap->isBlock = true;
            ap->block = std::move(*storage);
            res.apertures[code] = std::move(ap);
            return;
        }
        bool ok{};
        int code = QStringView{w}.mid(3).toInt(&ok); // ABD<nn>
        if(!ok) return warn(QObject::tr("Bad AB command: %1").arg(w));
        auto& [_, storage] = abStack.emplace_back(code, std::make_unique<Objects>());
        sinks.push_back(storage.get());
    }

    void parseSR(const QString& w) {
        if(w.size() <= 2) { // %SR*% — закрытие и размножение
            if(!inSr) return;
            inSr = false;
            // Сначала снимаем приёмник: копии кладутся уже наружу, иначе мы бы
            // дописывали в тот же вектор, который перебираем.
            const Objects block = std::move(srObjects);
            srObjects.clear();
            sinks.pop_back();
            // Копии укладываются сперва по Y, затем по X (4.12).
            for(auto [ix, iy]: v::cartesian_product(v::iota(0, srX), v::iota(0, srY))) {
                const QTransform tr = QTransform::fromTranslate(srI * ix, srJ * iy);
                for(const Object& o: block) {
                    Object copy = o;
                    TransformCurves(copy.curves, tr);
                    sink()->push_back(std::move(copy));
                }
            }
            return;
        }
        static const QRegularExpression re{uR"(^SRX(\d+)Y(\d+)I([\d.+-]+)J([\d.+-]+))"_s};
        auto m = re.match(w);
        if(!m.hasMatch()) return warn(QObject::tr("Bad SR command: %1").arg(w));
        srX = std::max(1, m.captured(1).toInt());
        srY = std::max(1, m.captured(2).toInt());
        srI = res.format.lenToMm(m.captured(3).toDouble());
        srJ = res.format.lenToMm(m.captured(4).toDouble());
        inSr = true;
        srObjects.clear();
        sinks.push_back(&srObjects);
    }

    void addObject(Curves curves) {
        if(curves.empty()) return;
        sink()->push_back({std::move(curves), st.polarity});
    }

    // -------------------------------------------------------------- операции
    void operation(const QString& w) {
        std::optional<double> X, Y, I, J;
        int dcode = -1;
        const int n = w.size();
        for(int i = 0; i < n;) {
            QChar c = w[i];
            if(!c.isLetter()) { ++i; continue; }
            int start = ++i;
            while(i < n && (w[i].isDigit() || w[i] == u'+' || w[i] == u'-' || w[i] == u'.')) ++i;
            QStringView num = QStringView{w}.sliced(start, i - start);
            if(num.isEmpty()) continue;
            switch(c.unicode()) {
            case u'X': X = res.format.x(num); break;
            case u'Y': Y = res.format.y(num); break;
            case u'I': I = res.format.x(num); break;
            case u'J': J = res.format.y(num); break;
            case u'D': dcode = num.toInt(); break;
            default  : break;
            }
        }
        if(dcode >= 10) { // выбор апертуры
            st.aperture = dcode;
            return;
        }

        QPointF pt = st.current;
        if(res.format.notation == Notation::Absolute) {
            if(X) pt.rx() = *X;
            if(Y) pt.ry() = *Y;
        } else {
            if(X) pt.rx() += *X;
            if(Y) pt.ry() += *Y;
        }

        switch(dcode) {
        case 1: plot(pt, I, J); break;
        case 2:
            if(st.region) closeContour();
            st.current = pt;
            break;
        case 3: flash(pt); break;
        default: st.current = pt; break; // координаты без D-кода (устар.)
        }
    }

    void plot(QPointF to, std::optional<double> I, std::optional<double> J) {
        Curve seg{
            geo::Vertex{st.current}
        };
        if(st.plot == PlotMode::Linear || !I || !J) {
            seg.emplace_back(to);
        } else {
            QPointF center = arcCenter(st.current, to, *I, *J);
            auto type = st.plot == PlotMode::Cw ? geo::Vertex::Cw : geo::Vertex::Ccw;
            if(st.multiQuadrant && QLineF{st.current, to}.length() < 1e-9) {
                // start == end → полная окружность, делим пополам
                QPointF mid = center * 2.0 - st.current;
                seg.emplace_back(mid, center, type);
                seg.emplace_back(to, center, type);
            } else {
                seg.emplace_back(to, center, type);
            }
        }

        if(st.region) {
            if(regionCurve.empty()) regionCurve.push_back(seg.front());
            for(size_t k = 1; k < seg.size(); ++k) regionCurve.push_back(seg[k]);
        } else {
            res.strokes.push_back(seg);
            addObject(stroke(seg));
        }
        st.current = to;
    }

    QPointF arcCenter(QPointF from, QPointF to, double i, double j) const {
        if(st.multiQuadrant) return from + QPointF{i, j};
        // одноквадрантный режим: знаки смещений опущены (8.1.10)
        const double ai = std::abs(i), aj = std::abs(j);
        QPointF best;
        double bestErr = std::numeric_limits<double>::max();
        for(double si: {+1.0, -1.0})
            for(double sj: {+1.0, -1.0}) {
                QPointF c = from + QPointF{ai * si, aj * sj};
                double r1 = QLineF{c, from}.length();
                double r2 = QLineF{c, to}.length();
                double a1 = std::atan2(from.y() - c.y(), from.x() - c.x());
                double a2 = std::atan2(to.y() - c.y(), to.x() - c.x());
                double sweep = a2 - a1;
                if(st.plot == PlotMode::Cw) sweep = -sweep;
                while(sweep < 0) sweep += 2 * pi;
                if(sweep > pi / 2 + 1e-6) continue; // не более 90°
                double err = std::abs(r1 - r2);
                if(err < bestErr) bestErr = err, best = c;
            }
        return bestErr == std::numeric_limits<double>::max() ? from + QPointF{ai, aj} : best;
    }

    double strokeDia() const {
        auto it = res.apertures.find(st.aperture);
        if(it == res.apertures.end() || !it->second) return 0.0;
        QRectF r = BoundingRect(it->second->body);
        return std::min(r.width(), r.height());
    }

    Curves stroke(const Curve& seg) const {
        double dia = strokeDia() * st.tr.scale;
        if(dia <= 0.0) return {}; // апертура нулевого размера изображения не даёт
        return Inflate(Curves{seg}, dia, JoinType::Round, EndType::Round);
    }

    void closeContour() {
        if(regionCurve.size() > 2) {
            if(QLineF{regionCurve.front().pt, regionCurve.back().pt}.length() > 1e-9)
                regionCurve.push_back(regionCurve.front()); // страховка от незамкнутости
            regionContours.push_back(regionCurve);
        }
        regionCurve.clear();
    }

    void closeRegion() {
        closeContour();
        st.region = false;
        if(regionContours.empty()) return;
        addObject(BoolOp.Union(regionContours, FillRule::NonZero));
        regionContours.clear();
    }

    void flash(QPointF pos) {
        st.current = pos;
        auto it = res.apertures.find(st.aperture);
        if(it == res.apertures.end() || !it->second)
            return warn(QObject::tr("Flash with undefined aperture D%1").arg(st.aperture));

        const Aperture& ap = *it->second;
        QTransform tr = st.tr.toQTransform();
        tr *= QTransform::fromTranslate(pos.x(), pos.y());

        if(ap.isBlock) {
            // Полярность блока инвертируется, если он вставляется в режиме LPC (4.11.1)
            const bool invert = st.polarity == Polarity::Clear;
            for(const Object& o: ap.block) {
                Object copy = o;
                TransformCurves(copy.curves, tr);
                if(invert)
                    copy.polarity = copy.polarity == Polarity::Dark ? Polarity::Clear : Polarity::Dark;
                sink()->push_back(std::move(copy));
            }
            return;
        }
        Curves body = ap.body;
        TransformCurves(body, tr);
        addObject(std::move(body));
    }

    // ----------------------------------------------------------------- макрос
    Curves buildMacro(const Macro& macro, const std::vector<double>& args) {
        // $1..$n — параметры вызывающей команды AD (4.5.4.1)
        auto vars = v::zip(v::iota(1), args) | r::to<std::map<int, double>>();

        Curves result;
        auto apply = [&](Curves shape, bool exposureOn) {
            if(shape.empty()) return;
            if(exposureOn)
                result = result.empty() ? std::move(shape)
                                        : BoolOp.Union(result, shape, FillRule::NonZero);
            else if(!result.empty())
                result = BoolOp.Difference(result, shape, FillRule::NonZero);
        };

        for(const QString& wordStr: macro.body) {
            QStringView w = QStringView{wordStr}.trimmed();
            if(w.isEmpty()) continue;
            if(w.startsWith(u'$')) { // определение переменной
                int eq = w.indexOf(u'=');
                if(eq < 0) continue;
                int idx = w.sliced(1, eq - 1).toInt();
                vars[idx] = eval(w.sliced(eq + 1), vars);
                continue;
            }
            auto parts = w.split(u',');
            if(parts.isEmpty()) continue;
            const int code = parts.front().trimmed().toInt();
            auto arg = [&](int i, double def = 0.0) {
                return i < parts.size() ? res.format.lenToMm(eval(parts[i], vars)) : def;
            };
            auto raw = [&](int i, double def = 0.0) { // угол/счётчик — без пересчёта единиц
                return i < parts.size() ? eval(parts[i], vars) : def;
            };
            auto on = [&](int i) { return i < parts.size() && eval(parts[i], vars) > 0.5; };

            Curves shape;
            double rot = 0.0;
            switch(code) {
            case 0: continue; // комментарий
            case 1:           // окружность
                shape = circle(arg(2), {arg(3), arg(4)});
                rot = raw(5);
                break;
            case 2:
            case 20: // векторная линия
                shape = thickLine({arg(3), arg(4)}, {arg(5), arg(6)}, arg(2));
                rot = raw(7);
                break;
            case 21: // центрированная линия
                shape = rect(arg(2), arg(3), {arg(4), arg(5)});
                rot = raw(6);
                break;
            case 22: // линия по левому нижнему углу (устар.)
                shape = rect(arg(2), arg(3), {arg(4) + arg(2) * 0.5, arg(5) + arg(3) * 0.5});
                rot = raw(6);
                break;
            case 4: { // контур
                int nv = int(std::lround(raw(2)));
                Curve c;
                for(int k = 0; k <= nv && 3 + k * 2 + 1 < parts.size(); ++k)
                    c.emplace_back(QPointF{arg(3 + k * 2), arg(4 + k * 2)});
                if(c.size() > 2) {
                    if(QLineF{c.front().pt, c.back().pt}.length() > 1e-9) c.push_back(c.front());
                    shape = {std::move(c)};
                }
                rot = raw(5 + nv * 2);
            } break;
            case 5: // правильный многоугольник
                shape = regularPolygon(arg(5), int(std::lround(raw(2))), 0.0, {arg(3), arg(4)});
                rot = raw(6);
                break;
            case 7: { // термо-примитив: кольцо с четырьмя разрывами
                QPointF c{arg(1), arg(2)};
                double od = arg(3), id = arg(4), gap = arg(5);
                rot = raw(6);
                if(od <= 0.0) break;
                shape = BoolOp.Difference(circle(od, c), circle(id, c), FillRule::NonZero);
                Curves cross = rect(gap, od * 1.5, c);
                for(auto&& cc: rect(od * 1.5, gap, c)) cross.push_back(std::move(cc));
                shape = BoolOp.Difference(shape, cross, FillRule::NonZero);
            } break;
            case 6:
                warn(QObject::tr("Deprecated macro primitive 6 (moiré) is not supported"));
                continue;
            default:
                warn(QObject::tr("Unknown macro primitive: %1").arg(code));
                continue;
            }
            if(!qFuzzyIsNull(rot))
                for(auto&& c: shape) RotateCurve(c, rot);
            // Примитив 7 всегда с включённой экспозицией
            apply(std::move(shape), code == 7 ? true : on(1));
        }
        return result;
    }
};

} // namespace

// -----------------------------------------------------------------------------

double Format::toMm(QStringView token, int intDigits, int decDigits) const {
    if(token.isEmpty()) return 0.0;
    const bool neg = token.front() == u'-';
    if(token.front() == u'+' || token.front() == u'-') token = token.sliced(1);

    auto digits = token | v::filter([](QChar c) { return c.isDigit(); }) | r::to<std::vector>();
    if(digits.empty()) return 0.0;

    // При опущенных хвостовых нулях строка дополняется справа до полной длины.
    const size_t total = size_t(intDigits + decDigits);
    if(zeros == Zeros::OmitTrailing && digits.size() < total)
        digits.resize(total, u'0');

    double v = r::fold_left(digits, 0.0,
        [](double acc, QChar c) { return acc * 10.0 + (c.unicode() - u'0'); });
    v /= std::pow(10.0, decDigits);
    if(neg) v = -v;
    return unit == Unit::Inches ? v * 25.4 : v;
}

Curves flatten(const Objects& objects) {
    Curves result;
    // Подряд идущие объекты одной полярности объединяются за одну операцию.
    auto samePolarity = [](const Object& l, const Object& r) { return l.polarity == r.polarity; };
    for(auto&& group: objects | v::chunk_by(samePolarity)) {
        Curves batch = group
            | v::transform(&Object::curves)
            | v::join
            | r::to<Curves>();
        if(batch.empty()) continue;
        batch = BoolOp.Union(batch, FillRule::NonZero);
        if(r::begin(group)->polarity == Polarity::Dark)
            result = result.empty() ? std::move(batch)
                                    : BoolOp.Union(result, batch, FillRule::NonZero);
        else if(!result.empty())
            result = BoolOp.Difference(result, batch, FillRule::NonZero);
    }
    return result;
}

ParseResult parse(const QString& source) {
    Parser p{source};
    return std::move(p.res);
}

} // namespace Gerber2
