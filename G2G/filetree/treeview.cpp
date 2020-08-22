// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "treeview.h"
#include "abstractnode.h"
#include "forms/drillform/drillform.h"
#include "gbrnode.h"
#include "layerdelegate.h"
#include "project.h"
#include "settings.h"
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QtWidgets>

#include "../mainwindow.h"
#include "gcode.h"
#include <gcfile.h>

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
    , m_model(new FileModel(this))
{
    setModel(m_model);
    setAlternatingRowColors(true);
    setAnimated(true);
    setUniformRowHeights(true);

    connect(GerberNode::repaintTimer(), &QTimer::timeout, this, &TreeView::updateIcons);
    connect(m_model, &FileModel::rowsInserted, this, &TreeView::updateTree);
    connect(m_model, &FileModel::rowsRemoved, this, &TreeView::updateTree);
    connect(m_model, &FileModel::updateActions, this, &TreeView::updateTree);
    connect(m_model, &FileModel::select, [this](const QModelIndex& index) {
        selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    });
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::onSelectionChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::updateTree);
    connect(this, &TreeView::doubleClicked, this, &TreeView::on_doubleClicked);

    setIconSize(QSize(22, 22));

    {
        int w = indentation();
        int h = rowHeight(m_model->index(0, 0, QModelIndex()));
        QImage i(w, h, QImage::Format_ARGB32);
        i.fill(Qt::transparent);
        for (int y = 0; y < h; ++y)
            i.setPixelColor(w / 2, y, QColor(128, 128, 128));
        i.save("vline.png", "PNG");

        for (int x = w / 2; x < w; ++x)
            i.setPixelColor(x, h / 2, QColor(128, 128, 128));
        i.save("branch-more.png", "PNG");

        i.fill(Qt::transparent);
        for (int y = 0; y < h / 2; ++y)
            i.setPixelColor(w / 2, y, QColor(128, 128, 128));
        for (int x = w / 2; x < w; ++x)
            i.setPixelColor(x, h / 2, QColor(128, 128, 128));
        i.save("branch-end.png", "PNG");

        QFile file(":/qtreeviewstylesheet/QTreeView.qss");
        file.open(QFile::ReadOnly);
        setStyleSheet(file.readAll());
        header()->setMinimumHeight(h);
    }

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setStretchLastSection(false);

    setItemDelegateForColumn(1, new LayerDelegate(this));
    setItemDelegateForColumn(2, new RadioDelegate(this));
}

void TreeView::updateTree()
{
    expandAll();
}

void TreeView::updateIcons()
{
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractNode*>(index.internalPointer())->childCount();
    for (int r = 0; r < rowCount; ++r)
        update(m_model->index(r, 0, index));
}

void TreeView::on_doubleClicked(const QModelIndex& index)
{
    if (!index.column()) {
        m_menuIndex = index;
        if (index.parent() == m_model->index(NodeGerberFiles, 0, QModelIndex())) {
            hideOther();
        } else if (index.parent() == m_model->index(NodeDrillFiles, 0, QModelIndex())) {
            hideOther();
        } else if (index.parent() == m_model->index(NodeToolPath, 0, QModelIndex())) {
            qDebug(Q_FUNC_INFO);
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

void TreeView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
#ifndef QT_DEBUG
    if (!selected.indexes().isEmpty() && selected.indexes().first().isValid()) {
        QModelIndex& index = selected.indexes().first();
        const int row = index.parent().row();
        if (row == NodeGerberFiles || row == NodeDrillFiles || row == NodeToolPath) {
            const int id = index.data(Qt::UserRole).toInt();
            AbstractFile* file = App::project()->file(id);
            file->itemGroup()->setZValue(id);
        }
        if (row == NodeSpecial) {
            const int id = index.data(Qt::UserRole).toInt();
            App::project()->aShape(id)->setSelected(true);
        }
    }
    if (!deselected.indexes().isEmpty()) {
        QModelIndex& index = deselected.indexes().first();
        const int row = index.parent().row();
        if (row == NodeGerberFiles || row == NodeDrillFiles || row == NodeToolPath) {
            const int id = index.data(Qt::UserRole).toInt();
            AbstractFile* file = App::project()->file(id);
            file->itemGroup()->setZValue(-id);
        }
        if (row == NodeSpecial) {
            const int id = index.data(Qt::UserRole).toInt();
            App::project()->aShape(id)->setSelected(false);
        }
    }
#endif
}

void TreeView::hideOther()
{
    const int rowCount = static_cast<AbstractNode*>(m_menuIndex.parent().internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index2 = m_menuIndex.sibling(row, 0);
        auto* item = static_cast<AbstractNode*>(index2.internalPointer());
        if (row == m_menuIndex.row())
            item->setData(index2, Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(index2, Qt::Unchecked, Qt::CheckStateRole);
    }
    m_model->dataChanged(m_menuIndex.sibling(0, 0), m_menuIndex.sibling(rowCount, 0));
}

void TreeView::closeFile()
{
    m_model->removeRow(m_menuIndex.row(), m_menuIndex.parent());
    if (App::drillForm())
        App::drillForm()->on_pbClose_clicked();
}

void TreeView::saveGcodeFile()
{
    auto* file = App::project()->file<GCode::File>(m_menuIndex.data(Qt::UserRole).toInt());
    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        GCode::GCUtils::getLastDir().append(m_menuIndex.data().toString()),
        tr("GCode (*.%1)").arg(GlobalSettings::gcFileExtension())));

    if (name.isEmpty())
        return;

    file->save(name);
}

void TreeView::showExcellonDialog() { }

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    m_menuIndex = indexAt(event->pos());

    switch (m_menuIndex.parent().row()) {
    case -1:
        if (m_menuIndex.row() == NodeToolPath && static_cast<AbstractNode*>(m_menuIndex.internalPointer())->childCount()) {
            menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [this] {
                if (QMessageBox::question(this, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                    m_model->removeRows(0, static_cast<AbstractNode*>(m_menuIndex.internalPointer())->childCount(), m_menuIndex);
            });
            menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), [] { App::project()->saveSelectedToolpaths(); });
        }
        break;
    default:
        reinterpret_cast<AbstractNode*>(m_menuIndex.internalId())->menu(&menu, this);
        break;
    }

    if (!menu.isEmpty())
        menu.exec(mapToGlobal(event->pos()));
}
