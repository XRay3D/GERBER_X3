/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "svg_parser.h"
#include "svg_file.h"

#include <QFile>
#include <QRegularExpression>
#include <cmath>
#include <numbers>

namespace Svg {

static constexpr double pxToMm = 25.4 / 96.0;

// ─── number helpers ───────────────────────────────────────────────────────────

static void skipWs(QStringView s, int& i) {
    while(i < (int)s.size() && (s[i] == u' ' || s[i] == u',' || s[i] == u'\t' || s[i] == u'\n' || s[i] == u'\r'))
        ++i;
}

static double readNum(QStringView s, int& i) {
    skipWs(s, i);
    int start = i;
    if(i < (int)s.size() && (s[i] == u'-' || s[i] == u'+')) ++i;
    while(i < (int)s.size()) {
        QChar c = s[i];
        if(c.isDigit() || c == u'.') {
            ++i;
            continue;
        }
        if((c == u'e' || c == u'E') && i + 1 < (int)s.size()) {
            QChar next = s[i + 1];
            if(next.isDigit() || next == u'-' || next == u'+') {
                ++i;
                continue;
            }
        }
        break;
    }
    if(i == start) return 0.0;
    return s.mid(start, i - start).toDouble();
}

static bool hasMore(QStringView s, int& i) {
    skipWs(s, i);
    if(i >= (int)s.size()) return false;
    QChar c = s[i];
    return c.isDigit() || c == u'-' || c == u'+' || c == u'.';
}

// ─── length parsing ───────────────────────────────────────────────────────────

double Parser::parseLength(QStringView str, double viewportDim) {
    if(str.isEmpty()) return 0.0;
    auto s = str.trimmed();
    if(s.endsWith(u"mm")) return s.left(s.size() - 2).toDouble();
    if(s.endsWith(u"cm")) return s.left(s.size() - 2).toDouble() * 10.0;
    if(s.endsWith(u"in")) return s.left(s.size() - 2).toDouble() * 25.4;
    if(s.endsWith(u"pt")) return s.left(s.size() - 2).toDouble() * (25.4 / 72.0);
    if(s.endsWith(u"pc")) return s.left(s.size() - 2).toDouble() * (25.4 / 6.0);
    if(s.endsWith(u"px")) return s.left(s.size() - 2).toDouble() * pxToMm;
    if(s.endsWith(u"%")) return s.left(s.size() - 1).toDouble() / 100.0 * viewportDim;
    return s.toDouble() * pxToMm; // unitless = px
}

// ─── transform parsing ────────────────────────────────────────────────────────

QTransform Parser::parseTransform(QStringView str) {
    QTransform result;
    auto s = str.trimmed();
    int i = 0;
    while(i < (int)s.size()) {
        skipWs(s, i);
        if(i >= (int)s.size()) break;

        int nameStart = i;
        while(i < (int)s.size() && s[i] != u'(') ++i;
        auto name = s.mid(nameStart, i - nameStart).trimmed();
        if(i >= (int)s.size()) break;
        ++i; // skip '('

        QList<double> args;
        while(i < (int)s.size() && s[i] != u')') {
            skipWs(s, i);
            if(i < (int)s.size() && s[i] != u')')
                args.append(readNum(s, i));
        }
        if(i < (int)s.size()) ++i; // skip ')'

        QTransform t;
        if(name == u"translate") {
            t.translate(args.value(0), args.value(1, 0.0));
        } else if(name == u"scale") {
            double sx = args.value(0, 1.0);
            double sy = args.size() >= 2 ? args[1] : sx;
            t.scale(sx, sy);
        } else if(name == u"rotate") {
            double a = args.value(0);
            if(args.size() >= 3) {
                double cx = args[1], cy = args[2];
                t.translate(cx, cy);
                t.rotate(a);
                t.translate(-cx, -cy);
            } else {
                t.rotate(a);
            }
        } else if(name == u"skewX") {
            t.shear(std::tan(args.value(0) * std::numbers::pi / 180.0), 0.0);
        } else if(name == u"skewY") {
            t.shear(0.0, std::tan(args.value(0) * std::numbers::pi / 180.0));
        } else if(name == u"matrix" && args.size() >= 6) {
            // SVG matrix(a,b,c,d,e,f) maps to Qt matrix
            t.setMatrix(args[0], args[1], 0, args[2], args[3], 0, args[4], args[5], 1);
        }
        // Qt uses right-multiplication: result = t * result means t is applied first
        result = t * result;
    }
    return result;
}

// ─── points parsing ───────────────────────────────────────────────────────────

QPolygonF Parser::parsePoints(QStringView str) {
    QPolygonF poly;
    int i = 0;
    while(i < (int)str.size()) {
        if(!hasMore(str, i)) break;
        double x = readNum(str, i);
        if(!hasMore(str, i)) break;
        double y = readNum(str, i);
        poly.append({x, y});
    }
    return poly;
}

// ─── SVG arc ──────────────────────────────────────────────────────────────────

void Parser::svgArcTo(QPainterPath& path, QPointF p0, double rx, double ry,
    double xRot, bool largeArc, bool sweep, QPointF p1) {
    if(rx == 0.0 || ry == 0.0 || p0 == p1) {
        path.lineTo(p1);
        return;
    }
    rx = std::abs(rx);
    ry = std::abs(ry);

    const double phi = xRot * std::numbers::pi / 180.0;
    const double cosPhi = std::cos(phi);
    const double sinPhi = std::sin(phi);

    const double dx2 = (p0.x() - p1.x()) / 2.0;
    const double dy2 = (p0.y() - p1.y()) / 2.0;
    const double x1p = +cosPhi * dx2 + sinPhi * dy2;
    const double y1p = -sinPhi * dx2 + cosPhi * dy2;

    // Ensure radii are large enough
    double lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if(lambda > 1.0) {
        lambda = std::sqrt(lambda);
        rx *= lambda;
        ry *= lambda;
    }

    const double rx2 = rx * rx, ry2 = ry * ry;
    const double x1p2 = x1p * x1p, y1p2 = y1p * y1p;
    double sq = std::max(0.0, (rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2) / (rx2 * y1p2 + ry2 * x1p2));
    double coef = std::sqrt(sq);
    if(largeArc == sweep) coef = -coef;

    const double cxp = +coef * rx * y1p / ry;
    const double cyp = -coef * ry * x1p / rx;
    const double cx = cosPhi * cxp - sinPhi * cyp + (p0.x() + p1.x()) / 2.0;
    const double cy = sinPhi * cxp + cosPhi * cyp + (p0.y() + p1.y()) / 2.0;

    auto vecAngle = [](double ux, double uy, double vx, double vy) {
        double n = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
        double c = std::clamp((ux * vx + uy * vy) / n, -1.0, 1.0);
        double a = std::acos(c) * 180.0 / std::numbers::pi;
        if(ux * vy - uy * vx < 0.0) a = -a;
        return a;
    };

    const double ux = (x1p - cxp) / rx, uy = (y1p - cyp) / ry;
    const double vx = (-x1p - cxp) / rx, vy = (-y1p - cyp) / ry;

    double startAngle = vecAngle(1, 0, ux, uy);
    double dAngle = vecAngle(ux, uy, vx, vy);
    if(!sweep && dAngle > 0) dAngle -= 360.0;
    if(sweep && dAngle < 0) dAngle += 360.0;

    QRectF rect(cx - rx, cy - ry, 2 * rx, 2 * ry);
    if(qFuzzyIsNull(phi)) {
        path.arcTo(rect, -startAngle, -dAngle);
    } else {
        QTransform rot;
        rot.translate(cx, cy);
        rot.rotate(xRot);
        rot.translate(-cx, -cy);
        QPainterPath tmp;
        tmp.moveTo(p0);
        tmp.arcTo(rect, -startAngle, -dAngle);
        path.addPath(rot.map(tmp));
    }
}

// ─── SVG path d-attribute parser ─────────────────────────────────────────────

QPainterPath Parser::parseSvgPathData(QStringView d) {
    QPainterPath path;
    auto s = d.trimmed();
    int i = 0;

    QPointF cur{}, subStart{}, lastCtrl{};
    char lastCmd = 0;

    while(i < (int)s.size()) {
        skipWs(s, i);
        if(i >= (int)s.size()) break;

        char c = s[i].toLatin1();
        bool isCmd = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        if(isCmd) {
            lastCmd = c;
            ++i;
        } else if(lastCmd == 0) {
            ++i;
            continue;
        }

        const bool rel = (lastCmd >= 'a');
        auto absP = [&](double x, double y) { return rel ? QPointF{cur.x() + x, cur.y() + y} : QPointF{x, y}; };
        auto absX = [&](double x) { return rel ? cur.x() + x : x; };
        auto absY = [&](double y) { return rel ? cur.y() + y : y; };

        char upper = std::toupper(lastCmd);

        switch(upper) {
        case 'M': {
            QPointF p = absP(readNum(s, i), readNum(s, i));
            path.moveTo(p);
            cur = subStart = p;
            // Subsequent coords are implicit L/l
            lastCmd = rel ? 'l' : 'L';
            while(hasMore(s, i)) {
                p = absP(readNum(s, i), readNum(s, i));
                path.lineTo(p);
                cur = p;
            }
            break;
        }
        case 'Z':
            path.closeSubpath();
            cur = subStart;
            break;
        case 'L':
            do {
                QPointF p = absP(readNum(s, i), readNum(s, i));
                path.lineTo(p);
                cur = p;
            } while(hasMore(s, i));
            break;
        case 'H':
            do {
                double x = absX(readNum(s, i));
                path.lineTo(x, cur.y());
                cur.setX(x);
            } while(hasMore(s, i));
            break;
        case 'V':
            do {
                double y = absY(readNum(s, i));
                path.lineTo(cur.x(), y);
                cur.setY(y);
            } while(hasMore(s, i));
            break;
        case 'C':
            do {
                QPointF c1 = absP(readNum(s, i), readNum(s, i));
                QPointF c2 = absP(readNum(s, i), readNum(s, i));
                QPointF ep = absP(readNum(s, i), readNum(s, i));
                path.cubicTo(c1, c2, ep);
                lastCtrl = c2;
                cur = ep;
            } while(hasMore(s, i));
            break;
        case 'S':
            do {
                QPointF c1 = 2 * cur - lastCtrl;
                QPointF c2 = absP(readNum(s, i), readNum(s, i));
                QPointF ep = absP(readNum(s, i), readNum(s, i));
                path.cubicTo(c1, c2, ep);
                lastCtrl = c2;
                cur = ep;
            } while(hasMore(s, i));
            break;
        case 'Q':
            do {
                QPointF c1 = absP(readNum(s, i), readNum(s, i));
                QPointF ep = absP(readNum(s, i), readNum(s, i));
                path.quadTo(c1, ep);
                lastCtrl = c1;
                cur = ep;
            } while(hasMore(s, i));
            break;
        case 'T':
            do {
                QPointF c1 = 2 * cur - lastCtrl;
                QPointF ep = absP(readNum(s, i), readNum(s, i));
                path.quadTo(c1, ep);
                lastCtrl = c1;
                cur = ep;
            } while(hasMore(s, i));
            break;
        case 'A':
            do {
                double rx = readNum(s, i), ry = readNum(s, i);
                double xRot = readNum(s, i);
                double la = readNum(s, i), sw = readNum(s, i);
                QPointF ep = absP(readNum(s, i), readNum(s, i));
                svgArcTo(path, cur, rx, ry, xRot, la != 0.0, sw != 0.0, ep);
                cur = ep;
            } while(hasMore(s, i));
            break;
        default:
            ++i;
            break;
        }

        if(upper != 'C' && upper != 'S' && upper != 'Q' && upper != 'T')
            lastCtrl = cur;
    }
    return path;
}

// ─── main file parse ──────────────────────────────────────────────────────────

bool Parser::parseFile(const QString& fileName, File* file) {
    file_ = file;

    QFile f{fileName};
    if(!f.open(QFile::ReadOnly)) return false;
    QXmlStreamReader xml{&f};

    while(!xml.atEnd()) {
        xml.readNext();
        if(!xml.isStartElement() || xml.name() != u"svg") continue;

        auto attrs = xml.attributes();
        const QString widthStr = attrs.value(u"width").toString();
        const QString heightStr = attrs.value(u"height").toString();
        const QString viewBoxStr = attrs.value(u"viewBox").toString();

        double vbX = 0, vbY = 0, vbW = 0, vbH = 0;
        bool hasViewBox = false;

        if(!viewBoxStr.isEmpty()) {
            QStringList parts = viewBoxStr.split(QRegularExpression(u"[,\\s]+"_s), Qt::SkipEmptyParts);
            if(parts.size() >= 4) {
                vbX = parts[0].toDouble();
                vbY = parts[1].toDouble();
                vbW = parts[2].toDouble();
                vbH = parts[3].toDouble();
                hasViewBox = (vbW > 0 && vbH > 0);
            }
        }

        const double vpWmm = parseLength(widthStr);
        const double vpHmm = parseLength(heightStr);

        double scale = pxToMm;
        if(hasViewBox && vpWmm > 0 && vbW > 0)
            scale = vpWmm / vbW;
        else if(hasViewBox && vpHmm > 0 && vbH > 0)
            scale = vpHmm / vbH;

        // Base transform: translate viewBox origin, scale to mm
        // Y stays positive (downward) for now; Y-flip happens per element
        toMm_ = QTransform{}.translate(-vbX, -vbY).scale(scale, scale);

        // Start transform for direct children of <svg>
        QTransform rootTransform = toMm_;
        const QString svgTr = attrs.value(u"transform").toString();
        if(!svgTr.isEmpty())
            rootTransform = parseTransform(svgTr) * toMm_;

        while(!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement())
                parseElement(xml, rootTransform);
            else if(xml.isEndElement() && xml.name() == u"svg")
                break;
        }
        break;
    }
    return !xml.hasError();
}

// ─── element dispatch ─────────────────────────────────────────────────────────

void Parser::parseElement(QXmlStreamReader& xml, const QTransform& parentTransform) {
    const auto tag = xml.name();
    const auto attrs = xml.attributes();

    // Accumulate local transform: element's own SVG transform on top of parent
    QTransform localTransform = parentTransform;
    const QString transformStr = attrs.value(u"transform").toString();
    if(!transformStr.isEmpty())
        localTransform = parseTransform(transformStr) * parentTransform;

    // Y-flip: SVG Y-down → project Y-up
    // Applied as the last step when mapping element coordinates to mm
    static const QTransform yFlip = QTransform{}.scale(1.0, -1.0);
    const QTransform toMmYUp = localTransform * yFlip;

    const QString id = attrs.value(u"id").toString();

    auto addPath = [&](QPainterPath pPath) {
        if(pPath.isEmpty()) return;
        *file_ << SvgElement{toMmYUp.map(pPath), id};
    };

    bool isContainer = (tag == u"g" || tag == u"svg" || tag == u"symbol");
    bool isDefs = (tag == u"defs");

    if(!isContainer && !isDefs) {
        if(tag == u"line") {
            QPainterPath p;
            p.moveTo(attrs.value(u"x1").toDouble(), attrs.value(u"y1").toDouble());
            p.lineTo(attrs.value(u"x2").toDouble(), attrs.value(u"y2").toDouble());
            addPath(p);
        } else if(tag == u"rect") {
            const double x = attrs.value(u"x").toDouble();
            const double y = attrs.value(u"y").toDouble();
            const double w = attrs.value(u"width").toDouble();
            const double h = attrs.value(u"height").toDouble();
            double rx = attrs.value(u"rx").toDouble();
            double ry = attrs.value(u"ry").toDouble();
            if(rx == 0) rx = ry;
            if(ry == 0) ry = rx;
            QPainterPath p;
            if(rx > 0 || ry > 0)
                p.addRoundedRect(x, y, w, h, rx, ry);
            else
                p.addRect(x, y, w, h);
            addPath(p);
        } else if(tag == u"circle") {
            QPainterPath p;
            p.addEllipse({attrs.value(u"cx").toDouble(), attrs.value(u"cy").toDouble()},
                attrs.value(u"r").toDouble(), attrs.value(u"r").toDouble());
            addPath(p);
        } else if(tag == u"ellipse") {
            QPainterPath p;
            p.addEllipse({attrs.value(u"cx").toDouble(), attrs.value(u"cy").toDouble()},
                attrs.value(u"rx").toDouble(), attrs.value(u"ry").toDouble());
            addPath(p);
        } else if(tag == u"polyline") {
            QPolygonF pts = parsePoints(attrs.value(u"points"));
            if(pts.size() >= 2) {
                QPainterPath p;
                p.addPolygon(pts);
                addPath(p);
            }
        } else if(tag == u"polygon") {
            QPolygonF pts = parsePoints(attrs.value(u"points"));
            if(pts.size() >= 2) {
                QPainterPath p;
                p.addPolygon(pts);
                p.closeSubpath();
                addPath(p);
            }
        } else if(tag == u"path") {
            addPath(parseSvgPathData(attrs.value(u"d")));
        }
    }

    // Process children for containers; skip subtree otherwise
    const bool recurse = isContainer || isDefs;
    if(recurse) {
        while(!xml.atEnd()) {
            xml.readNext();
            if(xml.isStartElement())
                parseElement(xml, isDefs ? QTransform{} : localTransform);
            else if(xml.isEndElement() && xml.name() == tag)
                break;
        }
    } else {
        // Consume to end of this element
        int depth = 1;
        while(!xml.atEnd() && depth > 0) {
            xml.readNext();
            if(xml.isStartElement()) ++depth;
            else if(xml.isEndElement()) --depth;
        }
    }
}

} // namespace Svg
