/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_layer.h"
#include <QAbstractTableModel>
#include <QStyledItemDelegate>

namespace Dxf {

QStringList keys(const Layers& layers);

class LayerModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit LayerModel(Layers layers, QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    enum Column {
        Visible,
        EntityCount,
        Type,
        ColumnCount
    };

private:
    Layers layers;
    const QStringList names;
};

class ItemsTypeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ItemsTypeDelegate(QObject* parent = nullptr);
    ~ItemsTypeDelegate() override = default;

public:
    // QAbstractItemDelegate interface
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    void emitCommitData();
};

}
