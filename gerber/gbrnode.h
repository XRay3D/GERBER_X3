/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "gbrfile.h"

#include "interfaces/node.h"

#include <QObject>

namespace Gerber {
class Node : public QObject, public NodeInterface {
    Q_OBJECT

public:
    explicit Node(int id);
    ~Node() override;

public:
    // NodeInterface interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTreeView* tv) const override;

    static QTimer* decorationTimer();

private:
    static QTimer m_decorationTimer;
    void repaint();
    Qt::CheckState m_current = Qt::Unchecked;
};
}
