#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSettings>

//#define ANIM

class QDRuler;
//class Ruler;
class Scene;

class GraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit GraphicsView(QWidget* parent = nullptr);
    ~GraphicsView() override;
    void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomToSelected();
    void zoomIn();
    void zoomOut();
    static GraphicsView* self;
    static double scaleFactor() { return 1.0 / GraphicsView::self->matrix().m11(); }
    QPointF mappedPos(QMouseEvent* event) const;

signals:
    void fileDroped(const QString&);
    void mouseMove(const QPointF&);

private:
    QDRuler* hRuler;
    QDRuler* vRuler;
    //    Ruler* ruller;
    Scene* m_scene;
    const double zoomFactor = 1.5;

#ifdef ANIM
    void AnimFinished();
    void ScalingTime(qreal x);
    QPointF centerPoint;
    double zoom = 100;
    double numScheduledScalings = 0;
#endif

    void UpdateRuler();

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
};

#endif // VIEW_H
