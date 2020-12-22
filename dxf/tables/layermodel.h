#pragma once
#include "layer.h"
#include <QAbstractTableModel>
namespace Dxf {
class LayerModel : public QAbstractTableModel {
    Q_OBJECT

    Layers layers;
    const QStringList names;

public:
    explicit LayerModel(Layers layers, QObject* parent = nullptr);

signals:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
}
