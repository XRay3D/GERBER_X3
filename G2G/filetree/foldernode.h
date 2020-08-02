#pragma once
//#ifndef FOLDER_H
//#define FOLDER_H

#include "abstractnode.h"

class FolderNode : public AbstractNode {
    QString name;
    Qt::CheckState m_checkState = Qt::Checked;

public:
    explicit FolderNode(const QString& name);
    ~FolderNode() override = default;

    // AbstractNode interface
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    void menu(QMenu* menu , TreeView * tv) const override;
};

//#endif // FOLDER_H
