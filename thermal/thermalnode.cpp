// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
    m_item->m_node = this;
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

ThermalNode::~ThermalNode() { childItems.clear(); }

ThermalNode* ThermalNode::child(int row) const { return childItems.value(row).data(); }

ThermalNode* ThermalNode::parentItem() { return m_parentItem; }

int ThermalNode::childCount() const { return childItems.count(); }

int ThermalNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i] == this)
                return i;
    return 0;
}

void ThermalNode::append(ThermalNode* node)
{
    node->m_parentItem = this;
    childItems.append(QSharedPointer<ThermalNode>(node));
}

void ThermalNode::remove(int row) { childItems.removeAt(row); }

bool ThermalNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    {
        const QModelIndex i1(model->createIndex(row(), ThermalModel::Name, this));
        const QModelIndex i2(model->createIndex(row(), ThermalModel::GapCount, this));
        model->dataChanged(i1, i2, { role });
    }

    if (role == Qt::CheckStateRole && !index.column()) {
        m_checked = (value.value<Qt::CheckState>() == Qt::Checked);
        if (container) {
            for (auto node : childItems)
                node->setData(index, value, role);
        } else {
            m_item->mouseDoubleClickEvent(nullptr);
        }
        return true;
    } else if (role == Qt::EditRole) {
        if (container) {
            auto childItems(this->childItems);
            if (container && childItems.isEmpty())
                childItems = m_parentItem->childItems.mid(1);
            switch (index.column()) {
            case ThermalModel::Name:
            case ThermalModel::Position:
                return false;
            case ThermalModel::GapAngle:
                for (auto node : childItems) {
                    node->setData(index, value, role);
                }
                m_angle = value.toDouble();
                return true;
            case ThermalModel::apThickness:
                for (auto node : childItems) {
                    node->setData(index, value, role);
                }
                m_tickness = value.toDouble();
                return true;
            case ThermalModel::GapCount:
                for (auto node : childItems) {
                    node->setData(index, value, role);
                }
                m_count = value.toInt();
                return true;
            }
            return false;
        } else {
            switch (index.column()) {
            case ThermalModel::Name:
            case ThermalModel::Position:
                return false;
            case ThermalModel::GapAngle:
                m_item->setAngle(value.toDouble());
                return true;
            case ThermalModel::apThickness:
                m_item->setTickness(value.toDouble());
                return true;
            case ThermalModel::GapCount:
                m_item->setCount(value.toInt());
                return true;
            }
        }
    }
    return false;
}

Qt::ItemFlags ThermalNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!index.column())
        flags |= Qt::ItemIsUserCheckable;
    if (index.column() > ThermalModel::Position)
        flags |= Qt::ItemIsEditable;
    if (!container)
        flags |= Qt::ItemNeverHasChildren;
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
            case ThermalModel::Name:
                return name;
            case ThermalModel::Position:
                return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
            case ThermalModel::GapAngle:
                return m_angle;
            case ThermalModel::apThickness:
                return m_tickness;
            case ThermalModel::GapCount:
                return m_count;
            }
        } else {
            switch (index.column()) {
            case ThermalModel::Name:
                return name;
            case ThermalModel::Position:
                return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
            case ThermalModel::GapAngle:
                return m_item->angle();
            case ThermalModel::apThickness:
                return m_item->tickness();
            case ThermalModel::GapCount:
                return m_item->count();
            }
        }
        return {};
    case Qt::DecorationRole:
        if (!index.column()) {
            if (icon.isNull()) {
                QPixmap p(24, 24);
                p.fill(Qt::transparent);
                return QIcon(p);
            }
            return icon;
        }
        return {};
    case Qt::CheckStateRole:
        if (index.column())
            return {};
        if (container) {
            int val = 0;
            for (auto node : childItems) {
                val |= node->m_checked ? 2 : 1;
            }
            return chState[val];
        } else
            return m_checked ? Qt::Checked : Qt::Unchecked;
    case Qt::TextAlignmentRole:
        if (index.column())
            return Qt::AlignCenter;
        return {};
    default:
        break;
    }
    return {};
}

double ThermalNode::angle() const { return m_angle; }

double ThermalNode::tickness() const { return m_tickness; }

int ThermalNode::count() const { return m_count; }

IntPoint ThermalNode::pos() const { return m_pos; }

ThermalPreviewItem* ThermalNode::item() const { return m_item; }

bool ThermalNode::createFile() const { return m_checked && m_item; }

void ThermalNode::disable()
{
    m_checked = false;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

void ThermalNode::enable()
{
    m_checked = true;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

QModelIndex ThermalNode::index() const { return model->createIndex(row(), 0, reinterpret_cast<quintptr>(this)); }

bool ThermalNode::isChecked() const { return m_checked; }
