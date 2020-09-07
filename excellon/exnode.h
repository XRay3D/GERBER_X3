#pragma once

#include "abstractnode.h"

class ExcellonDialog;

namespace Excellon {
class File;

class Node : public AbstractNode {
    mutable ExcellonDialog* m_exFormatDialog = nullptr;

public:
    explicit Node(int id);
    ~Node() override = default;

    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu* menu, TreeView* tv) const override;
};
}
