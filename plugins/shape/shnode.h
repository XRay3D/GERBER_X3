/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "ft_node.h"

namespace Shapes {

class AbstractShape;

class Node : public FileTree::Node {
    friend class AbstractShape;
    AbstractShape* shape;

public:
    explicit Node(AbstractShape* shape);
    ~Node() override = default;

    // AbstractNode interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void menu(QMenu& menu, FileTree::View* tv) const override;
    int id() const override;
};

} // namespace Shapes
