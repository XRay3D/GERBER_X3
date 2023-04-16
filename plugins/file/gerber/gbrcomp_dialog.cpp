// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbrcomp_dialog.h"
#include "gbrcomp_view.h"

#include <QSettings>
#include <QtWidgets>

namespace Gerber::Comp {

Dialog::Dialog(QWidget* parent)
    : QDialog(parent) {
    setupUi(this);
    graphicsView->setScene(scene_ = new QGraphicsScene(graphicsView));
    graphicsView->scale(+1, -1);
    QSettings settings;
    settings.beginGroup("Dialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    splitter->restoreState(settings.value("splitter").toByteArray());
    componentsView->header()->restoreState(settings.value("header").toByteArray());
    graphicsView->setBackgroundBrush(Qt::black);
    connect(splitter, &QSplitter::splitterMoved, [this] { resizeEvent(); });
}

Dialog::~Dialog() {
    QSettings settings;
    settings.beginGroup("Dialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("splitter", splitter->saveState());
    settings.setValue("header", componentsView->header()->saveState());
}

void Dialog::setFile(int fileId) { componentsView->setFile(fileId); }

void Dialog::setupUi(QDialog* dialog) {
    if(dialog->objectName().isEmpty())
        dialog->setObjectName(QString::fromUtf8("Dialog"));
    dialog->resize(800, 600);

    splitter = new QSplitter(dialog);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);

    componentsView = new sView(splitter);
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
    connect(buttonBox, &QDialogButtonBox ::accepted, dialog, &QDialog ::accept);
    connect(buttonBox, &QDialogButtonBox ::rejected, dialog, &QDialog ::reject);
    QMetaObject::connectSlotsByName(dialog);
}

void Dialog::retranslateUi(QDialog* dialog) {
    dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", nullptr));
}

void Dialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    graphicsView->fitInView(graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void Dialog::resizeEvent(QResizeEvent* event) {
    if(event)
        QDialog::resizeEvent(event);
    graphicsView->fitInView(graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

} // namespace Gerber::Comp

#include "moc_gbrcomp_dialog.cpp"
