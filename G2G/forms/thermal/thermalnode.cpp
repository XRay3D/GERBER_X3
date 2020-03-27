#include "thermalnode.h"
#include "thermalmodel.h"

ThermalNode::ThermalNode(const QIcon& icon, const QString& name, double angle, double tickness, int count, const IntPoint& pos, ThermalPreviewItem* item)
    : container(false)
    , icon(icon)
    , name(name)
    , m_angle(angle)
    , m_tickness(tickness)
    , m_count(count)
    , m_pos(pos)
    , m_item(item)
{
    item->m_node = this;
}

ThermalNode::ThermalNode(const QIcon& icon, const QString& name)
    : container(true)
    , icon(icon)
    , name(name)
    , m_angle(0)
    , m_tickness(0.5)
    , m_count(4)
{
}

ThermalNode::~ThermalNode()
{
    childItems.clear();
}

ThermalNode* ThermalNode::child(int row) const
{
    return childItems.value(row).data();
}

ThermalNode* ThermalNode::parentItem()
{
    return m_parentItem;
}

int ThermalNode::childCount() const
{
    return childItems.count();
}

int ThermalNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i].data() == this)
                return i;
    return 0;
}

void ThermalNode::append(ThermalNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<ThermalNode>(item));
}

void ThermalNode::remove(int row) { childItems.removeAt(row); }

bool ThermalNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const QModelIndex i1(ThermalModel::m_instance->createIndex(row(), ThermalModel::ThName, this));
    const QModelIndex i2(ThermalModel::m_instance->createIndex(row(), ThermalModel::ThGapCount, this));
    ThermalModel::m_instance->dataChanged(i1, i2, { role });

    switch (role) {
    case Qt::EditRole:
        if (container && childItems.isEmpty()) {
            auto childItems(m_parentItem->childItems);
            childItems.takeFirst();
            switch (index.column()) {
            case 0:
            case 1:
                return false;
            case 2:
                for (QSharedPointer<ThermalNode> item : childItems)
                    item->setData(index, value, role);
                m_angle = value.toDouble();
                return true;
            case 3:
                for (QSharedPointer<ThermalNode> item : childItems)
                    item->setData(index, value, role);
                m_tickness = value.toDouble();
                return true;
            case 4:
                for (QSharedPointer<ThermalNode> item : childItems)
                    item->setData(index, value, role);
                m_count = value.toInt();
                return true;
            }
            return false;
        } else if (container) {
            switch (index.column()) {
            case 0:
            case 1:
                return false;
            case 2:
                for (QSharedPointer<ThermalNode> item : childItems) {
                    item->setData(index, value, role);
                }
                m_angle = value.toDouble();
                return true;
            case 3:
                for (QSharedPointer<ThermalNode> item : childItems) {
                    item->setData(index, value, role);
                }
                m_tickness = value.toDouble();
                return true;
            case 4:
                for (QSharedPointer<ThermalNode> item : childItems) {
                    item->setData(index, value, role);
                }
                m_count = value.toInt();
                return true;
            }
            return false;
        } else {
            switch (index.column()) {
            case 0:
            case 1:
                return false;
            case 2:
                m_item->setAngle(value.toDouble());
                return true;
            case 3:
                m_item->setTickness(value.toDouble());
                return true;
            case 4:
                m_item->setCount(value.toInt());
                return true;
            }
        }
        return false;
    case Qt::CheckStateRole:
        if (!index.column()) {
            m_checkState = value.toBool() ? Qt::Checked : Qt::Unchecked;
            if (m_item) {
                if (value.toBool()) {
                    m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                    m_item->setSelected(selected);
                } else {
                    selected = m_item->isSelected();
                    m_item->setFlag(QGraphicsItem::ItemIsSelectable, value.toBool());
                }
            }
            for (QSharedPointer<ThermalNode> item : childItems) {
                item->setData(index, value, role);
            }
            return true;
        }
        return false;
    default:
        break;
    }
    return false;
}

Qt::ItemFlags ThermalNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!index.column())
        flags |= Qt::ItemIsUserCheckable;
    if (index.column() > 1 /*&& !container*/)
        flags |= Qt::ItemIsEditable;
    if (!container)
        flags |= Qt::ItemNeverHasChildren;
    if (m_checkState == Qt::Checked)
        flags |= Qt::ItemIsSelectable;
    return flags;
}

QVariant ThermalNode::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (container) {
            switch (index.column()) {
            case 0:
                return name;
            case 1:
                return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
            case 2:
                return m_angle;
            case 3:
                return m_tickness;
            case 4:
                return m_count;
            }
        } else {
            switch (index.column()) {
            case 0:
                return name;
            case 1:
                return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
            case 2:
                return m_item->angle();
            case 3:
                return m_item->tickness();
            case 4:
                return m_item->count();
            }
        }
        return QVariant();
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (icon.isNull()) {
                QPixmap p(24, 24);
                p.fill(Qt::white);
                return QIcon(p);
            }
            return icon;
        }
        return QVariant();
    case Qt::CheckStateRole:
        if (index.column())
            return QVariant();
        if (container) {
            int val = 0;
            for (QSharedPointer<ThermalNode> item : childItems) {
                if (item->m_checkState == Qt::Unchecked)
                    val |= 1;
                else if (item->m_checkState == Qt::Checked)
                    val |= 2;
            }
            static const Qt::CheckState chState[]{
                Qt::Unchecked, // index 0
                Qt::Unchecked, // index 1
                Qt::Checked, // index 2
                Qt::PartiallyChecked // index 3
            };
            return chState[val];
        } else
            return m_checkState;
    case Qt::TextAlignmentRole:
        if (index.column())
            return Qt::AlignCenter;
        return QVariant();
    default:
        break;
    }
    return QVariant();
}

double ThermalNode::angle() const { return m_angle; }

double ThermalNode::tickness() const { return m_tickness; }

int ThermalNode::count() const { return m_count; }

IntPoint ThermalNode::pos() const { return m_pos; }

ThermalPreviewItem* ThermalNode::item() const { return m_item; }

bool ThermalNode::createFile() const { return m_checkState == Qt::Checked && !container; }

void ThermalNode::disable()
{
    m_checkState = Qt::Unchecked;
    selected = false;
    m_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void ThermalNode::enable()
{
    m_checkState = Qt::Checked;
    selected = true;
    m_item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_item->setSelected(true);
}
