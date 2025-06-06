/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "tool_database.h"
#include "tool_item.h"
#include "ui_tooldatabase.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>

ToolDatabase::ToolDatabase(QWidget* parent, mvector<Tool::Type> types)
    : QDialog{parent}
    , ui(new Ui::ToolDatabase)
    , types_(types) {

    ui->setupUi(this);
    ui->treeView->setButtons({ui->pbCopy, ui->pbDelete, ui->pbNew, ui->pbNewGroup});

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(types_.empty());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToolDatabase::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToolDatabase::reject);

    connect(ui->toolEdit, &ToolEditForm::itemChanged, [this](ToolItem* item) {
        if(item->isTool())
            tool_ = item->tool();
        ui->treeView->updateItem();
    });

    connect(ui->treeView, &ToolTreeView::itemSelected, [this](ToolItem* item) {
        if(item->isTool())
            tool_ = item->tool();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled((item->isTool() && types_.contains(item->tool().type())) || types_.empty());
        ui->toolEdit->setItem(item);
    });

    connect(ui->treeView, &ToolTreeView::doubleClicked, [this](const QModelIndex& index) {
        ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
        if((item->isTool() && types_.contains(item->tool().type()))) {
            if(item->tool().isValid()) {
                tool_ = item->tool();
                accept();
            } else {
                QMessageBox::information(this, tr("Invalid tool"), item->tool().errorStr());
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

ToolDatabase::~ToolDatabase() {
    QSettings settings;
    settings.beginGroup("ToolDatabase");
    settings.setValue("geometry", saveGeometry());
    // qWarning() << geometry();
    delete ui;
}

Tool ToolDatabase::tool() const { return tool_; }

void ToolDatabase::keyPressEvent(QKeyEvent* evt) {
    if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return) {
        ui->toolEdit->on_pbApply_clicked();
        return;
    }
    QDialog::keyPressEvent(evt);
}

void ToolDatabase::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QSettings settings;
    settings.beginGroup("ToolDatabase");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    // qWarning() << geometry();
}

#include "moc_tool_database.cpp"
