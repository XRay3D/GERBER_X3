/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "tool_editdialog.h"

#include "ui_tooleditdialog.h"
#include <QSettings>

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog{parent}
    , ui(new Ui::ToolEditDialog) {
    ui->setupUi(this);
    ui->toolEdit->setDialog();
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this] {
        if(ui->toolEdit->tool_.isValid()) {
            ui->toolEdit->tool_.setId(Tool::ID::Null);
            accept();
        } else {
            ui->toolEdit->on_pbApply_clicked();
            // toolEdit->tool_.errorMessageBox(this);
        }
    });
}

ToolEditDialog::~ToolEditDialog() {
    QSettings settings;
    settings.beginGroup(u"ToolEditDialog"_s);
    settings.setValue(u"geometry"_s, saveGeometry());
    // qWarning() << geometry();
}

Tool ToolEditDialog::tool() const { return ui->toolEdit->tool_; }

void ToolEditDialog::setTool(const Tool& tool) { ui->toolEdit->setTool(tool); }

void ToolEditDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QSettings settings;
    settings.beginGroup(u"ToolEditDialog"_s);
    restoreGeometry(settings.value(u"geometry"_s, QByteArray()).toByteArray());
    // qWarning() << geometry();
}

#include "moc_tool_editdialog.cpp"

#include <QSettings>
