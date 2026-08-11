/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "thermal_previewitem.h"

#include "thermal_vars.h"

#include <QIcon>
#include <QModelIndex>
#include <memory>

namespace Thermal {

class Model;
class Node;

class Node final {
public:
    explicit Node(const QIcon& icon, const QString& name, const ThParam& par, QPointF pos, AbstractThermPrGi* item, Model* model);
    explicit Node(const QIcon& icon, const QString& name, const ThParam& par, Model* model);
    explicit Node(Model* model);

    ~Node();

    Node* child(int row) const;

    Node* parentItem();

    int childCount() const;

    int row() const;

    void append(Node* item);
    void remove(int row);

    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    double angle() const;
    double tickness() const;
    int count() const;
    ThParam getParam() const;

    QPointF pos() const;
    AbstractThermPrGi* item() const;
    void disable();
    void enable();

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    bool isChecked() const;
    QModelIndex index(int column = 0) const;

    ThParam getPar() const;

private:
    const bool container{};
    const QIcon icon;
    const QString name;
    const QPointF pos_;

    ThParam par;

    AbstractThermPrGi* const item_;

    Node* parent_ = nullptr;
    std::vector<std::shared_ptr<Node>> childs;
    bool checked_{};

    Model* const model; // static wrong from anotherr dll
    static inline const Qt::CheckState chState[]{
        Qt::Unchecked,       // index 0
        Qt::Unchecked,       // index 1
        Qt::Checked,         // index 2
        Qt::PartiallyChecked // index 3
    };
};

} // namespace Thermal
