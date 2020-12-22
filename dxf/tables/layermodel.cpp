#include "layermodel.h"
#include "dxfvalues.h"
#include "layer.h"
#include <QGraphicsItemGroup>
#include <QPainter>
namespace Dxf {
static QStringList keys(const Layers& layers)

{
    QStringList sl;
    sl.reserve(layers.size());
    for (auto& [key, _] : layers)
        sl.append(key);
    return sl;
}

LayerModel::LayerModel(Layers layers, QObject* parent)
    : QAbstractTableModel(parent)
    , layers(layers)
    , names(keys(layers))
{
}

int LayerModel::rowCount(const QModelIndex& /*parent*/) const { return names.size(); }

int LayerModel::columnCount(const QModelIndex& /*parent*/) const { return 1; }

QVariant LayerModel::data(const QModelIndex& index, int role) const
{
    //    if (role == Qt::DisplayRole) {
    //        if (layers.at(names[index.row()])->gig)
    //            return layers.at(names[index.row()])->gig->isVisible();
    //    } else if (role == Qt::CheckStateRole) {
    //        if (layers.at(names[index.row()])->gig)
    //            return (layers.at(names[index.row()])->gig->isVisible()) ? Qt::Checked : Qt::Unchecked;
    //    } else if (role == Qt::EditRole) {
    //        if (layers.at(names[index.row()])->gig)
    //            return static_cast<int>(layers.at(names[index.row()])->gig->isVisible());
    //    } else if (role == Qt::DecorationRole) {
    //        QColor color(dxfColors[layers.at(names[index.row()])->colorNumber]);
    //        color.setAlpha(255);
    //        QPixmap pixmap(22, 22);
    //        pixmap.fill(Qt::transparent);
    //        QPainter p(&pixmap);
    //        p.setBrush(color);
    //        p.drawRect(3, 3, 15, 15);
    //        return pixmap;
    //    }
    return {};
}

bool LayerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    //    if (role == Qt::EditRole) {
    //        layers[names[index.row()]]->gig->setVisible(value.toInt());
    //    } else if (role == Qt::CheckStateRole) {
    //        layers[names[index.row()]]->gig->setVisible(value.toInt());
    //    }
    return true;
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return "Visible";
        } else {
            return names[section];
        }
    }
    return QAbstractTableModel ::headerData(section, orientation, role);
}

Qt::ItemFlags LayerModel::flags(const QModelIndex& /*index*/) const { return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable; }
}
