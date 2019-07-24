#ifndef DrillItem__H
#define DrillItem__H

#include "abstractnode.h"
#include <QObject>

namespace Excellon {
class File;
}

class DrillNode : public QObject, public AbstractNode {
    Q_OBJECT

public:
    DrillNode(int id);
    ~DrillNode();

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // DrillItem__H
