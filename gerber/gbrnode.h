/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "abstractnode.h"
#include <QObject>
#include "gbrfile.h"

namespace Gerber {
class Node : public QObject, public AbstractNode {
    Q_OBJECT

public:
    explicit Node(int id);
    ~Node() override;

public:
    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu* menu, TreeView* tv) const override;

    static QTimer* repaintTimer();

private:
    static QTimer m_repaintTimer;
    void repaint();
    Qt::CheckState m_current = Qt::Unchecked;
};
}
