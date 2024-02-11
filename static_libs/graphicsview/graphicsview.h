/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSettings>

class Ruler;
class QGridLayout;
// class Scene;

class GraphicsView : public QGraphicsView {
    Q_OBJECT

    Q_PROPERTY(double scale READ getScale WRITE setScale)
    Q_PROPERTY(QRectF viewRect READ getViewRect WRITE setViewRect)

public:
    explicit GraphicsView(QWidget* parent = nullptr);
    ~GraphicsView() override;
    //    void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomToSelected();
    void zoomIn();
    void zoomOut();
    void fitInView(QRectF destRect, bool withBorders = true);

    void setRuler(bool ruller);

    double scaleFactor();
    QPointF mappedPos(QMouseEvent* event) const;

    void setScale(double s) noexcept;
    double getScale() noexcept;

    void setOpenGL(bool useOpenGL);

    void setViewRect(const QRectF& r);
    QRectF getViewRect();
    QRectF getSelectedBoundingRect();
    bool boundingRectFl() const { return boundingRect_; }

signals:
    void fileDroped(const QString&);
    void mouseMove(const QPointF&);
    void mouseMove2(const QPointF&, const QPointF&);
    void mouseClickR(const QPointF&);
    void mouseClickL(const QPointF&);

private:
    Ruler* const hRuler;
    Ruler* const vRuler;
    QGridLayout* const gridLayout;
    //    Scene* scene_;
    bool ruler_ {};
    int rulerCtr {};
    bool boundingRect_ {};
    void updateRuler();
    template <class T>
    void animate(QObject* target, const QByteArray& propertyName, T begin, T end);
    QPoint latPos;
    QPointF point, rulPt1, rulPt2;

    void drawRuller(QPainter* painter, const QRectF& rect) const;
    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
};

#include "app.h"
