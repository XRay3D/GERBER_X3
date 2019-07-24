#ifndef FILE_H
#define FILE_H

#include "abstractnode.h"
#include <QObject>
#include <gbrfile.h>

class GerberNode : public QObject, public AbstractNode {
    Q_OBJECT

public:
    GerberNode(int id);
    ~GerberNode();
    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QTimer* repaintTimer();

private:

    static QTimer m_repaintTimer;
    void repaint();
    Qt::CheckState m_current = Qt::Unchecked;
};

#endif // FILE_H
