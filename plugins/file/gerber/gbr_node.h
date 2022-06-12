/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "ft_node.h"

#include <QObject>

namespace Gerber {

class File;

class Node : public QObject,
             public FileTree::Node {
    friend class File;
    Q_OBJECT

    static QTimer m_decorationTimer;
    void repaint() const;
    Qt::CheckState m_current = Qt::Unchecked;
    File* file;

public:
    explicit Node(File* file, int* id);
    ~Node() override;

    // FileTree::Node interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) const override;
    static QTimer* decorationTimer();
};

} // namespace Gerber
