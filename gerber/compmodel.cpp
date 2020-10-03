#include "compmodel.h"
#include "app.h"
#include "compnode.h"
#include "gbrfile.h"
#include "project.h"

#include "leakdetector.h"

namespace Gerber {

ComponentsModel::ComponentsModel(int fileId, QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new ComponentsNode(""))
{
    auto file = App::project()->file<Gerber::File>(fileId);

    QMap<QString, QVector<QPair<int, ComponentsNode*>>> map;

    auto unsorted = new ComponentsNode("unsorted");

    for (const auto& c : file->components()) {
        static const QRegExp rx("(\\D+)(\\d+).*");
        if (rx.exactMatch(c.refdes)) {
            if (map[rx.cap(1)].isEmpty())
                map[rx.cap(1)].append({ -1, new ComponentsNode(rx.cap(1)) });
            map[rx.cap(1)].append({ rx.cap(2).toInt(), new ComponentsNode(c) });
        } else {
            unsorted->append(new ComponentsNode(c));
        }
    }

    auto it = map.begin();
    while (it != map.end()) {
        std::sort(
            it.value().begin(),
            it.value().end(),
            [](const QPair<int, ComponentsNode*>& p1, const QPair<int, ComponentsNode*>& p2) {
                return p1.first < p2.first;
            });

        for (int i = 1; i < it.value().size(); ++i) {
            it.value().first().second->append(it.value()[i].second);
        }

        rootItem->append(it.value().first().second);
        ++it;
    }
    if (unsorted->childCount() > 0)
        rootItem->append(unsorted);
    else
        delete unsorted;
}

ComponentsModel::~ComponentsModel()
{
    delete rootItem;
    //    App::m_ComponentsModel = nullptr;
}

QModelIndex ComponentsModel::index(int row, int column, const QModelIndex& parent) const
{
    ComponentsNode* childItem = getItem(parent)->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex ComponentsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    ComponentsNode* parentItem = getItem(index)->parentItem();
    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant ComponentsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ComponentsNode* item = getItem(index);

    return item->data(index, role);
}

bool ComponentsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return getItem(index)->setData(index, value, role);
}

QVariant ComponentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        switch (section) {
        case 0: /* <field> Manufacturer. */
            return tr("Ref Des");
        case 1: /* <field> Manufacturer part number. */
            return tr("Manufacturer\n"
                      "part\n"
                      "number");
        case 2: /* <field> E.g. 220nF. */
            return tr("Value");
        case 3: /* (TH|SMD|BGA|Other) Mount type. */
            return tr("Mount\n"
                      "type");
        case 4: /* <field> Footprint name. It is strongly recommended to comply with the IPC-7351 footprint names and pin numbering for all standard components. */
            return tr("Footprint\n"
                      "name");
        case 5: /* <field> Package name. It is strongly recommended to comply with the JEDEC JEP95 standard. */
            return tr("Package\n"
                      "name");
        case 6: /* <field> Package description. */
            return tr("Package\n"
                      "name");
        case 7: /* <decimal> Height, in the unit of the file. */
            return tr("Height");
        default:
            return QString("");
        }
    return QVariant();
}

Qt::ItemFlags ComponentsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return getItem(index)->flags(index);
}

bool ComponentsModel::removeRows(int row, int count, const QModelIndex& parent)
{
    ComponentsNode* item = nullptr;
    if (parent.isValid())
        item = static_cast<ComponentsNode*>(parent.internalPointer());
    else
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

int ComponentsModel::columnCount(const QModelIndex&) const
{
    return 8;
}

int ComponentsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

ComponentsNode* ComponentsModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* item = static_cast<ComponentsNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

}
