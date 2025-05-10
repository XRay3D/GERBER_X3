/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "shapepluginin.h"
#include <QAbstractTableModel>
#include <QActionGroup>
#include <QTableView>

namespace ShPoly {

class Shape;

class Model : public QAbstractTableModel {
    Q_OBJECT
    friend Shape;

public:
    Model(QObject* parent);
    virtual ~Model();

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override;
    int columnCount(const QModelIndex& = {}) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    std::vector<Shape*> shapes;
};

class Editor : public Shapes::Editor {
    Q_OBJECT

    QTableView* view;
    QActionGroup actionGroup{this};

public:
    Editor(Shapes::Plugin* plugin);

    void add(Shapes::AbstractShape* shape) override;
    void remove(Shapes::AbstractShape* shape) override;
    void updateData() override { view->reset(); }

    ~Editor() override = default;
    Model* model;
    Shapes::Plugin* plugin;
};

} // namespace ShPoly
