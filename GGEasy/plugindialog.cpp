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
 ********************************************************************************/
#include "plugindialog.h"
#include "app.h"
#include "file_plugin.h"
#include "ft_node.h"
#include "gc_plugin.h"
#include "shapepluginin.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QJsonObject>
#include <QTreeWidget>

DialogAboutPlugins::DialogAboutPlugins(QWidget* parent)
    : QDialog(parent) {
    setupUi(this);
    retranslateUi(this);

    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    treeWidget->setAlternatingRowColors(true);
    treeWidget->setIconSize({24, 24});

    auto addRows = [](QTreeWidgetItem* twItem, char c, auto map) {
        QFont boldFont = twItem->font(0);
        boldFont.setBold(true);
        twItem->setIcon(0, decoration(Qt::lightGray, c));
        twItem->setFont(0, boldFont);
        twItem->setExpanded(true);
        for (const auto& [type, ptr] : map) {
            auto json {ptr->info()};
            auto featureItem = new QTreeWidgetItem(twItem);
            featureItem->setExpanded(true);
            featureItem->setIcon(0, ptr->icon());
            featureItem->setText(0, json.value("Name").toString().remove('&'));
            featureItem->setText(1, json.value("Version").toString());
            featureItem->setText(2, json.value("VendorAuthor").toString());
            featureItem->setToolTip(0, json.value("Info").toString());
            featureItem->setToolTip(1, json.value("Info").toString());
            featureItem->setToolTip(2, json.value("Info").toString());
        }
    };

    if (App::filePlugins().size()) {
        auto interfaceItem = new QTreeWidgetItem(treeWidget, {tr("File Plugins"), "", ""});
        addRows(interfaceItem, 'F', App::filePlugins());
    }

    if (App::shapePlugins().size()) {
        auto interfaceItem = new QTreeWidgetItem(treeWidget, {tr("Shape Plugins"), "", ""});
        addRows(interfaceItem, 'S', App::shapePlugins());
    }

    if (App::gCodePlugins().size()) {
        auto interfaceItem = new QTreeWidgetItem(treeWidget, {tr("GCode Plugins"), "", ""});
        addRows(interfaceItem, 'G', App::gCodePlugins());
    }

    resize(600, 600);
}

DialogAboutPlugins::~DialogAboutPlugins() { }

void DialogAboutPlugins::setupUi(QDialog* Dialog) {
    if (Dialog->objectName().isEmpty())
        Dialog->setObjectName(QString::fromUtf8("Dialog"));
    Dialog->resize(400, 300);
    verticalLayout = new QVBoxLayout(Dialog);
    verticalLayout->setSpacing(6);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);

    treeWidget = new QTreeWidget(Dialog);
    treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
    verticalLayout->addWidget(treeWidget);

    buttonBox = new QDialogButtonBox(Dialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
    //    pbInfo = buttonBox->addButton("Info", QDialogButtonBox::HelpRole);
    pbClose = buttonBox->addButton("Close", QDialogButtonBox::AcceptRole);
    verticalLayout->addWidget(buttonBox);

    retranslateUi(Dialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

    QMetaObject::connectSlotsByName(Dialog);
}

void DialogAboutPlugins::retranslateUi(QDialog* Dialog) {
    Dialog->setWindowTitle(QApplication::translate("Dialog", "About Plugins...", nullptr));
    treeWidget->setColumnCount(3);
    treeWidget->setHeaderLabels({//
        QApplication::translate("Dialog", "Name", nullptr),
        QApplication::translate("Dialog", "Version", nullptr),
        QApplication::translate("Dialog", "Author", nullptr)});
}
