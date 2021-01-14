// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "treeview.h"
//#include "forms/drillform/drillform.h"
#ifdef GBR_
#include "gbrnode.h"
#include "shheaders.h"
#endif

#include "settings.h"

//#include "interfaces/file.h"
#include "interfaces/node.h"
#include "mainwindow.h"
#include "radiodelegate.h"
#include "sidedelegate.h"
#include "textdelegate.h"
#include "typedelegate.h"
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include "leakdetector.h"

FileTreeView::FileTreeView(QWidget* parent)
    : QTreeView(parent)
{
    setAlternatingRowColors(true);
    setAnimated(true);
    setUniformRowHeights(true);

    App::setFileTreeView(this);
}

FileTreeView::~FileTreeView()
{
    App::setFileTreeView(nullptr);
}

void FileTreeView::updateTree()
{
    expandAll();
}

void FileTreeView::updateIcons()
{
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<NodeInterface*>(index.internalPointer())->childCount();
    for (int r = 0; r < rowCount; ++r)
        update(m_model->index(r, 0, index));
}

void FileTreeView::on_doubleClicked(const QModelIndex& index)
{
    if (!index.column()) {
        m_menuIndex = index;
        if (index.parent() == m_model->index(FileModel::GerberFiles, 0, QModelIndex())) {
            hideOther();
        } else if (index.parent() == m_model->index(FileModel::DrillFiles, 0, QModelIndex())) {
            hideOther();
        } else if (index.parent() == m_model->index(FileModel::ToolPath, 0, QModelIndex())) {
            hideOther();
            //            {
            //                const int id = m_menuIndex.data(Qt::UserRole).toInt();
            //                GCode::File* file = static_cast<GCode::File*>(App::project()->file(id));
            //                App::project()->showFiles(file->m_gcp.params[GCode::GCodeParams::GrItems].value<UsedItems>().keys());
            //                file->m_gcp.fileId = file->id();
            //                App::mainWindow()->editGcFile(file);
            //                updateTree();
            //            }
        }
    }
}

void FileTreeView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (!selected.indexes().isEmpty()) {
        auto index = selected.indexes().first();
        const int row = index.parent().row();
        switch (row) {
        case FileModel::GerberFiles:
        case FileModel::DrillFiles:
        case FileModel::ToolPath:
            //            const int id = index.data(Qt::UserRole).toInt();
            //            FileInterface* file = App::project()->file(id);
            //            file->itemGroup()->setZValue(id);
            m_model->setData(index, true, NodeInterface::SelectRole);
            break;
        case FileModel::Shapes:
            for (auto& index : selected.indexes())
                m_model->setData(index, true, NodeInterface::SelectRole);
            break;
        }
    }
    if (!deselected.indexes().isEmpty()) {
        auto index = deselected.indexes().first();
        const int row = index.parent().row();
        switch (row) {
        case FileModel::GerberFiles:
        case FileModel::DrillFiles:
        case FileModel::ToolPath:
            //            const int id = index.data(Qt::UserRole).toInt();
            //            FileInterface* file = App::project()->file(id);
            //            file->itemGroup()->setZValue(-id);
            m_model->setData(index, false, NodeInterface::SelectRole);
            break;
        case FileModel::Shapes:
            for (auto& index : selected.indexes())
                m_model->setData(index, false, NodeInterface::SelectRole);
            break;
        }
    }
}

void FileTreeView::hideOther()
{
    const int rowCount = static_cast<NodeInterface*>(m_menuIndex.parent().internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index2 = m_menuIndex.sibling(row, 0);
        auto* item = static_cast<NodeInterface*>(index2.internalPointer());
        if (row == m_menuIndex.row())
            item->setData(index2, Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(index2, Qt::Unchecked, Qt::CheckStateRole);
    }
    emit m_model->dataChanged(m_menuIndex.sibling(0, 0), m_menuIndex.sibling(rowCount, 0));
}

void FileTreeView::closeFile()
{
    m_model->removeRow(m_menuIndex.row(), m_menuIndex.parent());
    //    if (App::drillForm())
    //        App::drillForm()->on_pbClose_clicked();
}

void FileTreeView::closeFiles()
{
    m_model->removeRows(0, m_childCount, m_menuIndex);
}

void FileTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(m_model = static_cast<FileModel*>(model));

    connect(m_model, &FileModel::rowsInserted, this, &FileTreeView::updateTree);
    connect(m_model, &FileModel::rowsRemoved, this, &FileTreeView::updateTree);
    connect(m_model, &FileModel::updateActions, this, &FileTreeView::updateTree);
    //    connect(m_model, &FileModel::select, [this](const QModelIndex& index) {
    //        selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    //    });
    //#ifndef QT_DEBUG
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileTreeView::onSelectionChanged);
    //#endif
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileTreeView::updateTree);
    connect(this, &FileTreeView::doubleClicked, this, &FileTreeView::on_doubleClicked);

    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::Fixed);
    header()->setDefaultSectionSize(QFontMetrics(font()).size(Qt::TextSingleLine, "123456789").width()); // ~6 символов и ...
    header()->setSectionResizeMode(0, QHeaderView::Stretch);

    setItemDelegateForColumn(0, new TextDelegate(this));
    setItemDelegateForColumn(1, new SideDelegate(this));
    setItemDelegateForColumn(2, new TypeDelegate(this));

    {
        setIconSize(QSize(24, 24));
        const int w = indentation();
        const int h = rowHeight(model->index(0, 0, QModelIndex()));
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

void FileTreeView::showExcellonDialog() { }

void FileTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    m_menuIndex = indexAt(event->pos());
    qDebug() << __FUNCTION__ << m_menuIndex;
    if (!m_menuIndex.isValid())
        return;
    QMenu menu(this);
    m_childCount = static_cast<NodeInterface*>(m_menuIndex.internalPointer())->childCount();
    const int type = m_menuIndex.data(Qt::UserRole + 0).toInt();
    const bool isFolder = m_menuIndex.data(Qt::UserRole + 1).toInt() == -1;
    if (isFolder && m_childCount && App::fileInterfaces().contains(type)) {
        App::fileInterface(type)->createMainMenu(menu, this);
    } else {
        reinterpret_cast<NodeInterface*>(m_menuIndex.internalId())->menu(menu, this);
    }
    //    switch (m_menuIndex.parent().row()) {
    //    case -1:
    //        if (m_menuIndex.row() == FileModel::ToolPath && m_childCount) {
    //            menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [m_childCount, this] {
    //                if (QMessageBox::question(this, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    //                    m_model->removeRows(0, m_childCount, m_menuIndex);
    //            });
    //            menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), [] { App::project()->saveSelectedToolpaths(); });
    //        } else if (m_menuIndex.row() == FileModel::Shapes && m_childCount) {
    //            menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Objects"), [m_childCount, this] {
    //                if (QMessageBox::question(this, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    //                    m_model->removeRows(0, m_childCount, m_menuIndex);
    //            });
    //            //            // edit Shapes::Text
    //            //            mvector<Shapes::Text*> tx;
    //            //            for (auto& idx : selectedIndexes()) {
    //            //                if (auto sh = App::project()->aShape(idx.data(Qt::UserRole).toInt()); sh->type() == int(GiType::ShText))
    //            //                    tx.push_back(static_cast<Shapes::Text*>(sh));
    //            //            }
    //            //            if (tx.size())
    //            //                menu.addAction(QIcon::fromTheme("draw-text"), tr("&Edit Selected Texts"), [tx] {
    //            //                    ShTextDialog dlg(tx, App::mainWindow());
    //            //                    dlg.exec();
    //            //                });
    //        }
    //        break;
    //    default:
    //        break;
    //    }

    if (!menu.isEmpty())
        menu.exec(mapToGlobal(event->pos() + QPoint(0, menu.actionGeometry(menu.actions().first()).height())));
}

void FileTreeView::mousePressEvent(QMouseEvent* event)
{
    QTreeView::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            if (index.column() == 0) {
                if (event->pos().x() > visualRect(index).left() + 44)
                    edit(index);
            } else
                edit(index);
        }
    }
}

void FileTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    m_menuIndex = indexAt(event->pos());
    if (m_menuIndex.isValid() && m_menuIndex.parent().row() > -1)
        hideOther();
    else
        QTreeView::mouseDoubleClickEvent(event);
}
