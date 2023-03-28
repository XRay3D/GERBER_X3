// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "tool_treeview.h"
#include "tool_item.h"

#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

ToolTreeView::ToolTreeView(QWidget* parent)
    : QTreeView(parent) {
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setAlternatingRowColors(true);
    setAnimated(true);

    model_ = new ToolModel(this);
    setModel(model_); // NOTE V1053. Calling the 'foo' virtual function in the constructor/destructor may lead to unexpected result at runtime.
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolTreeView::updateActions);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    setMinimumWidth(400);
    {
        setIconSize(QSize(24, 24));
        const int w = indentation();
        const QModelIndex& row = model()->index(0, 0, QModelIndex());
        if (row.isValid()) {
            const int h = rowHeight(row);
            QImage i(w, h, QImage::Format_ARGB32);
            QPainter p(&i);
            p.setPen(QColor(128, 128, 128));
            // │
            i.fill(Qt::transparent);
            p.drawLine(w >> 1, /**/ 0, w >> 1, /**/ h);
            i.save("settings/vline.png", "PNG");
            // ├─
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("settings/branch-more.png", "PNG");
            // └─
            i.fill(Qt::transparent);
            p.drawLine(w >> 1, /**/ 0, w >> 1, h >> 1);
            p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
            i.save("settings/branch-end.png", "PNG");
            QFile file(":/qtreeviewstylesheet/QTreeView.qss");
            file.open(QFile::ReadOnly);
            setStyleSheet(file.readAll());
            header()->setMinimumHeight(h);
        }
    }
}

void ToolTreeView::newGroup() {
    QModelIndex index = selectionModel()->currentIndex();
    if (index.data(Qt::UserRole).toInt())
        index = index.parent();
    if (!model_->insertRows(0, 1, index))
        return;
    index = model_->index(0, 0, index);
    model_->setData(index, tr("New Group"), Qt::EditRole);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    updateActions();
}

void ToolTreeView::newTool() {
    QModelIndex index = selectionModel()->currentIndex();
    if (index.data(Qt::UserRole).toInt())
        index = index.parent();
    if (!model_->insertRows(index.data(Qt::UserRole + 1).toInt(), 1, index))
        return;

    ToolItem* item = nullptr;

    if (!index.isValid())
        item = static_cast<ToolItem*>(model_->index(0, 0, index).internalPointer());
    else
        item = static_cast<ToolItem*>(index.internalPointer())->lastChild();

    if (item) {
        item->setIsTool();
        item->tool().setName(tr("New Tool ") + QString::number(item->toolId()));
        index = model_->createIndex(item->row(), 0, item);
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    updateActions();
}

void ToolTreeView::deleteItem() {
    if (QMessageBox::question(this, tr("Warning"), tr("Are you sure you want to delete the item and all content?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        return;
    QModelIndex index = selectionModel()->currentIndex();
    if (model_->removeRows(index.row(), 1, index.parent()))
        updateActions();
}

void ToolTreeView::copyTool() {
    QModelIndex index = selectionModel()->currentIndex();
    ToolItem* itemSrc = static_cast<ToolItem*>(index.internalPointer());
    if (!model_->insertRows(index.row() + 1, 1, index.parent()))
        return;

    index = index.sibling(index.row() + 1, 0);

    ToolItem* itemDst = static_cast<ToolItem*>(index.internalPointer());
    itemDst->setIsTool();
    itemDst->tool() = itemSrc->tool();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    updateActions();
}

void ToolTreeView::updateActions() {
    QModelIndex index = selectionModel()->currentIndex();
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
    buttons_[Delete]->setEnabled(!selectionModel()->selection().isEmpty());
    if (item) {
        buttons_[Copy]->setEnabled(item->isTool());
        emit itemSelected(item);
    } else
        buttons_[Copy]->setEnabled(false);
    expandAll();
}

void ToolTreeView::setButtons(const mvector<QPushButton*>& buttons) {
    buttons_ = buttons;
    connect(buttons_[Copy], &QPushButton::clicked, this, &ToolTreeView::copyTool);
    connect(buttons_[Delete], &QPushButton::clicked, this, &ToolTreeView::deleteItem);
    connect(buttons_[New], &QPushButton::clicked, this, &ToolTreeView::newTool);
    connect(buttons_[NewGroup], &QPushButton::clicked, this, &ToolTreeView::newGroup);
    updateActions();
}

void ToolTreeView::updateItem() {
    resizeColumnToContents(0);
    for (QModelIndex index : selectionModel()->selection().indexes()) {
        update(index);
    }
}

#include "moc_tool_treeview.cpp"
