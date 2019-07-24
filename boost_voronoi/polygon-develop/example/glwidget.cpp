#include "glwidget.h"
#include <QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QTextStream>

#pragma comment (lib, "opengl32.lib")

GLWidget::GLWidget(QMainWindow* parent)
    : QGLWidget(/*QGLFormat(QGL::SampleBuffers),*/ parent)
    , primary_edges_only_(false)
    , internal_edges_only_(false)
{
    setFormat(QGLFormat(QGL::SampleBuffers));
    startTimer(40);
}

QSize GLWidget::sizeHint() const
{
    return QSize(600, 600);
}

void GLWidget::build(const QString& file_path)
{
    // Clear all containers.
    clear();

    // Read data.
    read_data(file_path);

    // No data, don't proceed.
    if (!brect_initialized_) {
        return;
    }

    // Construct bounding rectangle.
    construct_brect();

    // Construct voronoi diagram.
    construct_voronoi(
        point_data_.begin(), point_data_.end(),
        segment_data_.begin(), segment_data_.end(),
        &vd_);

    // Color exterior edges.
    for (const_edge_iterator it = vd_.edges().begin();
         it != vd_.edges().end(); ++it) {
        if (!it->is_finite()) {
            color_exterior(&(*it));
        }
    }

    // Update view port.
    update_view_port();
}

void GLWidget::show_primary_edges_only()
{
    primary_edges_only_ ^= true;
}

void GLWidget::show_internal_edges_only()
{
    internal_edges_only_ ^= true;
}

void GLWidget::initializeGL()
{
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
}

void GLWidget::paintGL()
{
    qglClearColor(QColor::fromRgb(255, 255, 255));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_points();
    draw_segments();
    draw_vertices();
    draw_edges();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::timerEvent(QTimerEvent* e)
{
    update();
}

void GLWidget::clear()
{
    brect_initialized_ = false;
    point_data_.clear();
    segment_data_.clear();
    vd_.clear();
}

void GLWidget::read_data(const QString& file_path)
{
    QFile data(file_path);
    if (!data.open(QFile::ReadOnly)) {
        QMessageBox::warning(
            this, tr("Voronoi Visualizer"),
            tr("Disable to open file ") + file_path);
    }
    QTextStream in_stream(&data);
    std::size_t num_points, num_segments;
    int x1, y1, x2, y2;
    in_stream >> num_points;
    for (std::size_t i = 0; i < num_points; ++i) {
        in_stream >> x1 >> y1;
        point_type p(x1, y1);
        update_brect(p);
        point_data_.push_back(p);
    }
    in_stream >> num_segments;
    for (std::size_t i = 0; i < num_segments; ++i) {
        in_stream >> x1 >> y1 >> x2 >> y2;
        point_type lp(x1, y1);
        point_type hp(x2, y2);
        update_brect(lp);
        update_brect(hp);
        segment_data_.push_back(segment_type(lp, hp));
    }
    in_stream.flush();
}

void GLWidget::update_brect(const GLWidget::point_type& point)
{
    if (brect_initialized_) {
        encompass(brect_, point);
    } else {
        set_points(brect_, point, point);
        brect_initialized_ = true;
    }
}

void GLWidget::construct_brect()
{
    double side = (std::max)(xh(brect_) - xl(brect_), yh(brect_) - yl(brect_));
    center(shift_, brect_);
    set_points(brect_, shift_, shift_);
    bloat(brect_, side * 1.2);
}

void GLWidget::color_exterior(const VD::edge_type* edge)
{
    if (edge->color() == EXTERNAL_COLOR) {
        return;
    }
    edge->color(EXTERNAL_COLOR);
    edge->twin()->color(EXTERNAL_COLOR);
    const VD::vertex_type* v = edge->vertex1();
    if (v == NULL || !edge->is_primary()) {
        return;
    }
    v->color(EXTERNAL_COLOR);
    const VD::edge_type* e = v->incident_edge();
    do {
        color_exterior(e);
        e = e->rot_next();
    } while (e != v->incident_edge());
}

void GLWidget::update_view_port()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    rect_type view_rect = brect_;
    deconvolve(view_rect, shift_);
    glOrtho(xl(view_rect), xh(view_rect),
        yl(view_rect), yh(view_rect),
        -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::draw_points()
{
    // Draw input points and endpoints of the input segments.
    glColor3f(0.0f, 0.5f, 1.0f);
    glPointSize(9);
    glBegin(GL_POINTS);
    for (std::size_t i = 0; i < point_data_.size(); ++i) {
        point_type point = point_data_[i];
        deconvolve(point, shift_);
        glVertex2f(point.x(), point.y());
    }
    for (std::size_t i = 0; i < segment_data_.size(); ++i) {
        point_type lp = low(segment_data_[i]);
        lp = deconvolve(lp, shift_);
        glVertex2f(lp.x(), lp.y());
        point_type hp = high(segment_data_[i]);
        hp = deconvolve(hp, shift_);
        glVertex2f(hp.x(), hp.y());
    }
    glEnd();
}

void GLWidget::draw_segments()
{
    // Draw input segments.
    glColor3f(0.0f, 0.5f, 1.0f);
    glLineWidth(2.7f);
    glBegin(GL_LINES);
    for (std::size_t i = 0; i < segment_data_.size(); ++i) {
        point_type lp = low(segment_data_[i]);
        lp = deconvolve(lp, shift_);
        glVertex2f(lp.x(), lp.y());
        point_type hp = high(segment_data_[i]);
        hp = deconvolve(hp, shift_);
        glVertex2f(hp.x(), hp.y());
    }
    glEnd();
}

void GLWidget::draw_vertices()
{
    // Draw voronoi vertices.
    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(6);
    glBegin(GL_POINTS);
    for (const_vertex_iterator it = vd_.vertices().begin();
         it != vd_.vertices().end(); ++it) {
        if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
            continue;
        }
        point_type vertex(it->x(), it->y());
        vertex = deconvolve(vertex, shift_);
        glVertex2f(vertex.x(), vertex.y());
    }
    glEnd();
}

void GLWidget::draw_edges()
{
    // Draw voronoi edges.
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.7f);
    for (const_edge_iterator it = vd_.edges().begin();
         it != vd_.edges().end(); ++it) {
        if (primary_edges_only_ && !it->is_primary()) {
            continue;
        }
        if (internal_edges_only_ && (it->color() == EXTERNAL_COLOR)) {
            continue;
        }
        std::vector<point_type> samples;
        if (!it->is_finite()) {
            clip_infinite_edge(*it, &samples);
        } else {
            point_type vertex0(it->vertex0()->x(), it->vertex0()->y());
            samples.push_back(vertex0);
            point_type vertex1(it->vertex1()->x(), it->vertex1()->y());
            samples.push_back(vertex1);
            if (it->is_curved()) {
                sample_curved_edge(*it, &samples);
            }
        }
        glBegin(GL_LINE_STRIP);
        for (std::size_t i = 0; i < samples.size(); ++i) {
            point_type vertex = deconvolve(samples[i], shift_);
            glVertex2f(vertex.x(), vertex.y());
        }
        glEnd();
    }
}

void GLWidget::clip_infinite_edge(const GLWidget::edge_type& edge, std::vector<GLWidget::point_type>* clipped_edge)
{
    const cell_type& cell1 = *edge.cell();
    const cell_type& cell2 = *edge.twin()->cell();
    point_type origin, direction;
    // Infinite edges could not be created by two segment sites.
    if (cell1.contains_point() && cell2.contains_point()) {
        point_type p1 = retrieve_point(cell1);
        point_type p2 = retrieve_point(cell2);
        origin.x((p1.x() + p2.x()) * 0.5);
        origin.y((p1.y() + p2.y()) * 0.5);
        direction.x(p1.y() - p2.y());
        direction.y(p2.x() - p1.x());
    } else {
        origin = cell1.contains_segment() ? retrieve_point(cell2) : retrieve_point(cell1);
        segment_type segment = cell1.contains_segment() ? retrieve_segment(cell1) : retrieve_segment(cell2);
        coordinate_type dx = high(segment).x() - low(segment).x();
        coordinate_type dy = high(segment).y() - low(segment).y();
        if ((low(segment) == origin) ^ cell1.contains_point()) {
            direction.x(dy);
            direction.y(-dx);
        } else {
            direction.x(-dy);
            direction.y(dx);
        }
    }
    coordinate_type side = xh(brect_) - xl(brect_);
    coordinate_type koef = side / (std::max)(fabs(direction.x()), fabs(direction.y()));
    if (edge.vertex0() == NULL) {
        clipped_edge->push_back(point_type(
            origin.x() - direction.x() * koef,
            origin.y() - direction.y() * koef));
    } else {
        clipped_edge->push_back(
            point_type(edge.vertex0()->x(), edge.vertex0()->y()));
    }
    if (edge.vertex1() == NULL) {
        clipped_edge->push_back(point_type(
            origin.x() + direction.x() * koef,
            origin.y() + direction.y() * koef));
    } else {
        clipped_edge->push_back(
            point_type(edge.vertex1()->x(), edge.vertex1()->y()));
    }
}

void GLWidget::sample_curved_edge(const GLWidget::edge_type& edge, std::vector<GLWidget::point_type>* sampled_edge)
{
    coordinate_type max_dist = 1E-3 * (xh(brect_) - xl(brect_));
    point_type point = edge.cell()->contains_point() ? retrieve_point(*edge.cell()) : retrieve_point(*edge.twin()->cell());
    segment_type segment = edge.cell()->contains_point() ? retrieve_segment(*edge.twin()->cell()) : retrieve_segment(*edge.cell());
    voronoi_visual_utils<coordinate_type>::discretize(
        point, segment, max_dist, sampled_edge);
}

GLWidget::point_type GLWidget::retrieve_point(const GLWidget::cell_type& cell)
{
    source_index_type index = cell.source_index();
    source_category_type category = cell.source_category();
    if (category == SOURCE_CATEGORY_SINGLE_POINT) {
        return point_data_[index];
    }
    index -= point_data_.size();
    if (category == SOURCE_CATEGORY_SEGMENT_START_POINT) {
        return low(segment_data_[index]);
    } else {
        return high(segment_data_[index]);
    }
}

GLWidget::segment_type GLWidget::retrieve_segment(const GLWidget::cell_type& cell)
{
    source_index_type index = cell.source_index() - point_data_.size();
    return segment_data_[index];
}
