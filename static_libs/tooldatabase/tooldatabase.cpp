// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "tooldatabase.h"
#include "toolitem.h"
#include "ui_tooldatabase.h"

#include <QKeyEvent>
#include <QMessageBox>

#include "leakdetector.h"

ToolDatabase::ToolDatabase(QWidget* parent, mvector<Tool::Type> types)
    : QDialog(parent)
    , ui(new Ui::ToolDatabase)
    , m_types(types)
{

    ui->setupUi(this);
    ui->treeView->setButtons({ ui->pbCopy, ui->pbDelete, ui->pbNew, ui->pbNewGroup });

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_types.empty());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToolDatabase::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToolDatabase::reject);

    connect(ui->toolEdit, &ToolEditForm::itemChanged, [this](ToolItem* item) {
        if (item->isTool())
            m_tool = item->tool();
        ui->treeView->updateItem();
    });

    connect(ui->treeView, &ToolTreeView::itemSelected, [this](ToolItem* item) {
        if (item->isTool())
            m_tool = item->tool();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled((item->isTool() && m_types.contains(item->tool().type())) || m_types.empty());
        ui->toolEdit->setItem(item);
    });

    connect(ui->treeView, &ToolTreeView::doubleClicked, [this](const QModelIndex& index) {
        ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
        if ((item->isTool() && m_types.contains(item->tool().type()))) {
            if (item->tool().isValid()) {
                m_tool = item->tool();
                accept();
            } else {
                QMessageBox ::information(this, tr("Invalid tool"), item->tool().errorStr());
            }
        }
    });

    ui->pbCopy->setIcon(QIcon::fromTheme("edit-copy"));
    ui->pbDelete->setIcon(QIcon::fromTheme("edit-delete"));
    ui->pbNew->setIcon(QIcon::fromTheme("list-add"));
    ui->pbNewGroup->setIcon(QIcon::fromTheme("folder-add"));

    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);
}

ToolDatabase::~ToolDatabase() { delete ui; }

Tool ToolDatabase::tool() const { return m_tool; }

void ToolDatabase::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return) {
        ui->toolEdit->on_pbApply_clicked();
        return;
    }
    QDialog::keyPressEvent(evt);
}
