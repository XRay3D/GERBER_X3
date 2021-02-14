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

#include "app.h"
#include "gbrfile.h"
#include "project.h"
#include <QDebug>
#include <QSettings>
#include <scene.h>

#include "leakdetector.h"

namespace Gerber {

ComponentsDialog::ComponentsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ComponentsDialog)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene(ui->graphicsView));
    QSettings settings; //("settings/ComponentsDialog.ini", QSettings::IniFormat);
    settings.beginGroup("ComponentsDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    ui->componentsView->header()->restoreState(settings.value("header").toByteArray());
    ui->graphicsView->setBackgroundBrush(Qt::black);
}

ComponentsDialog::~ComponentsDialog()
{
    QSettings settings; //("settings/ComponentsDialog.ini", QSettings::IniFormat);
    settings.beginGroup("ComponentsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.setValue("header", ui->componentsView->header()->saveState());
    for (auto item : ui->graphicsView->scene()->items())
        App::scene()->addItem(item);
    //ui->graphicsView->scene()->removeItem(item);
    delete ui;
}

void ComponentsDialog::setFile(int fileId)
{
    auto file = App::project()->file<File>(fileId);
    file->itemGroup(File::Components)->addToScene(ui->graphicsView->scene());
    ui->componentsView->setFile(fileId);
}

}
