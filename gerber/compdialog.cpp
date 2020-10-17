/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
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
