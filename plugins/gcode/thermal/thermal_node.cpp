// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_node.h"
#include "thermal_model.h"

namespace Thermal {

Node::Node(const QIcon& icon, const QString& name, const ThParam& par, const IntPoint& pos, AbstractThermPrGi* item, Model* model)
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

Node::Node(const QIcon& icon, const QString& name, const ThParam& par, Model* model)
    : container(true)
    , icon(icon)
    , name(name)
    , par(par)
    , item_(nullptr)
    , model(model) {
}

Node::Node(Model* model)
    : item_(nullptr)
    , model(model) {
}

Node::~Node() { childs.clear(); }

Node* Node::child(int row) const { return childs.at(row).get(); }

Node* Node::parentItem() { return parent_; }

int Node::childCount() const { return static_cast<int>(childs.size()); }

int Node::row() const {
    if (parent_)
        for (size_t i = 0, size = parent_->childs.size(); i < size; ++i)
            if (parent_->childs[i].get() == this)
                return static_cast<int>(i);
    return 0;
}

void Node::append(Node* item) {
    item->parent_ = this;
    childs.emplace_back(std::shared_ptr<Node>(item));
}

void Node::remove(int row) { childs.remove(row); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
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
            case Model::Name:
                //            case Model::Position:
                return false;
            case Model::GapAngle:
                par.angle = value.toDouble();
                for (auto node : qAsConst(childItems))
                    node->setData(index, value, role);
                return true;
            case Model::apThickness:
                par.tickness = value.toDouble();
                for (auto node : qAsConst(childItems))
                    node->setData(index, value, role);
                return true;
            case Model::GapCount:
                par.count = value.toInt();
                for (auto node : qAsConst(childItems))
                    node->setData(index, value, role);
                return true;
            }
            return false;
        } else {
            switch (index.column()) {
            case Model::Name:
                //            case Model::Position:
                //                return false;
            case Model::GapAngle:
                par.angle = value.toDouble();
                item_->redraw();
                return true;
            case Model::apThickness:
                par.tickness = value.toDouble();
                item_->redraw();
                return true;
            case Model::GapCount:
                par.count = value.toInt();
                item_->redraw();
                return true;
            }
        }
    }
    return false;
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case Model::Name:
            return name.size() ? name : pos_.toString();
            //        case Model::Position:
            //            return QVariant::fromValue(pos_); // QString("%1 : %2").arg(pos_.X * dScale).arg(pos_.Y * dScale).replace('.', ',');
        case Model::GapAngle:
            return par.angle;
        case Model::apThickness:
            return par.tickness;
        case Model::GapCount:
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

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!index.column())
        flags |= Qt::ItemIsUserCheckable;
    if (index.column() > Model::/*Position*/ Name)
        flags |= Qt::ItemIsEditable;
    if (!container)
        flags |= Qt::ItemNeverHasChildren;
    flags |= Qt::ItemIsSelectable;
    return flags;
}

double Node::angle() const { return par.angle; }

double Node::tickness() const { return par.tickness; }

int Node::count() const { return par.count; }

ThParam Node::getParam() const { return par; }

IntPoint Node::pos() const { return pos_; }

AbstractThermPrGi* Node::item() const { return item_; }

bool Node::createFile() const { return checked_ && item_; }

void Node::disable() {
    checked_ = false;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

void Node::enable() {
    checked_ = true;
    model->dataChanged(index(), index(), {});
    model->dataChanged(parentItem()->index(), parentItem()->index(), {});
}

bool Node::isChecked() const { return checked_; }

QModelIndex Node::index(int column) const { return model->createIndex(row(), column, reinterpret_cast<quintptr>(this)); }

ThParam Node::getPar() const {
    return par;
}

} // namespace Thermal
