// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "pathgroupnode.h"

PathGroupNode::PathGroupNode()
    : FileTree::Node{FileTree::PathGroup} {
}

bool PathGroupNode::setData(const QModelIndex& index, const QVariant& value, int role) {
    return {};
}

Qt::ItemFlags PathGroupNode::flags(const QModelIndex& index) const {
    return {};
}

QVariant PathGroupNode::data(const QModelIndex& index, int role) const {
    return {};
}

void PathGroupNode::menu(QMenu& menu, FileTree::View* tv) {
}
