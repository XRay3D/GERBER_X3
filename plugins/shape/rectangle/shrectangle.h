/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "gi.h"
#include "shape.h"
#include "shapepluginin.h"
#include <QActionGroup>
#include <QJsonObject>
#include <QtWidgets>

namespace Rectangle_ {

class Shape final : public Shapes::AbstractShape {
    friend class Model;

public:
    explicit Shape(QPointF pt1 = {}, QPointF pt2 = {});
    ~Shape() override = default;

    // QGraphicsItem interface
    int type() const override { return GiType::ShRectangle; }
    // GraphicsItem interface
    void redraw() override;
    // AbstractShape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt) override;
    enum {
        Center,
        Point1,
        Point2,
        Point3,
        Point4,
        PtCount
    };
    class Model* model{};
};

class Model : public QAbstractTableModel {
    Q_OBJECT
    friend class Shape;
    QStringList headerData_{
        tr("  Width  "),
        tr("  Height  "),
        tr("  Center  "),
        tr("  Point 1  "),
        tr("  Point 2  "),
        tr("  Point 3  "),
        tr("  Point 4  "),
    };
    Shape* shape{};

public:
    Model(QObject* parent);
    virtual ~Model();

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override { return headerData_.size(); }
    int columnCount(const QModelIndex& = {}) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    void setShape(Shape* shape_) { shape = shape_; }
};

class Editor : public QWidget {
    Q_OBJECT

    QTableView* view;
    QActionGroup actionGroup{this};

public:
    Editor(class Plugin* plugin);

    void setShape(Shape* shape) {
        model->setShape(shape);
        shape->model = model;
        view->reset();
    }

    ~Editor() override = default;
    Model* model;
    class Plugin* plugin;
};

class Plugin : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "rectangle.json")
    Q_INTERFACES(Shapes::Plugin)

    mutable Editor editor_{this};

public:
    // Shapes::Plugin interface
    uint32_t type() const override { return GiType::ShRectangle; }
    QIcon icon() const override { return QIcon::fromTheme("draw-rectangle"); }
    Shapes::AbstractShape* createShape(const QPointF& point = {}) const override {
        auto shape = new Shape(point, point + QPointF{10, 10});
        editor_.setShape(shape);
        return shape;
    }
    QWidget* editor() override { return &editor_; };
};

} // namespace Rectangle_
