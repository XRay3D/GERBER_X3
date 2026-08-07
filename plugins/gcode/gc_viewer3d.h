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
#pragma once

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector3D>

#include <vector>

namespace GCode {

// Цвета осей X/Y/Z. Те же, что у соответствующих букв в подсветке синтаксиса
// УП, — так стрелки в 3D читаются заодно с текстом.
inline const QColor axisColor[]{
    {0xFF, 0x3F, 0x3F}, // X
    {0x3F, 0xFF, 0x3F}, // Y
    {0x3F, 0x3F, 0xFF}, // Z
};

// Перемещение УП: прямая (G0/G1) или дуга (G2/G3). Дуги хранятся
// параметрически, чтобы разбивать их на хорды под текущий масштаб, а не раз и
// навсегда. Дуга длиннее 90° разрезается парсером на части — так ограничен и
// шаг аппаратной тесселяции.
struct PathMove {
    QVector3D from;
    QVector3D to;
    QVector3D center;   // центр дуги; по нормальной оси совпадает с from
    float startAngle{}; // рад, отсчёт в плоскости дуги
    float sweep{};      // рад со знаком; 0 — прямая
    float radius{};
    int lineNo{}; // номер строки УП, нумерация с нуля
    int plane{};  // 0 — G17 XY, 1 — G18 ZX, 2 — G19 YZ
    bool rapid{}; // G0 — холостой ход
    bool isArc() const { return sweep != 0.f; }
};

// Отрезок траектории инструмента: концы в миллиметрах и номер строки УП
// (нумерация с нуля, совпадает с номером блока QTextDocument), из которой
// отрезок получен. Дуги разбиваются на несколько отрезков с одним и тем же
// lineNo, поэтому lineNo по вектору не убывает — на этом построен поиск
// диапазона подсветки.
struct PathSegment {
    QVector3D from;
    QVector3D to;
    int lineNo;
    bool rapid; // G0 — холостой ход
};

// Просмотрщик управляющей программы в 3D в духе Candle.
// Все цвета берутся из настроек приложения (GuiColors):
//   Background — фон, ToolPath — рабочие ходы, G0 — холостые ходы,
//   Grid01/Grid05/Grid10 — сетка, CutArea — габаритный параллелепипед,
//   Pin — подсветка выбранных строк УП,
//   Home — маркер положения инструмента в начале подсвеченного участка.
// Стрелки осей в начале координат раскрашены в axisColor.
//
// Дуги G2/G3 рисуются одним из двух способов. Если видеокарта даёт контекст
// OpenGL 4.0 core, дуга уходит на GPU одним патчем и разбивается тесселятором
// под экранную длину — кривая гладкая на любом увеличении. Иначе дуга бьётся на
// хорды на CPU, а разбиение пересчитывается при изменении масштаба, чтобы
// прогиб хорды оставался меньше пикселя. Способ выбирается в конструкторе:
// формат контекста нужно запросить до создания окна.
class Viewer3d : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT

public:
    enum ViewPreset {
        Isometric,
        Top,
        Front,
        Left,
    };
    Q_ENUM(ViewPreset)

    explicit Viewer3d(QWidget* parent = nullptr);
    ~Viewer3d() override;

    // Разобрать текст УП и построить траекторию.
    void setProgramText(const QString& text);
    // Подсветить отрезки, полученные из строк [first, last] текста УП.
    // Отрицательный first снимает подсветку.
    void setHighlightedLines(int first, int last);
    // Перечитать цвета из настроек и перестроить буферы.
    void updateColors();

    void fitToView();
    void setViewPreset(ViewPreset preset);
    void setRapidsVisible(bool visible);
    bool rapidsVisible() const { return rapidsVisible_; }

    // Перспективная или ортогональная проекция; состояние запоминается в
    // настройках приложения.
    void setPerspective(bool enabled);
    bool perspective() const { return perspective_; }

    // Дуги считает GPU (аппаратная тесселяция), а не CPU.
    bool hardwareTessellation() const { return tessellation_; }

signals:
    // Пользователь ткнул мышью в отрезок траектории.
    void lineSelected(int lineNo);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Вершина с собственным цветом; раскладка совпадает с атрибутами шейдера.
    struct Vertex {
        float x, y, z;
        float r, g, b, a;
    };

    // Патч дуги для аппаратной тесселяции: одна вершина на дугу.
    struct ArcVertex {
        float cx, cy, cz; // центр
        float r, g, b, a; // цвет
        float start;      // начальный угол, рад
        float sweep;      // разворот со знаком, рад
        float radius;     //
        float plane;      // индекс плоскости дуги
        float n0, n1;     // координата по нормальной оси: начало и конец
    };

    void buildPathVertices();
    void addArcPatch(std::vector<ArcVertex>& out, const PathMove& move, const QColor& color);
    void buildAuxVertices();
    void buildGizmoVertices();
    void buildHighlightVertices();
    // moves_ -> segments_ с заданным допуском на прогиб хорды, мм.
    void tessellate(double tolerance);
    // Пересчитать разбиение дуг под текущий масштаб (только программный режим).
    void updateTessellation();
    // Размер экранного пикселя в мм на расстоянии точки интереса.
    float worldPerPixel() const;

    void drawVertices(QOpenGLBuffer& buffer, const std::vector<Vertex>& data, bool& dirty);
    void drawArcs(QOpenGLBuffer& buffer, const std::vector<ArcVertex>& data, bool& dirty);
    // Единичный вектор от точки интереса к камере.
    QVector3D cameraDir() const;
    QMatrix4x4 mvpMatrix() const;
    // Номер строки ближайшего к точке pos отрезка или -1.
    int pickLine(QPointF pos) const;

    std::vector<PathMove> moves_;          // разобранная УП
    std::vector<PathSegment> segments_;    // разбиение moves_ на хорды: пикинг и линии
    std::vector<Vertex> pathVertices_;     // траектория
    std::vector<Vertex> auxVertices_;      // сетка и габариты
    std::vector<Vertex> gizmoVertices_;    // стрелки осей с буквами
    std::vector<Vertex> hlVertices_;       // подсветка + маркер инструмента
    std::vector<ArcVertex> arcVertices_;   // дуги траектории (патчи)
    std::vector<ArcVertex> hlArcVertices_; // подсвеченные дуги (патчи)

    QOpenGLShaderProgram program_;
    QOpenGLShaderProgram arcProgram_; // с тесселяцией; пуста в программном режиме
    QOpenGLVertexArrayObject vao_;
    QOpenGLBuffer pathBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer auxBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer gizmoBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer hlBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer arcBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer hlArcBuffer_{QOpenGLBuffer::VertexBuffer};
    bool pathDirty_{true};
    bool auxDirty_{true};
    bool gizmoDirty_{true};
    bool hlDirty_{true};
    bool arcDirty_{true};
    bool hlArcDirty_{true};

    bool tessellation_{};     // дуги считает GPU
    double tessTolerance_{};  // текущий допуск программного разбиения, мм
    float lineWidthMax_{1.f}; // предел glLineWidth у контекста

    // Габариты траектории.
    QVector3D bbMin_{};
    QVector3D bbMax_{};
    float sceneRadius_{10.f};

    // Камера: точка интереса, азимут/возвышение в градусах и удаление.
    QVector3D center_{};
    float yaw_{-45.f};
    float pitch_{30.f};
    float distance_{50.f};
    static constexpr float fov_{45.f};

    // Подсвеченные строки УП.
    int hlFirstLine_{-1};
    int hlLastLine_{-1};

    bool rapidsVisible_{true};
    bool perspective_{true};
    bool viewTouched_{}; // пока камеру не крутили руками, вписываем при resize
    QPoint lastMousePos_;
    QPoint pressPos_;
    bool dragged_{};
};

} // namespace GCode
