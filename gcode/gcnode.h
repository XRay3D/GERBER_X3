#pragma once
#ifndef MILLING_H
#define MILLING_H

#include "abstractnode.h"
#include <QGraphicsItemGroup>
#include <gcfile.h>

class GcodeNode : public AbstractNode {

public:
    explicit GcodeNode(int id);
    ~GcodeNode() override = default;

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    GCode::File* const m_file;
};

#endif // MILLING_H
