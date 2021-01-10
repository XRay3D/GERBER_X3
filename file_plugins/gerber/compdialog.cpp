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
#include "compdialog.h"
#include "ui_compdialog.h"

#include "leakdetector.h"

namespace Gerber {

ComponentsDialog::ComponentsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ComponentsDialog)
{
    ui->setupUi(this);
}

ComponentsDialog::~ComponentsDialog()
{
    delete ui;
}

void ComponentsDialog::setFile(int fileId)
{
    ui->componentsView->setFile(fileId);
}

}
