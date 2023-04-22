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

#include <QAbstractTableModel>
#include <QActionGroup>
#include <QTableView>
#include <QWidget>

namespace ShTxt {

class Model : public QAbstractTableModel {
    Q_OBJECT
    friend class Shape;
    QStringList headerData_{
        tr(" Pos "),
        tr(" Text "),
        tr(" Font "),
        tr(" Angle "),
        tr(" Height "),
        tr(" X/Y "),
        tr(" Handle Align "),
        tr(" Side "),
    };

public:
    Model(QObject* parent);
    virtual ~Model();

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& = {}) const override { return headerData_.size(); }
    int columnCount(const QModelIndex& = {}) const override { return 2; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override { return Qt::ItemIsEditable | Qt::ItemIsEnabled; }
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    std::vector<Shape*> shapes;
};

class Editor : public QWidget {
    Q_OBJECT

    QTableView* view;
    QActionGroup actionGroup{this};

public:
    Editor(class Plugin* plugin);

    void addShape(Shape* shape);

    ~Editor() override = default;
    Model* model;
    class Plugin* plugin;
};

} // namespace ShTxt
