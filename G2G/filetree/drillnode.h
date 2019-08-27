#ifndef DRILL_H
#define DRILL_H

#include "abstractnode.h"
#include <QObject>

namespace Excellon {
class File;
}

class DrillNode : public QObject, public AbstractNode {
    Q_OBJECT

public:
    explicit DrillNode(int id);
    ~DrillNode() override = default;

    // AbstractItem interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // DRILL_H
