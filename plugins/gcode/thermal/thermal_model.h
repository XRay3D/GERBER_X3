/*******************************************************************************
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

#include "thermal_vars.h"

#include <QAbstractItemModel>
#include <mvector.h>

namespace Thermal {

class Node;

class Model : public QAbstractItemModel {
    Q_OBJECT
    friend class Form;
    friend class Node;

    QIcon repaint(QColor color, const QIcon& icon) const;

    Node* const rootItem = nullptr;
    mvector<Node*> data_;

public:
    enum {
        Name,
        //        Position,
        GapAngle,
        apThickness,
        GapCount,
        ColumnCount
    };

    explicit Model(QObject* parent = nullptr);
    ~Model() override;

    virtual Node* appendRow(const QIcon& icon, const QString& name, const ThParam& par);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    ThParam thParam();

private:
    Node* getItem(const QModelIndex& index) const;
};

} // namespace Thermal
