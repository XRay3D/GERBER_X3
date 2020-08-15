#pragma once



#include "abstractnode.h"
#include <QObject>

namespace Excellon {
class File;
}
class ExcellonDialog;

class ExcellonNode : public QObject, public AbstractNode {
    Q_OBJECT
    mutable ExcellonDialog* m_exFormatDialog = nullptr;

public:
    explicit ExcellonNode(int id);
    ~ExcellonNode() override = default;

    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu* menu, TreeView* tv) const override;
};


