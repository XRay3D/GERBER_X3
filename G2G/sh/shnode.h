#pragma once

#include "abstractnode.h"
#include <QGraphicsItemGroup>
#include <gcfile.h>

class ShNode : public AbstractNode {

public:
    explicit ShNode(int id);
    ~ShNode() override = default;

    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    // AbstractNode interface
    void menu(QMenu* menu, TreeView* tv) const override;
};
