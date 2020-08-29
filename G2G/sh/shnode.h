#pragma once

#include "abstractnode.h"
namespace Shapes {
class Node : public AbstractNode {

public:
    explicit Node(int id);
    ~Node() override = default;

    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    // AbstractNode interface
    void menu(QMenu* menu, TreeView* tv) const override;
};
}
