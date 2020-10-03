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
