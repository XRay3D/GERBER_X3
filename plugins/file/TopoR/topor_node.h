/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "ft_node.h"

namespace TopoR {

class Layer;
class File;

class Node : public FileTree::Node {
    friend class File;
    File* file;

public:
    explicit Node(File* file);
    ~Node() override;

    // FileTree::Node interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) override;
    int32_t id() const override;
};

class NodeLayer : public FileTree::Node {
public:
    // Публичные: Node::menu обращается к ним из лямбды меню (закрытие лямбды
    // -- отдельный класс, не «часть» Node для нужд friend, даже когда лямбда
    // лексически лежит внутри метода Node).
    const QString name;
    Layer* const layer;

    explicit NodeLayer(const QString& name, Layer* layer);
    ~NodeLayer() override = default;

    // FileTree::Node interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) override;
    int32_t id() const override;
};

} // namespace TopoR
