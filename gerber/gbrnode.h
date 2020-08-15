#pragma once
//#ifndef GBRFILE_H
//#define GBRFILE_H

#include "abstractnode.h"
#include <QObject>
#include <gbrfile.h>

class GerberNode : public QObject, public AbstractNode {
    Q_OBJECT

public:
    explicit GerberNode(int id);
    ~GerberNode() override;

public:
    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu* menu , TreeView * tv) const override;

    static QTimer* repaintTimer();

private:
    static QTimer m_repaintTimer;
    void repaint();
    Qt::CheckState m_current = Qt::Unchecked;
};

//#endif // GBRFILE_H
