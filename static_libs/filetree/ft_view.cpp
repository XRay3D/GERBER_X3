// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "ft_view.h"

#include "ft_node.h"
#include "ft_radiodelegate.h"
#include "ft_sidedelegate.h"
#include "ft_textdelegate.h"
#include "ft_typedelegate.h"

#include "file_plugin.h"
#include "fileifce.h"
#include "qcheckbox.h"
#include "shapepluginin.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <doublespinbox.h>

namespace FileTree {

View::View(QWidget* parent)
    : QTreeView(parent) {
    setAlternatingRowColors(true);
    setAnimated(true);
    setUniformRowHeights(true);

    App::setFileTreeView(this);
}

View::~View() {
    App::setFileTreeView(nullptr);
}

void View::updateTree() {
    expandAll();
}

void View::updateIcons() {
    QModelIndex index = model_->index(0, 0, QModelIndex());
    int rowCount = static_cast<Node*>(index.internalPointer())->childCount();
    for (int r = 0; r < rowCount; ++r)
        update(model_->index(r, 0, index));
}

void View::on_doubleClicked(const QModelIndex& index) {
    if (!index.column()) {
        menuIndex_ = index;
        if (index.data(Role::NodeType).toInt() != Type::Folder)
            hideOther();
        //        if (index.parent() == model_->index(Model::GerberFiles, 0, QModelIndex())) {
        //            hideOther();
        //        } else if (index.parent() == model_->index(Model::DrillFiles, 0, QModelIndex())) {
        //            hideOther();
        //        } else if (index.parent() == model_->index(Model::ToolPath, 0, QModelIndex())) {
        //            hideOther();
        //            {
        //                const int id = menuIndex_.data(Qt::UserRole).toInt();
        //                FileInterface* file = static_cast<FileInterface*>(App::project()->file(id));
        //                App::project()->showFiles(file->gcp_.params[GCode::GCodeParams::GrItems].value<UsedItems>().keys());
        //                file->gcp_.fileId = file->id();
        //                App::mainWindow()->editGcFile(file);
        //                updateTree();
        //            }
        //    }
    }
}

void View::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    if (!selected.indexes().isEmpty()) {
        for (auto& index : selected.indexes())
            model_->setData(index, true, Role::Select);
    }
    if (!deselected.indexes().isEmpty()) {
        for (auto& index : deselected.indexes())
            model_->setData(index, false, Role::Select);
    }
}

void View::hideOther() {
    const int rowCount = static_cast<Node*>(menuIndex_.parent().internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index2 = menuIndex_.sibling(row, 0);
        auto* item = static_cast<Node*>(index2.internalPointer());
        if (row == menuIndex_.row())
            item->setData(index2, Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(index2, Qt::Unchecked, Qt::CheckStateRole);
    }
    QTimer::singleShot(100, [this, rowCount] { emit model_->dataChanged(
                                                   menuIndex_.sibling(0, 0),
                                                   menuIndex_.sibling(rowCount - 1, 0),
                                                   {Qt::CheckStateRole}); });
}

void View::closeFile() {
    model_->removeRow(menuIndex_.row(), menuIndex_.parent());
    //    if (App::drillForm())
    //        App::drillForm().on_pbClose_clicked();
}

void View::closeFiles() {
    model_->removeRows(0, childCount_, menuIndex_);
}

void View::setModel(QAbstractItemModel* model) {
    QTreeView::setModel(model_ = static_cast<Model*>(model));

    connect(model_, &Model::rowsInserted, this, &View::updateTree);
    connect(model_, &Model::rowsRemoved, this, &View::updateTree);
    connect(model_, &Model::updateActions, this, &View::updateTree);
    connect(model_, &Model::select, [this](const QModelIndex& index) {
        selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    });

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &View::onSelectionChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &View::updateTree);
    connect(this, &View::doubleClicked, this, &View::on_doubleClicked);

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

void View::showExcellonDialog() { }

void View::contextMenuEvent(QContextMenuEvent* event) {
    menuIndex_ = indexAt(event->pos());
    qDebug() << menuIndex_;
    if (!menuIndex_.isValid())
        return;
    QMenu menu(this);
    childCount_ = static_cast<Node*>(menuIndex_.internalPointer())->childCount();
    if (menuIndex_.data(Role::NodeType).toInt() == Type::Folder) {
        const int type = menuIndex_.data(Role::Id).toInt();
        const int cType = menuIndex_.data(Role::ContentType).toInt();
        if (cType == Type::File) {
            App::filePlugin(type)->createMainMenu(menu, this);
        } else if (cType == Type::Shape) {
            App::shapePlugins().begin()->second->createMainMenu(menu, this);
        }
    } else {
        reinterpret_cast<Node*>(menuIndex_.internalId())->menu(menu, this);
        if (auto selectedRows {selectionModel()->selectedRows().toVector()}; selectedRows.count() > 1) {
            menu.addSeparator();
            // FIXME rename Action in future.
            menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete Selected"), [selectedRows, this]() mutable {
                std::ranges::sort(selectedRows, std::greater {}, &QModelIndex::row);
                for (auto&& index : selectedRows)
                    model_->removeRow(index.row(), index.parent());
            });
        }
        {
            auto selectedRows {selectionModel()->selectedRows().toVector()};
            if (selectedRows.empty())
                selectedRows.push_back(menuIndex_);
            auto file = App::project()->file(selectedRows.front().data(FileTree::Id).toInt());
            if (file) {
                menu.addSeparator();
                menu.addAction(QIcon::fromTheme(""), tr("Transform"), [selectedRows, this]() mutable {
                    QDialog dialog(this);
                    dialog.setMaximumSize(0, 0);
                    dialog.setWindowTitle(tr("Transform"));
                    QLabel la("Angle:", &dialog);
                    QLabel lx("Translate X:", &dialog);
                    QLabel ly("Translate Y:", &dialog);
                    QLabel lsx("Scale X:", &dialog);
                    QLabel lsy("Scale Y:", &dialog);
                    DoubleSpinBox dsbxAng(&dialog);
                    DoubleSpinBox dsbxTrX(&dialog);
                    DoubleSpinBox dsbxTrY(&dialog);
                    DoubleSpinBox dsbxScX(&dialog);
                    DoubleSpinBox dsbxScY(&dialog);
                    dsbxAng.setRange(-360, +360);
                    dsbxAng.setSuffix(tr(" °"));
                    dsbxScX.setRange(-10, +10);
                    dsbxScY.setRange(-10, +10);
                    dsbxTrX.setRange(-1000, +1000);
                    dsbxTrX.setSuffix(tr(" mm"));
                    dsbxTrY.setRange(-1000, +1000);
                    dsbxTrY.setSuffix(tr(" mm"));
                    QFormLayout layout(&dialog);
                    layout.addRow(&la, &dsbxAng);
                    layout.addRow(&lx, &dsbxTrX);
                    layout.addRow(&ly, &dsbxTrY);
                    layout.addRow(&lsx, &dsbxScX);
                    layout.addRow(&lsy, &dsbxScY);
                    layout.setLabelAlignment(Qt::AlignRight);

                    // QPushButton button(tr("Apply"), &d);
                    // layout.addRow(new QWidget(&d), &button);
                    dialog.resize({0, 0});

                    auto file = App::project()->file(selectedRows.front().data(FileTree::Id).toInt());

                    auto transform = file->transform();

                    dsbxAng.setValue(transform.angle);
                    dsbxTrX.setValue(transform.translate.x());
                    dsbxTrY.setValue(transform.translate.y());
                    dsbxScX.setValue(transform.scale.x());
                    dsbxScY.setValue(transform.scale.y());

                    auto setTransform = [&] {
                        transform = {
                            .angle = dsbxAng.value(),
                            .translate {dsbxTrX.value(), dsbxTrY.value()},
                            .scale {dsbxScX.value(), dsbxScY.value()},
                        };

                        for (auto&& index : selectedRows) {
                            auto file = App::project()->file(index.data(FileTree::Id).toInt());
                            if (file)
                                file->setTransform(transform);
                        }
                    };

                    connect(&dsbxAng, &QDoubleSpinBox::valueChanged, setTransform);
                    connect(&dsbxTrX, &QDoubleSpinBox::valueChanged, setTransform);
                    connect(&dsbxTrY, &QDoubleSpinBox::valueChanged, setTransform);
                    connect(&dsbxScX, &QDoubleSpinBox::valueChanged, setTransform);
                    connect(&dsbxScY, &QDoubleSpinBox::valueChanged, setTransform);

                    // connect(&button, &QPushButton::clicked, &d, &QDialog::accept);
                    // connect(&d, &QDialog::rejected, [&] { dsbxA.setValue(0), dsbxX.setValue(0), dsbxY.setValue(0); });

                    dialog.exec();
                });
            }
        }
    }

    if (!menu.isEmpty())
        menu.exec(viewport()->mapToGlobal(event->pos()));
}

void View::mousePressEvent(QMouseEvent* event) {
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

void View::mouseDoubleClickEvent(QMouseEvent* event) {
    menuIndex_ = indexAt(event->pos());
    if (menuIndex_.isValid() && menuIndex_.parent().row() > -1)
        hideOther();
    else
        QTreeView::mouseDoubleClickEvent(event);
}

} // namespace FileTree

#include "moc_ft_view.cpp"
