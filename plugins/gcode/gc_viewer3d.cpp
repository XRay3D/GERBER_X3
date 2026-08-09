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
#include "gc_viewer3d.h"

#include "app.h"

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QPropertyAnimation>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <ctre.hpp>

#include <cmath>
#include <numbers>
#include <optional>

// Константы OpenGL 4.0: в заголовках GL, с которыми собран Qt, их может не быть.
#ifndef GL_PATCHES
    #define GL_PATCHES 0x000E
#endif
#ifndef GL_PATCH_VERTICES
    #define GL_PATCH_VERTICES 0x8E72
#endif
#ifndef GL_MAX_TESS_GEN_LEVEL
    #define GL_MAX_TESS_GEN_LEVEL 0x8E7E
#endif
#ifndef GL_ALIASED_LINE_WIDTH_RANGE
    #define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#endif

namespace GCode {

namespace {

constexpr double gcvPi = std::numbers::pi;

const QString gcvSettingsGroup = u"GCodeViewer3d"_s;
const QString gcvPerspectiveKey = u"perspective"_s;
const QString gcvRapidsKey = u"rapidsVisible"_s;

// Плоскость дуги: индексы осей плоскости (a0, a1) и нормали (an).
struct GcvArcPlane {
    int a0, a1, an;
};

constexpr GcvArcPlane gcvArcPlanes[]{
    {0, 1, 2}, // G17 XY
    {2, 0, 1}, // G18 ZX
    {1, 2, 0}, // G19 YZ
};

// Индексы слов с координатами; смещения дуги I/J/K лежат сразу за X/Y/Z,
// поэтому смещение по оси ax — это gcvWords[GcvI + ax].
enum GcvWord {
    GcvX,
    GcvY,
    GcvZ,
    GcvI,
    GcvJ,
    GcvK,
    GcvR,
    GcvWordCount
};

// Число хорд, при котором прогиб дуги не превышает допуска, мм.
int gcvArcSteps(const PathMove& move, double tolerance) {
    const double maxAngle = move.radius > 1e-9
        ? 2. * std::acos(std::clamp(1. - tolerance / move.radius, -1., 1.))
        : 2. * gcvPi;
    return std::clamp(int(std::ceil(std::abs(move.sweep) / std::max(maxAngle, 1e-4))), 1, 4096);
}

// Точка дуги по параметру t из [0, 1].
QVector3D gcvArcPoint(const PathMove& move, double t) {
    const auto& pl = gcvArcPlanes[move.plane];
    const double angle = move.startAngle + move.sweep * t;
    QVector3D pt;
    pt[pl.a0] = float(move.center[pl.a0] + move.radius * std::cos(angle));
    pt[pl.a1] = float(move.center[pl.a1] + move.radius * std::sin(angle));
    pt[pl.an] = float(move.from[pl.an] + (move.to[pl.an] - move.from[pl.an]) * t);
    return pt;
}

// Дуга -> одно или несколько перемещений не длиннее 90°. Ограничение по длине
// нужно аппаратной тесселяции: уровень разбиения патча ограничен сверху, и на
// четверти окружности его с запасом хватает на любое увеличение.
void gcvAppendArc(std::vector<PathMove>& out, const QVector3D& from, const QVector3D& to,
    const QVector3D& center, int planeIdx, bool ccw, int lineNo) {
    const auto& pl = gcvArcPlanes[planeIdx];
    const double c0 = center[pl.a0];
    const double c1 = center[pl.a1];
    const double radius = std::hypot(from[pl.a0] - c0, from[pl.a1] - c1);
    const double startAngle = std::atan2(from[pl.a1] - c1, from[pl.a0] - c0);
    const double endAngle = std::atan2(to[pl.a1] - c1, to[pl.a0] - c0);

    double sweep = endAngle - startAngle;
    // Полный круг (конец совпадает с началом) даёт sweep == 0 и разворачивается
    // здесь в целые 2*pi нужного знака.
    if(ccw)
        while(sweep <= 0.0) sweep += 2.0 * gcvPi;
    else
        while(sweep >= 0.0) sweep -= 2.0 * gcvPi;

    const int parts = std::max(1, int(std::ceil(std::abs(sweep) / (gcvPi / 2))));
    PathMove move{
        .center{center},
        .radius{float(radius)},
        .lineNo{lineNo},
        .plane{planeIdx},
    };
    for(int i{}; i < parts; ++i) {
        const double a0 = startAngle + sweep * i / parts;
        const double a1 = startAngle + sweep * (i + 1) / parts;
        const double n0 = from[pl.an] + (to[pl.an] - from[pl.an]) * double(i) / parts;
        const double n1 = from[pl.an] + (to[pl.an] - from[pl.an]) * double(i + 1) / parts;

        QVector3D p0, p1;
        p0[pl.a0] = float(c0 + radius * std::cos(a0));
        p0[pl.a1] = float(c1 + radius * std::sin(a0));
        p0[pl.an] = float(n0);
        p1[pl.a0] = float(c0 + radius * std::cos(a1));
        p1[pl.a1] = float(c1 + radius * std::sin(a1));
        p1[pl.an] = float(n1);

        move.startAngle = float(a0);
        move.sweep = float(sweep / parts);
        // Начало первой части и конец последней берём из УП как есть — без
        // погрешности пересчёта через угол.
        move.from = i ? p0 : from;
        move.to = i + 1 == parts ? to : p1;
        out.push_back(move);
    }
}

// Разбор текста УП. Поддерживаются G0..G3, G17..G19, G20/G21, G90/G91 и
// G90.1/G91.1; строки без слов перемещения отрезков не дают.
std::vector<PathMove> gcvParseProgram(const QString& text) {
    static constexpr ctll::fixed_string wordPattern{R"(([A-Za-z])[ \t]*([\+\-]?(?:\d+\.?\d*|\.\d+)))"};

    std::vector<PathMove> result;
    QVector3D pos{};
    int motion{-1}; // модальный код перемещения: 0..3, -1 — ещё не задан
    int plane{};
    bool absolute{true};
    bool absoluteArcCenter{};
    double unit{1.}; // G20 — дюймы

    const auto lines = text.split(u'\n');
    for(int lineNo{}; lineNo < lines.size(); ++lineNo) {
        QString line = lines[lineNo];
        if(auto idx = line.indexOf(u';'); idx >= 0) line.truncate(idx);
        while(true) { // скобочные комментарии
            auto beg = line.indexOf(u'(');
            if(beg < 0) break;
            auto end = line.indexOf(u')', beg);
            line.remove(beg, (end < 0 ? line.size() : end + 1) - beg);
        }
        if(line.isEmpty()) continue;

        std::optional<double> word[GcvWordCount];
        bool hasMotionWord{};

        std::u16string_view data{line};
        for(auto [whole, letter, number]: ctre::search_all<wordPattern>(data)) {
            char16_t code = *letter.data();
            if(code >= u'a') code -= u'a' - u'A';
            const double value = QStringView{number.data(), qsizetype(number.size())}.toDouble();
            auto set = [&](GcvWord w) { word[w] = value, hasMotionWord = true; };
            switch(code) {
            case u'G':
                switch(qRound(value * 10)) { // десятые нужны для G90.1/G91.1
                case 0:
                case 10:
                case 20:
                case 30 : motion = qRound(value); break;
                case 170: plane = 0; break;
                case 180: plane = 1; break;
                case 190: plane = 2; break;
                case 200: unit = 25.4; break;
                case 210: unit = 1.; break;
                case 900: absolute = true; break;
                case 901: absoluteArcCenter = true; break;
                case 910: absolute = false; break;
                case 911: absoluteArcCenter = false; break;
                }
                break;
            case u'X': set(GcvX); break;
            case u'Y': set(GcvY); break;
            case u'Z': set(GcvZ); break;
            case u'I': set(GcvI); break;
            case u'J': set(GcvJ); break;
            case u'K': set(GcvK); break;
            case u'R': set(GcvR); break;
            default  : break; // F, S, M, N, T — на геометрию не влияют
            }
        }

        if(motion < 0 || !hasMotionWord) continue;

        QVector3D target = pos;
        for(int axis{}; axis < 3; ++axis)
            if(word[axis])
                target[axis] = float(absolute ? *word[axis] * unit : pos[axis] + *word[axis] * unit);

        if(motion < 2) { // G0/G1
            if(target != pos)
                result.push_back(PathMove{
                    .from{pos},
                    .to{target},
                    .lineNo{lineNo},
                    .rapid{motion == 0},
                });
            pos = target;
            continue;
        }

        const auto& pl = gcvArcPlanes[plane];
        QVector3D center;
        bool hasCenter{};
        if(word[GcvI + pl.a0] || word[GcvI + pl.a1]) {
            const double off0 = word[GcvI + pl.a0].value_or(0.) * unit;
            const double off1 = word[GcvI + pl.a1].value_or(0.) * unit;
            center = pos;
            center[pl.a0] = float(absoluteArcCenter ? off0 : pos[pl.a0] + off0);
            center[pl.a1] = float(absoluteArcCenter ? off1 : pos[pl.a1] + off1);
            hasCenter = true;
        } else if(word[GcvR]) {
            const double radius = *word[GcvR] * unit;
            const double d0 = target[pl.a0] - pos[pl.a0];
            const double d1 = target[pl.a1] - pos[pl.a1];
            const double chord = std::hypot(d0, d1);
            const double h2 = radius * radius - chord * chord / 4.;
            if(chord > 1e-9 && h2 >= 0.) {
                // Знак: R > 0 — меньшая дуга, R < 0 — большая.
                const double sign = ((motion == 2) == (radius > 0.)) ? -1. : 1.;
                const double h = std::sqrt(h2);
                center = pos;
                center[pl.a0] = float((pos[pl.a0] + target[pl.a0]) / 2. + sign * h * -d1 / chord);
                center[pl.a1] = float((pos[pl.a1] + target[pl.a1]) / 2. + sign * h * d0 / chord);
                hasCenter = true;
            }
        }

        if(hasCenter)
            gcvAppendArc(result, pos, target, center, plane, motion == 3, lineNo);
        else if(target != pos) // дуга без центра — рисуем хордой
            result.push_back(PathMove{.from{pos}, .to{target}, .lineNo{lineNo}});
        pos = target;
    }

    return result;
}

// Расстояние от точки до отрезка в экранных координатах.
double gcvDistanceToSegment(QPointF pt, QPointF a, QPointF b) {
    const QPointF ab = b - a;
    const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if(len2 < 1e-12) return QLineF{pt, a}.length();
    const QPointF ap = pt - a;
    const double t = std::clamp((ap.x() * ab.x() + ap.y() * ab.y()) / len2, 0., 1.);
    return QLineF{pt, a + ab * t}.length();
}

// Шейдеры: цвет несёт каждая вершина, поэтому программа одна на всю сцену.
constexpr auto gcvVertexShader110 = R"(
attribute vec3 aPos;
attribute vec4 aColor;
uniform mat4 uMvp;
varying vec4 vColor;
void main() {
    vColor = aColor;
    gl_Position = uMvp * vec4(aPos, 1.0);
})";

constexpr auto gcvFragmentShader110 = R"(
varying vec4 vColor;
void main() { gl_FragColor = vColor; })";

constexpr auto gcvVertexShader150 = R"(#version 150
in vec3 aPos;
in vec4 aColor;
uniform mat4 uMvp;
out vec4 vColor;
void main() {
    vColor = aColor;
    gl_Position = uMvp * vec4(aPos, 1.0);
})";

constexpr auto gcvFragmentShader150 = R"(#version 150
in vec4 vColor;
out vec4 fragColor;
void main() { fragColor = vColor; })";

// Дуга целиком: одна вершина-патч со своими параметрами, разбиение считает
// тесселятор. Вершинный шейдер только протаскивает атрибуты дальше.
constexpr auto gcvArcVertexShader = R"(#version 400 core
in vec3 aCenter;
in vec4 aColor;
in vec4 aArc;    // startAngle, sweep, radius, plane
in vec2 aNormal; // координата по нормальной оси: начало и конец
out vec3 vCenter;
out vec4 vColor;
out vec4 vArc;
out vec2 vNormal;
void main() {
    vCenter = aCenter;
    vColor = aColor;
    vArc = aArc;
    vNormal = aNormal;
})";

// Уровень разбиения — по экранной длине дуги: примерно один отрезок на три
// пикселя, но не больше предела тесселятора.
constexpr auto gcvArcTessControlShader = R"(#version 400 core
layout(vertices = 1) out;
in vec3 vCenter[];
in vec4 vColor[];
in vec4 vArc[];
in vec2 vNormal[];
out vec3 tcCenter[];
out vec4 tcColor[];
out vec4 tcArc[];
out vec2 tcNormal[];
uniform float uPixelsPerUnit;
uniform float uMaxLevel;
void main() {
    tcCenter[gl_InvocationID] = vCenter[gl_InvocationID];
    tcColor[gl_InvocationID] = vColor[gl_InvocationID];
    tcArc[gl_InvocationID] = vArc[gl_InvocationID];
    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
    float pixels = abs(vArc[0].y) * vArc[0].z * uPixelsPerUnit;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = clamp(ceil(pixels / 3.0) + 1.0, 2.0, uMaxLevel);
})";

// Изолиния не даёт вершину при u == 1, поэтому параметр растягиваем: иначе
// между соседними дугами оставался бы разрыв в один отрезок.
constexpr auto gcvArcTessEvalShader = R"(#version 400 core
layout(isolines, equal_spacing) in;
in vec3 tcCenter[];
in vec4 tcColor[];
in vec4 tcArc[];
in vec2 tcNormal[];
out vec4 vColor;
uniform mat4 uMvp;
void main() {
    float level = gl_TessLevelOuter[1];
    float t = min(gl_TessCoord.x * level / max(level - 1.0, 1.0), 1.0);
    vec4 arc = tcArc[0];
    vec3 u, v, w;
    if(arc.w < 0.5) {        // G17 XY
        u = vec3(1.0, 0.0, 0.0); v = vec3(0.0, 1.0, 0.0); w = vec3(0.0, 0.0, 1.0);
    } else if(arc.w < 1.5) { // G18 ZX
        u = vec3(0.0, 0.0, 1.0); v = vec3(1.0, 0.0, 0.0); w = vec3(0.0, 1.0, 0.0);
    } else {                 // G19 YZ
        u = vec3(0.0, 1.0, 0.0); v = vec3(0.0, 0.0, 1.0); w = vec3(1.0, 0.0, 0.0);
    }
    float angle = arc.x + arc.y * t;
    vec3 c = tcCenter[0];
    vec3 p = u * (dot(c, u) + arc.z * cos(angle))
           + v * (dot(c, v) + arc.z * sin(angle))
           + w * mix(tcNormal[0].x, tcNormal[0].y, t);
    vColor = tcColor[0];
    gl_Position = uMvp * vec4(p, 1.0);
})";

constexpr auto gcvArcFragmentShader = R"(#version 400 core
in vec4 vColor;
out vec4 fragColor;
void main() { fragColor = vColor; })";

enum {
    AttrPos,
    AttrColor
};

enum {
    AttrArcCenter,
    AttrArcColor,
    AttrArcParams,
    AttrArcNormal
};

// Поддержку тесселяции проверяем отдельным закадровым контекстом: формат окна
// нужно задать до его создания, а запрашивать 4.0 core вслепую нельзя — на
// старом железе окно просто не создастся.
bool gcvTessellationSupported() {
    static const bool supported = [] {
        // Аварийный выключатель на случай кривого драйвера.
        if(qEnvironmentVariableIsSet("GGEASY_NO_TESSELLATION")) return false;
        if(QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGL)
            return false; // OpenGL ES: тесселяция только с 3.2, не связываемся
        QSurfaceFormat fmt;
        fmt.setVersion(4, 0);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        QOpenGLContext ctx;
        ctx.setFormat(fmt);
        if(!ctx.create()) return false;
        const auto& got = ctx.format();
        return got.profile() == QSurfaceFormat::CoreProfile
            && (got.majorVersion() > 4 || (got.majorVersion() == 4 && got.minorVersion() >= 0));
    }();
    return supported;
}

} // namespace

Viewer3d::Viewer3d(QWidget* parent)
    : QOpenGLWidget{parent} {
    tessellation_ = gcvTessellationSupported();

    QSurfaceFormat fmt = format();
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    if(tessellation_) {
        fmt.setVersion(4, 0);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
    }
    setFormat(fmt);
    setMinimumSize(200, 200);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(false);

    MySettings settings;
    settings.beginGroup(gcvSettingsGroup);
    settings.getValue(gcvPerspectiveKey, perspective_, true);
    settings.getValue(gcvRapidsKey, rapidsVisible_, true);
    settings.endGroup();
}

Viewer3d::~Viewer3d() {
    if(!context()) return;
    makeCurrent();
    pathBuffer_.destroy();
    auxBuffer_.destroy();
    gizmoBuffer_.destroy();
    hlBuffer_.destroy();
    arcBuffer_.destroy();
    hlArcBuffer_.destroy();
    vao_.destroy();
    doneCurrent();
}

void Viewer3d::setProgramText(const QString& text) {
    moves_ = gcvParseProgram(text);
    // Стартовое разбиение — под типовой масштаб; в программном режиме его
    // уточнит updateTessellation, когда станут известны размеры окна.
    tessellate(0.02);

    bbMin_ = bbMax_ = {};
    if(!segments_.empty()) {
        bbMin_ = bbMax_ = segments_.front().from;
        auto grow = [this](const QVector3D& p) {
            bbMin_ = {std::min(bbMin_.x(), p.x()), std::min(bbMin_.y(), p.y()), std::min(bbMin_.z(), p.z())};
            bbMax_ = {std::max(bbMax_.x(), p.x()), std::max(bbMax_.y(), p.y()), std::max(bbMax_.z(), p.z())};
        };
        for(auto&& seg: segments_) grow(seg.from), grow(seg.to);
    }

    hlFirstLine_ = hlLastLine_ = -1;
    fitToView(); // задаёт center_/sceneRadius_, от них зависит вспомогательная геометрия
    buildPathVertices();
    buildAuxVertices();
    buildHighlightVertices();
    update();
}

void Viewer3d::tessellate(double tolerance) {
    tessTolerance_ = tolerance;
    segments_.clear();
    segments_.reserve(moves_.size());
    for(auto&& move: moves_) {
        if(!move.isArc()) {
            segments_.emplace_back(move.from, move.to, move.lineNo, move.rapid);
            continue;
        }
        const int steps = gcvArcSteps(move, tolerance);
        QVector3D prev = move.from;
        for(int i{1}; i <= steps; ++i) {
            const QVector3D pt = i == steps ? move.to : gcvArcPoint(move, double(i) / steps);
            segments_.emplace_back(prev, pt, move.lineNo, move.rapid);
            prev = pt;
        }
    }
    pathDirty_ = hlDirty_ = true;
}

void Viewer3d::updateTessellation() {
    if(tessellation_ || moves_.empty()) return;
    // Прогиб хорды держим в четверти пикселя, но не мельче микрона — иначе на
    // сильном приближении число вершин уходит в никуда.
    const double tolerance = std::clamp(worldPerPixel() * 0.25, 1e-3, 0.5);
    if(tessTolerance_ > 0.
        && tolerance < tessTolerance_ * 1.4 && tessTolerance_ < tolerance * 1.4)
        return; // масштаб изменился незначительно — пересчёт не нужен
    tessellate(tolerance);
    buildPathVertices();
    buildHighlightVertices();
}

float Viewer3d::worldPerPixel() const {
    return 2.f * distance_ * std::tan(qDegreesToRadians(fov_ * 0.5f)) / std::max(1, height());
}

void Viewer3d::setCameraCenter(const QVector3D& center) {
    if(center_ == center) return;
    center_ = center;
    // viewTouched_ намеренно не трогаем: подвод камеры к курсору -- это не
    // "покрутили руками", и вписывание при изменении размера должно остаться.
    update();
}

void Viewer3d::centerOnHighlight() {
    if(!hlAnchorValid_ || center_ == hlAnchor_) return;

    if(centerAnimation_) centerAnimation_->stop();

    if(!App::settings().guiSmoothScSh()) {
        setCameraCenter(hlAnchor_);
        return;
    }

    if(!centerAnimation_) {
        centerAnimation_ = new QPropertyAnimation{this, "cameraCenter", this};
        centerAnimation_->setEasingCurve(QEasingCurve::InOutSine);
        centerAnimation_->setDuration(200); // как у плавного fitInView в GraphicsView
    }
    centerAnimation_->setStartValue(center_);
    centerAnimation_->setEndValue(hlAnchor_);
    centerAnimation_->start();
}

void Viewer3d::setHighlightedLines(int first, int last) {
    if(first > last) std::swap(first, last);
    if(hlFirstLine_ == first && hlLastLine_ == last) return;
    hlFirstLine_ = first;
    hlLastLine_ = last;
    buildHighlightVertices();
    update();
}

void Viewer3d::updateColors() {
    buildPathVertices();
    buildAuxVertices();
    buildHighlightVertices();
    update();
}

void Viewer3d::setRapidsVisible(bool visible) {
    if(rapidsVisible_ == visible) return;
    rapidsVisible_ = visible;

    MySettings settings;
    settings.beginGroup(gcvSettingsGroup);
    settings.setValue(gcvRapidsKey, rapidsVisible_);
    settings.endGroup();

    buildPathVertices();
    update();
}

void Viewer3d::setPerspective(bool enabled) {
    if(perspective_ == enabled) return;
    perspective_ = enabled;

    MySettings settings;
    settings.beginGroup(gcvSettingsGroup);
    settings.setValue(gcvPerspectiveKey, perspective_);
    settings.endGroup();

    update();
}

void Viewer3d::fitToView() {
    center_ = (bbMin_ + bbMax_) * 0.5f;
    sceneRadius_ = std::max((bbMax_ - bbMin_).length() * 0.5f, 1.f);

    // Удаление подбираем по проекции габаритного параллелепипеда на оси камеры,
    // а не по описанной сфере, — иначе вытянутая траектория занимает лишь часть
    // кадра.
    const QVector3D dir = cameraDir();
    const QVector3D right = QVector3D::crossProduct({0.f, 0.f, 1.f}, dir).normalized();
    const QVector3D up = QVector3D::crossProduct(dir, right).normalized();
    const float tanY = std::tan(qDegreesToRadians(fov_ * 0.5f));
    const float tanX = tanY * std::max(0.1f, float(width()) / std::max(1, height()));

    float distance = sceneRadius_ * 0.1f;
    for(int i{}; i < 8; ++i) {
        const QVector3D corner{
            i & 1 ? bbMax_.x() : bbMin_.x(),
            i & 2 ? bbMax_.y() : bbMin_.y(),
            i & 4 ? bbMax_.z() : bbMin_.z()};
        const QVector3D v = corner - center_;
        const float toCamera = QVector3D::dotProduct(v, dir);
        distance = std::max({distance,
            std::abs(QVector3D::dotProduct(v, right)) / tanX + toCamera,
            std::abs(QVector3D::dotProduct(v, up)) / tanY + toCamera});
    }
    distance_ = distance * 1.05f;
    update();
}

void Viewer3d::setViewPreset(ViewPreset preset) {
    switch(preset) {
    case Isometric: yaw_ = -45.f, pitch_ = 30.f; break;
    case Top      : yaw_ = -90.f, pitch_ = 89.9f; break;
    case Front    : yaw_ = -90.f, pitch_ = 0.f; break;
    case Left     : yaw_ = 180.f, pitch_ = 0.f; break;
    }
    fitToView();
}

void Viewer3d::buildPathVertices() {
    const QColor cutColor = App::settings().guiColor(GuiColors::ToolPath);
    const QColor rapidColor = App::settings().guiColor(GuiColors::G0);
    auto colorOf = [&](bool rapid) -> const QColor& { return rapid ? rapidColor : cutColor; };

    auto addLine = [](std::vector<Vertex>& out, QVector3D a, QVector3D b, const QColor& c) {
        const float r = float(c.redF()), g = float(c.greenF()), bl = float(c.blueF()), al = float(c.alphaF());
        out.emplace_back(Vertex{a.x(), a.y(), a.z(), r, g, bl, al});
        out.emplace_back(Vertex{b.x(), b.y(), b.z(), r, g, bl, al});
    };

    pathVertices_.clear();
    arcVertices_.clear();

    if(tessellation_) {
        // Прямые идут линиями, дуги — патчами: разобьёт тесселятор.
        for(auto&& move: moves_) {
            if(move.rapid && !rapidsVisible_) continue;
            if(move.isArc()) addArcPatch(arcVertices_, move, colorOf(move.rapid));
            else addLine(pathVertices_, move.from, move.to, colorOf(move.rapid));
        }
    } else {
        pathVertices_.reserve(segments_.size() * 2);
        for(auto&& seg: segments_) {
            if(seg.rapid && !rapidsVisible_) continue;
            addLine(pathVertices_, seg.from, seg.to, colorOf(seg.rapid));
        }
    }

    pathDirty_ = arcDirty_ = true;
}

void Viewer3d::addArcPatch(std::vector<ArcVertex>& out, const PathMove& move, const QColor& c) {
    const auto& pl = gcvArcPlanes[move.plane];
    out.emplace_back(ArcVertex{
        move.center.x(), move.center.y(), move.center.z(),
        float(c.redF()), float(c.greenF()), float(c.blueF()), float(c.alphaF()),
        move.startAngle, move.sweep, move.radius, float(move.plane),
        move.from[pl.an], move.to[pl.an]});
}

void Viewer3d::buildAuxVertices() {
    auxVertices_.clear();

    auto addLine = [this](QVector3D a, QVector3D b, const QColor& c) {
        const float r = float(c.redF()), g = float(c.greenF()), bl = float(c.blueF()), al = float(c.alphaF());
        auxVertices_.emplace_back(Vertex{a.x(), a.y(), a.z(), r, g, bl, al});
        auxVertices_.emplace_back(Vertex{b.x(), b.y(), b.z(), r, g, bl, al});
    };

    // Сетка в плоскости XY. Шаг подбирается так, чтобы линий было немного:
    // каждая десятая — Grid10, каждая пятая — Grid05, остальные — Grid01.
    const QColor gridColor[]{
        App::settings().guiColor(GuiColors::Grid01),
        App::settings().guiColor(GuiColors::Grid05),
        App::settings().guiColor(GuiColors::Grid10),
    };

    // Область сетки охватывает и траекторию, и ноль детали.
    const double xMin = std::min(0.f, bbMin_.x()), xMax = std::max(0.f, bbMax_.x());
    const double yMin = std::min(0.f, bbMin_.y()), yMax = std::max(0.f, bbMax_.y());
    double step = 1.;
    while(std::max({xMax - xMin, yMax - yMin, 10.}) / step > 150.) step *= 10.;

    const int kx0 = int(std::floor(xMin / step)) - 1, kx1 = int(std::ceil(xMax / step)) + 1;
    const int ky0 = int(std::floor(yMin / step)) - 1, ky1 = int(std::ceil(yMax / step)) + 1;
    auto colorOf = [&gridColor](int k) { return gridColor[k % 10 == 0 ? 2 : k % 5 == 0 ? 1
                                                                                       : 0]; };

    for(int k{kx0}; k <= kx1; ++k) {
        const float x = float(k * step);
        addLine({x, float(ky0 * step), 0.f}, {x, float(ky1 * step), 0.f}, colorOf(std::abs(k)));
    }
    for(int k{ky0}; k <= ky1; ++k) {
        const float y = float(k * step);
        addLine({float(kx0 * step), y, 0.f}, {float(kx1 * step), y, 0.f}, colorOf(std::abs(k)));
    }

    if(!segments_.empty()) { // габаритный параллелепипед траектории
        const QColor c = App::settings().guiColor(GuiColors::CutArea);
        const float x[]{bbMin_.x(), bbMax_.x()}, y[]{bbMin_.y(), bbMax_.y()}, z[]{bbMin_.z(), bbMax_.z()};
        for(int i{}; i < 2; ++i)
            for(int j{}; j < 2; ++j) {
                addLine({x[0], y[i], z[j]}, {x[1], y[i], z[j]}, c);
                addLine({x[i], y[0], z[j]}, {x[i], y[1], z[j]}, c);
                addLine({x[i], y[j], z[0]}, {x[i], y[j], z[1]}, c);
            }
    }

    auxDirty_ = true;
}

// Начало координат: стрелки по осям с буквами X/Y/Z. Буквы развёрнуты к
// камере, поэтому геометрия пересобирается на каждый кадр.
void Viewer3d::buildGizmoVertices() {
    // Штрихи букв в квадрате [-0.5, 0.5]: {x0, y0, x1, y1}.
    static const std::vector<std::array<float, 4>> glyph[]{
        {{-.4f, -.5f, .4f, .5f}, {-.4f, .5f, .4f, -.5f}}, // X
        {{-.4f, .5f, 0.f, 0.f}, {.4f, .5f, 0.f, 0.f}, {0.f, 0.f, 0.f, -.5f}}, // Y
        {{-.4f, .5f, .4f, .5f}, {.4f, .5f, -.4f, -.5f}, {-.4f, -.5f, .4f, -.5f}}  // Z
    };

    gizmoVertices_.clear();
    gizmoDirty_ = true;

    const QVector3D dir = cameraDir();
    const QVector3D right = QVector3D::crossProduct({0.f, 0.f, 1.f}, dir).normalized();
    const QVector3D up = QVector3D::crossProduct(dir, right).normalized();

    // Длину стрелок держим постоянной на экране (доля от высоты кадра), иначе
    // на крупной программе они превращаются в точку, а при зуме — в частокол.
    const float len = distance_ * std::tan(qDegreesToRadians(fov_ * 0.5f)) * 0.18f;
    const float head = len * 0.15f; // длина наконечника
    const float rad = len * 0.05f;  // радиус наконечника
    const float size = len * 0.22f; // размер буквы

    auto addLine = [this](QVector3D a, QVector3D b, const QColor& c) {
        const float r = float(c.redF()), g = float(c.greenF()), bl = float(c.blueF()), al = float(c.alphaF());
        gizmoVertices_.emplace_back(Vertex{a.x(), a.y(), a.z(), r, g, bl, al});
        gizmoVertices_.emplace_back(Vertex{b.x(), b.y(), b.z(), r, g, bl, al});
    };

    for(int i{}; i < 3; ++i) {
        QVector3D axis, p, q;
        axis[i] = p[(i + 1) % 3] = q[(i + 2) % 3] = 1.f;
        const QColor& color = axisColor[i];

        const QVector3D tip = axis * len;
        const QVector3D base = axis * (len - head);
        addLine({}, tip, color);

        const QVector3D ring[]{p * rad, q * rad, -p * rad, -q * rad};
        for(int k{}; k < 4; ++k) {
            addLine(tip, base + ring[k], color);
            addLine(base + ring[k], base + ring[(k + 1) % 4], color);
        }

        const QVector3D letter = axis * (len + size * 1.5f);
        for(auto&& [x0, y0, x1, y1]: glyph[i])
            addLine(letter + right * (x0 * size) + up * (y0 * size),
                letter + right * (x1 * size) + up * (y1 * size), color);
    }
}

void Viewer3d::buildHighlightVertices() {
    hlVertices_.clear();
    hlArcVertices_.clear();
    hlDirty_ = hlArcDirty_ = true;
    hlAnchorValid_ = false;
    if(hlFirstLine_ < 0 || moves_.empty()) return;

    // lineNo по moves_ не убывает, так что нужные перемещения лежат подряд.
    auto beg = std::lower_bound(moves_.begin(), moves_.end(), hlFirstLine_,
        [](const PathMove& m, int line) { return m.lineNo < line; });
    auto end = std::upper_bound(beg, moves_.end(), hlLastLine_,
        [](int line, const PathMove& m) { return line < m.lineNo; });
    if(beg == end) return;

    // Настроечная прозрачность подсветки может быть совсем низкой — поднимаем,
    // иначе выделение не читается поверх траектории.
    QColor c = App::settings().guiColor(GuiColors::Pin);
    c.setAlpha(std::max(c.alpha(), 200));
    auto addLine = [this](QVector3D a, QVector3D b, const QColor& c) {
        const float r = float(c.redF()), g = float(c.greenF()), bl = float(c.blueF()), al = float(c.alphaF());
        hlVertices_.emplace_back(Vertex{a.x(), a.y(), a.z(), r, g, bl, al});
        hlVertices_.emplace_back(Vertex{b.x(), b.y(), b.z(), r, g, bl, al});
    };

    for(auto it = beg; it != end; ++it) {
        if(!it->isArc())
            addLine(it->from, it->to, c);
        else if(tessellation_)
            addArcPatch(hlArcVertices_, *it, c);
        else { // повторяем то же разбиение, что и у самой траектории
            const int steps = gcvArcSteps(*it, tessTolerance_);
            QVector3D prev = it->from;
            for(int i{1}; i <= steps; ++i) {
                const QVector3D pt = i == steps ? it->to : gcvArcPoint(*it, double(i) / steps);
                addLine(prev, pt, c);
                prev = pt;
            }
        }
    }

    { // маркер инструмента в начале подсвеченного участка
        const QColor home = App::settings().guiColor(GuiColors::Home);
        const QVector3D p = beg->from;
        hlAnchor_ = p;
        hlAnchorValid_ = true;
        const float len = std::max(sceneRadius_ * 0.03f, 0.5f);
        addLine(p - QVector3D{len, 0.f, 0.f}, p + QVector3D{len, 0.f, 0.f}, home);
        addLine(p - QVector3D{0.f, len, 0.f}, p + QVector3D{0.f, len, 0.f}, home);
        addLine(p, p + QVector3D{0.f, 0.f, len * 4.f}, home);
    }
}

void Viewer3d::initializeGL() {
    initializeOpenGLFunctions();

    const bool wanted = tessellation_;
    const auto& fmt = context()->format();
    const bool core = fmt.profile() == QSurfaceFormat::CoreProfile;
    // Драйвер мог выдать контекст беднее запрошенного — проверяем ещё раз.
    if(tessellation_ && !(core && fmt.majorVersion() >= 4)) tessellation_ = false;

    program_.addShaderFromSourceCode(QOpenGLShader::Vertex, core ? gcvVertexShader150 : gcvVertexShader110);
    program_.addShaderFromSourceCode(QOpenGLShader::Fragment, core ? gcvFragmentShader150 : gcvFragmentShader110);
    program_.bindAttributeLocation("aPos", AttrPos);
    program_.bindAttributeLocation("aColor", AttrColor);
    if(!program_.link())
        qWarning() << "GCode::Viewer3d: shader link failed:" << program_.log();

    if(tessellation_) {
        arcProgram_.addShaderFromSourceCode(QOpenGLShader::Vertex, gcvArcVertexShader);
        arcProgram_.addShaderFromSourceCode(QOpenGLShader::TessellationControl, gcvArcTessControlShader);
        arcProgram_.addShaderFromSourceCode(QOpenGLShader::TessellationEvaluation, gcvArcTessEvalShader);
        arcProgram_.addShaderFromSourceCode(QOpenGLShader::Fragment, gcvArcFragmentShader);
        arcProgram_.bindAttributeLocation("aCenter", AttrArcCenter);
        arcProgram_.bindAttributeLocation("aColor", AttrArcColor);
        arcProgram_.bindAttributeLocation("aArc", AttrArcParams);
        arcProgram_.bindAttributeLocation("aNormal", AttrArcNormal);
        if(!arcProgram_.link()) {
            qWarning() << "GCode::Viewer3d: tessellation shader link failed:" << arcProgram_.log();
            tessellation_ = false; // откатываемся на разбиение дуг на CPU
        } else {
            GLint maxLevel{64};
            glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxLevel);
            arcProgram_.bind();
            arcProgram_.setUniformValue("uMaxLevel", float(maxLevel));
            arcProgram_.release();
            glPatchParameteri(GL_PATCH_VERTICES, 1);
        }
    }

    // Буферы могли быть построены до создания контекста, когда способ отрисовки
    // дуг ещё только предполагался.
    if(wanted != tessellation_) {
        buildPathVertices();
        buildHighlightVertices();
    }

    // В core-профиле любая ширина линии, кроме единицы, — GL_INVALID_VALUE,
    // поэтому подсветку там утолщать нельзя.
    lineWidthMax_ = 1.f;
    if(!core) {
        GLfloat range[2]{1.f, 1.f};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
        lineWidthMax_ = std::max(1.f, range[1]);
    }

    vao_.create();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Viewer3d::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    if(!viewTouched_) fitToView(); // первичная вписка — уже с реальными пропорциями
}

void Viewer3d::paintGL() {
    const QColor bg = App::settings().guiColor(GuiColors::Background);
    glClearColor(float(bg.redF()), float(bg.greenF()), float(bg.blueF()), 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!program_.isLinked()) return;

    updateTessellation(); // масштаб мог измениться — дуги пересчитываются на CPU

    const QMatrix4x4 mvp = mvpMatrix();
    QOpenGLVertexArrayObject::Binder vaoBinder{&vao_};

    // Пикселей на миллиметр в плоскости точки интереса — по этой величине
    // тесселятор выбирает частоту разбиения дуг.
    const float pixelsPerUnit = 1.f / std::max(worldPerPixel(), 1e-6f);

    auto drawPath = [&] {
        program_.bind();
        program_.setUniformValue("uMvp", mvp);
        drawVertices(pathBuffer_, pathVertices_, pathDirty_);
        program_.release();
        if(!arcVertices_.empty()) {
            arcProgram_.bind();
            arcProgram_.setUniformValue("uMvp", mvp);
            arcProgram_.setUniformValue("uPixelsPerUnit", pixelsPerUnit);
            drawArcs(arcBuffer_, arcVertices_, arcDirty_);
            arcProgram_.release();
        }
    };

    glEnable(GL_DEPTH_TEST);
    // Сетку и габариты рисуем без записи глубины, иначе они закрывают
    // траекторию, лежащую под плоскостью Z0, при взгляде сверху.
    glDepthMask(GL_FALSE);
    program_.bind();
    program_.setUniformValue("uMvp", mvp);
    drawVertices(auxBuffer_, auxVertices_, auxDirty_);
    program_.release();
    glDepthMask(GL_TRUE);
    drawPath();

    // Стрелки осей и подсветку рисуем последними и без теста глубины, чтобы их
    // не закрывала траектория.
    glDisable(GL_DEPTH_TEST);
    program_.bind();
    program_.setUniformValue("uMvp", mvp);
    buildGizmoVertices(); // зависит от положения камеры — строим каждый кадр
    drawVertices(gizmoBuffer_, gizmoVertices_, gizmoDirty_);
    glLineWidth(std::min(2.f, lineWidthMax_));
    drawVertices(hlBuffer_, hlVertices_, hlDirty_);
    program_.release();
    if(!hlArcVertices_.empty()) {
        arcProgram_.bind();
        arcProgram_.setUniformValue("uMvp", mvp);
        arcProgram_.setUniformValue("uPixelsPerUnit", pixelsPerUnit);
        drawArcs(hlArcBuffer_, hlArcVertices_, hlArcDirty_);
        arcProgram_.release();
    }
    glLineWidth(1.f);
}

void Viewer3d::drawVertices(QOpenGLBuffer& buffer, const std::vector<Vertex>& data, bool& dirty) {
    if(data.empty()) return;
    if(!buffer.isCreated()) buffer.create(), dirty = true;
    buffer.bind();
    if(dirty) {
        buffer.allocate(data.data(), int(data.size() * sizeof(Vertex)));
        dirty = false;
    }
    program_.enableAttributeArray(AttrPos);
    program_.enableAttributeArray(AttrColor);
    program_.setAttributeBuffer(AttrPos, GL_FLOAT, 0, 3, sizeof(Vertex));
    program_.setAttributeBuffer(AttrColor, GL_FLOAT, 3 * sizeof(float), 4, sizeof(Vertex));
    glDrawArrays(GL_LINES, 0, int(data.size()));
    program_.disableAttributeArray(AttrPos);
    program_.disableAttributeArray(AttrColor);
    buffer.release();
}

void Viewer3d::drawArcs(QOpenGLBuffer& buffer, const std::vector<ArcVertex>& data, bool& dirty) {
    if(data.empty() || !arcProgram_.isLinked()) return;
    if(!buffer.isCreated()) buffer.create(), dirty = true;
    buffer.bind();
    if(dirty) {
        buffer.allocate(data.data(), int(data.size() * sizeof(ArcVertex)));
        dirty = false;
    }
    constexpr int stride = sizeof(ArcVertex);
    arcProgram_.enableAttributeArray(AttrArcCenter);
    arcProgram_.enableAttributeArray(AttrArcColor);
    arcProgram_.enableAttributeArray(AttrArcParams);
    arcProgram_.enableAttributeArray(AttrArcNormal);
    arcProgram_.setAttributeBuffer(AttrArcCenter, GL_FLOAT, 0, 3, stride);
    arcProgram_.setAttributeBuffer(AttrArcColor, GL_FLOAT, 3 * sizeof(float), 4, stride);
    arcProgram_.setAttributeBuffer(AttrArcParams, GL_FLOAT, 7 * sizeof(float), 4, stride);
    arcProgram_.setAttributeBuffer(AttrArcNormal, GL_FLOAT, 11 * sizeof(float), 2, stride);
    glDrawArrays(GL_PATCHES, 0, int(data.size()));
    arcProgram_.disableAttributeArray(AttrArcCenter);
    arcProgram_.disableAttributeArray(AttrArcColor);
    arcProgram_.disableAttributeArray(AttrArcParams);
    arcProgram_.disableAttributeArray(AttrArcNormal);
    buffer.release();
}

QVector3D Viewer3d::cameraDir() const {
    const float y = qDegreesToRadians(yaw_);
    const float p = qDegreesToRadians(std::clamp(pitch_, -89.9f, 89.9f));
    return {std::cos(p) * std::cos(y), std::cos(p) * std::sin(y), std::sin(p)};
}

QMatrix4x4 Viewer3d::mvpMatrix() const {
    QMatrix4x4 proj;
    const float aspect = float(width()) / std::max(1, height());
    const float span = sceneRadius_ * 20.f + 1000.f;
    const float farPlane = distance_ + span;
    if(perspective_) {
        proj.perspective(fov_, aspect, std::max(0.01f, distance_ * 0.005f), farPlane);
    } else {
        // Полувысота кадра та же, что и у перспективы на расстоянии distance_,
        // поэтому масштаб при переключении проекции не скачет, а зум, вписывание
        // и панорамирование считаются одинаково для обоих режимов.
        const float halfH = distance_ * std::tan(qDegreesToRadians(fov_ * 0.5f));
        proj.ortho(-halfH * aspect, halfH * aspect, -halfH, halfH, distance_ - span, farPlane);
    }
    QMatrix4x4 view;
    view.lookAt(center_ + cameraDir() * distance_, center_, {0.f, 0.f, 1.f});
    return proj * view;
}

int Viewer3d::pickLine(QPointF pos) const {
    if(segments_.empty()) return -1;
    const QMatrix4x4 mvp = mvpMatrix();
    const double w = width(), h = height();
    auto project = [&](const QVector3D& p, QPointF& out) {
        const QVector4D v = mvp * QVector4D{p, 1.f};
        if(v.w() <= 1e-6f) return false;
        out = {(v.x() / v.w() * 0.5 + 0.5) * w, (0.5 - v.y() / v.w() * 0.5) * h};
        return true;
    };

    double best = 8.; // порог попадания в пикселях
    int bestLine = -1;
    for(auto&& seg: segments_) {
        if(seg.rapid && !rapidsVisible_) continue;
        QPointF a, b;
        if(!project(seg.from, a) || !project(seg.to, b)) continue;
        if(const double d = gcvDistanceToSegment(pos, a, b); d < best)
            best = d, bestLine = seg.lineNo;
    }
    return bestLine;
}

void Viewer3d::mousePressEvent(QMouseEvent* event) {
    lastMousePos_ = pressPos_ = event->pos();
    dragged_ = false;
    event->accept();
}

void Viewer3d::mouseMoveEvent(QMouseEvent* event) {
    const QPoint delta = event->pos() - lastMousePos_;
    lastMousePos_ = event->pos();
    if((event->pos() - pressPos_).manhattanLength() > 3) dragged_ = true;

    if(event->buttons() & Qt::LeftButton) { // вращение
        viewTouched_ = true;
        yaw_ -= delta.x() * 0.4f;
        pitch_ = std::clamp(pitch_ + delta.y() * 0.4f, -89.9f, 89.9f);
        update();
    } else if(event->buttons() & (Qt::RightButton | Qt::MiddleButton)) { // панорамирование
        viewTouched_ = true;
        const float worldPerPixel = 2.f * distance_
            * std::tan(qDegreesToRadians(fov_ * 0.5f)) / std::max(1, height());
        const QVector3D dir = cameraDir();
        const QVector3D right = QVector3D::crossProduct({0.f, 0.f, 1.f}, dir).normalized();
        const QVector3D up = QVector3D::crossProduct(dir, right).normalized();
        center_ += (up * delta.y() - right * delta.x()) * worldPerPixel;
        update();
    }
    event->accept();
}

void Viewer3d::mouseReleaseEvent(QMouseEvent* event) {
    if(!dragged_ && event->button() == Qt::LeftButton)
        if(int line = pickLine(event->position()); line >= 0)
            emit lineSelected(line);
    event->accept();
}

void Viewer3d::mouseDoubleClickEvent(QMouseEvent* event) {
    fitToView();
    event->accept();
}

void Viewer3d::wheelEvent(QWheelEvent* event) {
    viewTouched_ = true;
    const double steps = event->angleDelta().y() / 120.;
    distance_ = float(std::clamp(distance_ * std::pow(0.85, steps),
        double(sceneRadius_) * 0.01, double(sceneRadius_) * 100.));
    update();
    event->accept();
}

} // namespace GCode

#include "moc_gc_viewer3d.cpp"
