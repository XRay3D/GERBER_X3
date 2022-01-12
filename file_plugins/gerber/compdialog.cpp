// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "compdialog.h"
#include "compview.h"

#include "app.h"
#include "gbrfile.h"
#include "project.h"
#include <QDebug>
#include <QSettings>
#include <QtWidgets>
#include <scene.h>

namespace Gerber {

ComponentsDialog::ComponentsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    graphicsView->setScene(m_scene = new QGraphicsScene(graphicsView));
    graphicsView->scale(+1, -1);
    QSettings settings;
    settings.beginGroup("ComponentsDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    splitter->restoreState(settings.value("splitter").toByteArray());
    componentsView->header()->restoreState(settings.value("header").toByteArray());
    graphicsView->setBackgroundBrush(Qt::black);
    connect(splitter, &QSplitter::splitterMoved, [this] { resizeEvent(); });
}

ComponentsDialog::~ComponentsDialog()
{
    QSettings settings;
    settings.beginGroup("ComponentsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", splitter->saveState());
    settings.setValue("header", componentsView->header()->saveState());
}

void ComponentsDialog::setFile(int fileId) { componentsView->setFile(fileId); }

void ComponentsDialog::setupUi(QDialog* dialog)
{
    if (dialog->objectName().isEmpty())
        dialog->setObjectName(QString::fromUtf8("ComponentsDialog"));
    dialog->resize(800, 600);

    splitter = new QSplitter(dialog);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);

    componentsView = new ComponentsView(splitter);
    componentsView->setObjectName(QString::fromUtf8("componentsView"));

    graphicsView = new QGraphicsView(splitter);
    graphicsView->setObjectName(QString::fromUtf8("graphicsView"));

    splitter->addWidget(componentsView);
    splitter->addWidget(graphicsView);

    auto buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    auto verticalLayout = new QVBoxLayout(dialog);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);
    verticalLayout->addWidget(splitter);
    verticalLayout->addWidget(buttonBox);

    retranslateUi(dialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(dialog);
}

void ComponentsDialog::retranslateUi(QDialog* dialog)
{
    dialog->setWindowTitle(QApplication::translate("ComponentsDialog", "Dialog", nullptr));
}

void ComponentsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    graphicsView->fitInView(graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void ComponentsDialog::resizeEvent(QResizeEvent* event)
{
    if (event)
        QDialog::resizeEvent(event);
    graphicsView->fitInView(graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

}
