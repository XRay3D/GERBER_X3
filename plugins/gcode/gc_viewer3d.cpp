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
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <ctre.hpp>

#include <cmath>
#include <numbers>
#include <optional>

namespace GCode {

namespace {

constexpr double gcvPi = std::numbers::pi;

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

void gcvAppendArc(std::vector<PathSegment>& out, const QVector3D& from, const QVector3D& to,
    const QVector3D& center, const GcvArcPlane& pl, bool ccw, int lineNo) {
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

    // Шаг такой, чтобы стрелка прогиба хорды не превышала 0.05 мм.
    constexpr double tolerance = 0.05;
    const double maxAngle = radius > 1e-9
        ? 2.0 * std::acos(std::clamp(1.0 - tolerance / radius, -1.0, 1.0))
        : 2.0 * gcvPi;
    const int steps = std::clamp(int(std::ceil(std::abs(sweep) / std::max(maxAngle, 1e-3))), 2, 2048);

    QVector3D prev = from;
    for(int i{1}; i <= steps; ++i) {
        QVector3D pt;
        if(i == steps) {
            pt = to;
        } else {
            const double angle = startAngle + sweep * i / steps;
            pt[pl.a0] = float(c0 + radius * std::cos(angle));
            pt[pl.a1] = float(c1 + radius * std::sin(angle));
            pt[pl.an] = float(from[pl.an] + (to[pl.an] - from[pl.an]) * double(i) / steps);
        }
        out.emplace_back(prev, pt, lineNo, false);
        prev = pt;
    }
}

// Разбор текста УП. Поддерживаются G0..G3, G17..G19, G20/G21, G90/G91 и
// G90.1/G91.1; строки без слов перемещения отрезков не дают.
std::vector<PathSegment> gcvParseProgram(const QString& text) {
    static constexpr ctll::fixed_string wordPattern{R"(([A-Za-z])[ \t]*([\+\-]?(?:\d+\.?\d*|\.\d+)))"};

    std::vector<PathSegment> result;
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
            if(target != pos) result.emplace_back(pos, target, lineNo, motion == 0);
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
            gcvAppendArc(result, pos, target, center, pl, motion == 3, lineNo);
        else if(target != pos) // дуга без центра — рисуем хордой
            result.emplace_back(pos, target, lineNo, false);
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

enum {
    AttrPos,
    AttrColor
};

} // namespace

Viewer3d::Viewer3d(QWidget* parent)
    : QOpenGLWidget{parent} {
    QSurfaceFormat fmt = format();
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    setFormat(fmt);
    setMinimumSize(200, 200);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(false);
}

Viewer3d::~Viewer3d() {
    if(!context()) return;
    makeCurrent();
    pathBuffer_.destroy();
    auxBuffer_.destroy();
    hlBuffer_.destroy();
    vao_.destroy();
    doneCurrent();
}

void Viewer3d::setProgramText(const QString& text) {
    segments_ = gcvParseProgram(text);

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
    buildPathVertices();
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

    pathVertices_.clear();
    pathVertices_.reserve(segments_.size() * 2);
    for(auto&& seg: segments_) {
        if(seg.rapid && !rapidsVisible_) continue;
        const QColor& c = seg.rapid ? rapidColor : cutColor;
        const float r = float(c.redF()), g = float(c.greenF()), b = float(c.blueF()), a = float(c.alphaF());
        for(const QVector3D& p: {seg.from, seg.to})
            pathVertices_.emplace_back(Vertex{p.x(), p.y(), p.z(), r, g, b, a});
    }
    pathDirty_ = true;
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

    { // маркер нуля детали
        const QColor c = App::settings().guiColor(GuiColors::Zero);
        const float len = std::max(sceneRadius_ * 0.15f, float(step));
        addLine({}, {len, 0.f, 0.f}, c);
        addLine({}, {0.f, len, 0.f}, c);
        addLine({}, {0.f, 0.f, len}, c);
    }

    auxDirty_ = true;
}

void Viewer3d::buildHighlightVertices() {
    hlVertices_.clear();
    hlDirty_ = true;
    if(hlFirstLine_ < 0 || segments_.empty()) return;

    // lineNo по segments_ не убывает, так что нужные отрезки лежат подряд.
    auto beg = std::lower_bound(segments_.begin(), segments_.end(), hlFirstLine_,
        [](const PathSegment& s, int line) { return s.lineNo < line; });
    auto end = std::upper_bound(beg, segments_.end(), hlLastLine_,
        [](int line, const PathSegment& s) { return line < s.lineNo; });
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

    for(auto it = beg; it != end; ++it) addLine(it->from, it->to, c);

    { // маркер инструмента в начале подсвеченного участка
        const QColor home = App::settings().guiColor(GuiColors::Home);
        const QVector3D p = beg->from;
        const float len = std::max(sceneRadius_ * 0.03f, 0.5f);
        addLine(p - QVector3D{len, 0.f, 0.f}, p + QVector3D{len, 0.f, 0.f}, home);
        addLine(p - QVector3D{0.f, len, 0.f}, p + QVector3D{0.f, len, 0.f}, home);
        addLine(p, p + QVector3D{0.f, 0.f, len * 4.f}, home);
    }
}

void Viewer3d::initializeGL() {
    initializeOpenGLFunctions();

    const bool core = context() && context()->format().profile() == QSurfaceFormat::CoreProfile;
    program_.addShaderFromSourceCode(QOpenGLShader::Vertex, core ? gcvVertexShader150 : gcvVertexShader110);
    program_.addShaderFromSourceCode(QOpenGLShader::Fragment, core ? gcvFragmentShader150 : gcvFragmentShader110);
    program_.bindAttributeLocation("aPos", AttrPos);
    program_.bindAttributeLocation("aColor", AttrColor);
    if(!program_.link())
        qWarning() << "GCode::Viewer3d: shader link failed:" << program_.log();
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

    QOpenGLVertexArrayObject::Binder vaoBinder{&vao_};
    program_.bind();
    program_.setUniformValue("uMvp", mvpMatrix());

    glEnable(GL_DEPTH_TEST);
    // Сетку и габариты рисуем без записи глубины, иначе они закрывают
    // траекторию, лежащую под плоскостью Z0, при взгляде сверху.
    glDepthMask(GL_FALSE);
    drawVertices(auxBuffer_, auxVertices_, auxDirty_);
    glDepthMask(GL_TRUE);
    drawVertices(pathBuffer_, pathVertices_, pathDirty_);

    // Подсветку рисуем последней и без теста глубины, чтобы она была видна
    // сквозь остальную траекторию.
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.f);
    drawVertices(hlBuffer_, hlVertices_, hlDirty_);
    glLineWidth(1.f);

    program_.release();
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
    buffer.release();
}

QVector3D Viewer3d::cameraDir() const {
    const float y = qDegreesToRadians(yaw_);
    const float p = qDegreesToRadians(std::clamp(pitch_, -89.9f, 89.9f));
    return {std::cos(p) * std::cos(y), std::cos(p) * std::sin(y), std::sin(p)};
}

QMatrix4x4 Viewer3d::mvpMatrix() const {
    QMatrix4x4 proj;
    const float nearPlane = std::max(0.01f, distance_ * 0.005f);
    const float farPlane = distance_ + sceneRadius_ * 20.f + 1000.f;
    proj.perspective(fov_, float(width()) / std::max(1, height()), nearPlane, farPlane);
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
