#pragma once



#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSettings>

class QDRuler;
class Scene;

class GraphicsView : public QGraphicsView {
    Q_OBJECT

    Q_PROPERTY(double scale READ getScale WRITE setScale)

public:
    explicit GraphicsView(QWidget* parent = nullptr);
    ~GraphicsView() override;
    void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomToSelected();
    void zoomIn();
    void zoomOut();

    double scaleFactor();
    QPointF mappedPos(QMouseEvent* event) const;

    void setScale(double s);
    double getScale();
signals:
    void fileDroped(const QString&);
    void mouseMove(const QPointF&);

private:
    QDRuler* hRuler;
    QDRuler* vRuler;
    Scene* m_scene;

    void updateRuler();
    template <class T>
    void anim(QObject* target, const QByteArray& propertyName, T begin, T end);

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

#include <app.h>


