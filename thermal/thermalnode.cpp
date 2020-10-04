// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermalnode.h"
#include "thermalmodel.h"

ThermalNode::ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, const IntPoint& pos, ThermalPreviewItem* item)
    : container(false)
    , icon(icon)
    , name(name)
    , m_pos(pos)
    , par(par)
    , m_item(item)
{
    m_item->m_node = this;
    m_item->redraw();
}

ThermalNode::ThermalNode(const QIcon& icon, const QString& name, const ThParam& par)
    : container(true)
    , icon(icon)
    , name(name)
    , par(par)
{
}

ThermalNode::ThermalNode(ThermalModel* _model) { model = _model; }

ThermalNode::~ThermalNode() { childs.clear(); }

ThermalNode* ThermalNode::child(int row) const { return childs.value(row).data(); }

ThermalNode* ThermalNode::parentItem() { return m_parent; }

int ThermalNode::childCount() const { return childs.count(); }

int ThermalNode::row() const
{
    if (m_parent)
        for (int i = 0, size = m_parent->childs.size(); i < size; ++i)
            if (m_parent->childs[i] == this)
                return i;
    return 0;
}

void ThermalNode::append(ThermalNode* node)
{
    node->m_parent = this;
    childs.append(QSharedPointer<ThermalNode>(node));
}

void ThermalNode::remove(int row) { childs.removeAt(row); }

bool ThermalNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && !index.column()) {
        static bool updateGuard = false;
        m_checked = (value.value<Qt::CheckState>() == Qt::Checked);
        if (container) {
            auto childItems(this->childs);
            if (container && childItems.isEmpty())
                childItems = m_parent->childs.mid(1);
            updateGuard = true;
            for (auto node : childItems)
                node->setData(index, value, role);
            model->dataChanged(childItems.first()->index(), childItems.last()->index(), { role });
            updateGuard = false;
        } else {
            if (!updateGuard)
                model->dataChanged(m_parent->index(), m_parent->index(), { role });
            m_item->mouseDoubleClickEvent(nullptr);
        }
        return true;
    } else if (role == Qt::EditRole) {
        if (container) {
            auto childItems(this->childs);
            if (container && childItems.isEmpty())
                childItems = m_parent->childs.mid(1);
            switch (index.column()) {
            case ThermalModel::Name:
            case ThermalModel::Position:
                return false;
            case ThermalModel::GapAngle:
                par.angle = value.toDouble();
                for (auto node : childItems)
                    node->setData(index, value, role);
                return true;
            case ThermalModel::apThickness:
                par.tickness = value.toDouble();
                for (auto node : childItems)
                    node->setData(index, value, role);
                return true;
            case ThermalModel::GapCount:
                par.count = value.toInt();
                for (auto node : childItems)
                    node->setData(index, value, role);
                return true;
            }
            return false;
        } else {
            switch (index.column()) {
            case ThermalModel::Name:
            case ThermalModel::Position:
                return false;
            case ThermalModel::GapAngle:
                par.angle = value.toDouble();
                m_item->redraw();
                return true;
            case ThermalModel::apThickness:
                par.tickness = value.toDouble();
                m_item->redraw();
                return true;
            case ThermalModel::GapCount:
                par.count = value.toInt();
                m_item->redraw();
                return true;
            }
        }
    }
    return false;
}

QVariant ThermalNode::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case ThermalModel::Name:
            return name;
        case ThermalModel::Position:
            return QString("%1 : %2").arg(m_pos.X * dScale).arg(m_pos.Y * dScale).replace('.', ',');
        case ThermalModel::GapAngle:
            return par.angle;
        case ThermalModel::apThickness:
            return par.tickness;
        case ThermalModel::GapCount:
            return par.count;
        }
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
            if (childs.isEmpty())
                return m_checked ? Qt::Checked : Qt::Unchecked;
            int val = 0;
            for (auto node : childs) {
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

double ThermalNode::angle() const { return par.angle; }

double ThermalNode::tickness() const { return par.tickness; }

int ThermalNode::count() const { return par.count; }

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

QModelIndex ThermalNode::index(int column) const { return model->createIndex(row(), column, reinterpret_cast<quintptr>(this)); }

bool ThermalNode::isChecked() const { return m_checked; }
