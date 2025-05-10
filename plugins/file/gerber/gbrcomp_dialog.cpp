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
#include "utils.h"

#include <QSettings>
#include <QtWidgets>

namespace Gerber::Comp {

Dialog::Dialog(QWidget* parent)
    : QDialog{parent} {
    setupUi(this);
    grView->setScene(scene_ = new QGraphicsScene{grView});
    grView->scale(+1, -1);
    QSettings settings;
    settings.beginGroup("Dialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    splitter->restoreState(settings.value("splitter").toByteArray());
    componentsView->header()->restoreState(settings.value("header").toByteArray());
    grView->setBackgroundBrush(Qt::black);
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
        dialog->setObjectName(u"dialog"_s);
    dialog->resize(800, 600);

    splitter = new QSplitter{dialog};
    splitter->setObjectName(u"splitter"_s);
    splitter->setOrientation(Qt::Horizontal);

    componentsView = new sView{splitter};
    componentsView->setObjectName(u"componentsView"_s);

    grView = new QGraphicsView{splitter};
    grView->setObjectName(u"grView"_s);

    splitter->addWidget(componentsView);
    splitter->addWidget(grView);

    auto buttonBox = new QDialogButtonBox{dialog};
    buttonBox->setObjectName(u"buttonBox"_s);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    auto verticalLayout = new QVBoxLayout{dialog};
    verticalLayout->setObjectName(u"verticalLayout"_s);
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
    grView->fitInView(grView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void Dialog::resizeEvent(QResizeEvent* event) {
    if(event)
        QDialog::resizeEvent(event);
    grView->fitInView(grView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

} // namespace Gerber::Comp

#include "moc_gbrcomp_dialog.cpp"
