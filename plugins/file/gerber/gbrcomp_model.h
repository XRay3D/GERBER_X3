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
#include <QAbstractItemModel>

namespace Gerber::Comp {

class sNode;

class sModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit sModel(int fileId, QObject* parent = nullptr);

    // QAbstractItemModel interface

    ~sModel();

    QModelIndex index(int row, int column, const QModelIndex& parent) const;

    QModelIndex parent(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    bool removeRows(int row, int count, const QModelIndex& parent);

    int columnCount(const QModelIndex& /*parent*/) const;
    int rowCount(const QModelIndex& parent) const;

signals:

private:
    sNode* rootItem;

    sNode* getItem(const QModelIndex& index) const;
};

} // namespace Gerber::Comp
