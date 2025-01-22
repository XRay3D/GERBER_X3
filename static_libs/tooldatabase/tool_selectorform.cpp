// This is a personal academic project. Dear PVS-Studio, please check it.
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
#include "tool_selectorform.h"
#include "settings.h"
#include "tool_database.h"
#include "tool_editdialog.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWidgets>
#include <app.h>

ToolSelectorForm::ToolSelectorForm(QWidget* parent)
    : QWidget{parent}
    , counter{static_cast<int>(parent->findChildren<ToolSelectorForm*>().count())}
    , toolFileName_{App::settingsPath() + '/' + parent->objectName() + QString::number(counter) + ".json"} {
    setupUi(this);
    readTool();
    label_->setStyleSheet(tool_.id() < 0 ? "QLabel { color: red }" : "");
}

ToolSelectorForm::~ToolSelectorForm() {
    writeTool();
}

void ToolSelectorForm::setTool(const Tool& tool) {
    tool_ = tool;
    label_->setStyleSheet(tool.id() < 0 ? "QLabel { color: red }" : "");
    updateForm();
}

const Tool& ToolSelectorForm::tool() const { return tool_; }

void ToolSelectorForm::on_pbSelect_clicked() {
    ToolDatabase tdb(this, {Tool::EndMill, Tool::Engraver, Tool::Laser});
    if(tdb.exec())
        setTool(tdb.tool());
}

void ToolSelectorForm::on_pbEdit_clicked() {
    ToolEditDialog d;
    d.setTool(tool_);
    if(d.exec())
        setTool(d.tool());
}

void ToolSelectorForm::updateForm() {
    lblPixmap->setPixmap(tool_.icon().pixmap({22, 22}));
    lblName->setText(tool_.name());
    setToolTip(tool_.note());
    emit updateName();
}

void ToolSelectorForm::readTool() {
    QFile file(toolFileName_);
    if(file.open(QIODevice::ReadOnly))
        tool_.read(QJsonDocument::fromJson(file.readAll()).object());
    else
        qWarning("Couldn't open tools file.");
    updateForm();
}

void ToolSelectorForm::writeTool() const {
    QFile file(toolFileName_);
    if(file.open(QIODevice::WriteOnly)) {
        QJsonObject json;
        tool_.write(json);
        file.write(QJsonDocument(json).toJson());
    } else {
        qWarning("Couldn't open tools file.");
    }
}

QLabel* ToolSelectorForm::label() const {
    return label_;
}

void ToolSelectorForm::setupUi(QWidget* ToolSelectorForm) {
    if(ToolSelectorForm->objectName().isEmpty())
        ToolSelectorForm->setObjectName(u"ToolSelectorForm"_s);
    ToolSelectorForm->resize(236, 180);

    auto gridLayout = new QGridLayout{ToolSelectorForm};
    gridLayout->setObjectName(u"gridLayout"_s);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    {
        label_ = new QLabel{ToolSelectorForm};
        label_->setObjectName(u"label_"_s);

        {
            QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(0);
            sizePolicy.setHeightForWidth(label_->sizePolicy().hasHeightForWidth());
            label_->setSizePolicy(sizePolicy);
        }

        QFontMetrics fm(font());
        label_->setMinimumWidth(std::max(
            fm.horizontalAdvance(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr)),
            fm.horizontalAdvance(QCoreApplication::translate("DepthForm", "Depth:", nullptr))));
        label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gridLayout->addWidget(label_, 0, 0, 1, 1);
    }

    {
        auto horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(u"horizontalLayout"_s);

        lblPixmap = new QLabel{ToolSelectorForm};
        lblPixmap->setObjectName(u"lblPixmap"_s);
        horizontalLayout->addWidget(lblPixmap);

        lblName = new QLabel{ToolSelectorForm};
        lblName->setObjectName(u"lblName"_s);
        horizontalLayout->addWidget(lblName);

        horizontalLayout->setStretch(1, 1);
        gridLayout->addLayout(horizontalLayout, 0, 1, 1, 2);
    }
    {
        pbSelect = new QPushButton{ToolSelectorForm};
        pbSelect->setObjectName(u"pbSelect"_s);
        pbSelect->setIcon(QIcon::fromTheme("view-form"));
        gridLayout->addWidget(pbSelect, 1, 1, 1, 1);
    }

    {
        pbEdit = new QPushButton{ToolSelectorForm};
        pbEdit->setObjectName(u"pbEdit"_s);
        pbEdit->setIcon(QIcon::fromTheme("document-edit"));
        gridLayout->addWidget(pbEdit, 1, 2, 1, 1);
    }

    retranslateUi(ToolSelectorForm);

    QMetaObject::connectSlotsByName(ToolSelectorForm);
}

void ToolSelectorForm::retranslateUi(QWidget* ToolSelectorForm) {
    ToolSelectorForm->setWindowTitle(QCoreApplication::translate("ToolSelectorForm", "Form", nullptr));
    if(counter > 1)
        label_->setText(QCoreApplication::translate("ToolSelectorForm", "Tool %1:", nullptr).arg(counter));
    else
        label_->setText(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr));
    pbEdit->setText(QCoreApplication::translate("ToolSelectorForm", "Edit", nullptr));
    pbSelect->setText(QCoreApplication::translate("ToolSelectorForm", "Select", nullptr));
}

#include "moc_tool_selectorform.cpp"
