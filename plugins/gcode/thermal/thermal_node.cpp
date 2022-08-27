// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_node.h"
#include "thermal_model.h"

ThermalNode::ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, const IntPoint& pos, AbstractThermPrGi* item, ThermalModel* model)
    : container(false)
    , icon(icon)
    , name(name)
    , pos_(pos)
    , par(par)
    , item_(item)
    , model(model) {
    item_->node_ = this;
    item_->redraw();
}

ThermalNode::ThermalNode(const QIcon& icon, const QString& name, const ThParam& par, ThermalModel* model)
    : container(true)
    , icon(icon)
    , name(name)
    , par(par)
    , item_(nullptr)
    , model(model) {
}

ThermalNode::ThermalNode(ThermalModel* model)
    : item_(nullptr)
    , model(model) {
}

ThermalNode::~ThermalNode() { childs.clear(); }

ThermalNode* ThermalNode::child(int row) const { return childs.at(row).get(); }

ThermalNode* ThermalNode::parentItem() { return parent_; }

int ThermalNode::childCount() const { return static_cast<int>(childs.size()); }

int ThermalNode::row() const {
    if (parent_)
        for (size_t i = 0, size = parent_->childs.size(); i < size; ++i)
            if (parent_->childs[i].get() == this)
                return static_cast<int>(i);
    return 0;
}

void ThermalNode::append(ThermalNode* item) {
    item->parent_ = this;
    childs.emplace_back(std::shared_ptr<ThermalNode>(item));
}

void ThermalNode::remove(int row) { childs.remove(row); }

bool ThermalNode::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role == Qt::CheckStateRole && !index.column()) {
        static bool updateGuard = false;
        checked_ = (value.value<Qt::CheckState>() == Qt::Checked);
        if (container) {
            auto childItems(this->childs);
            if (container && childItems.empty())
                childItems = parent_->childs.mid(1);
            updateGuard = true;
            for (auto node : qAsConst(childItems))
                node->setData(index, value, role);
            if (childItems.size())
                emit model->dataChanged(childItems.front()->index(), childItems.back()->index(), {role});
            updateGuard = false;
        } else {
            if (!updateGuard)
                emit model->dataChanged(parent_->index(), parent_->index(), {role});
            item_->mouseDoubleClickEvent(nullptr);
        }
        return true;
    } else if (role == Qt::EditRole) {
        if (container) {
            auto childItems(this->childs);
            if (container && childItems.empty())
                childItems = parent_->childs.mid(1);
            switch (index.column()) {
            case ThermalModel::Name:
            case ThermalModel::Position:
                return false;
            case ThermalModel::GapAngle:
                par.angle = value.toDouble();
                for (auto node : qAsConst(childItems))
                    node->setData(index, value, role);
                return true;
            case ThermalModel::apThickness:
                par.tickness = value.toDouble();
                for (auto node : qAsConst(childItems))
                    node->setData(index, value, role);
                return true;
            case ThermalModel::GapCount:
                par.count = value.toInt();
                for (auto node : qAsConst(childItems))
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
                item_->redraw();
                return true;
            case ThermalModel::apThickness:
                par.tickness = value.toDouble();
                item_->redraw();
                return true;
            case ThermalModel::GapCount:
                par.count = value.toInt();
                item_->redraw();
                return true;
            }
        }
    }
    return false;
}

QVariant ThermalNode::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case ThermalModel::Name:
            return name;
        case ThermalModel::Position:
            return QString("%1 : %2").arg(pos_.X * dScale).arg(pos_.Y * dScale).replace('.', ',');
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
            if (childs.empty())
                return checked_ ? Qt::Checked : Qt::Unchecked;
            int val = 0;
            for (const auto& node : childs) {
                val |= node->checked_ ? 2 : 1;
            }
            return chState[val];
        } else
            return checked_ ? Qt::Checked : Qt::Unchecked;
    case Qt::TextAlignmentRole:
        if (index.column())
            return Qt::AlignCenter;
        return {};
    default:
        break;
    }
    return {};
}

Qt::ItemFlags ThermalNode::flags(const QModelIndex& index) const {
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

ThParam ThermalNode::getParam() const { return par; }

IntPoint ThermalNode::pos() const { return pos_; }

AbstractThermPrGi* ThermalNode::item() const { return item_; }

bool ThermalNode::createFile() const { return checked_ && item_; }

void ThermalNode::disable() {
    checked_ = false;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

void ThermalNode::enable() {
    checked_ = true;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

bool ThermalNode::isChecked() const { return checked_; }

QModelIndex ThermalNode::index(int column) const { return model->createIndex(row(), column, reinterpret_cast<quintptr>(this)); }

ThParam ThermalNode::getPar() const {
    return par;
}
