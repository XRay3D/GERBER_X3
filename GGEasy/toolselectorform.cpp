// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "toolselectorform.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWidgets>

#include "tooldatabase.h"
#include "tooleditdialog.h"

#include "leakdetector.h"

ToolSelectorForm::ToolSelectorForm(QWidget* parent)
    : QWidget(parent)
    , counter { parent->findChildren<ToolSelectorForm*>().count() }
    , m_toolFileName {
        qApp->applicationDirPath()
        + "/settings/"
        + parent->objectName() + QString::number(counter)
        + ".dat"
    }
{
    setupUi(this);


    readTool();
}

ToolSelectorForm::~ToolSelectorForm()
{
    writeTool();
}

void ToolSelectorForm::setTool(const Tool& tool)
{
    m_tool = tool;
    updateForm();
}

const Tool& ToolSelectorForm::tool() const { return m_tool; }

void ToolSelectorForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraver, Tool::Laser });
    if (tdb.exec()) {
        setTool(tdb.tool());
    }
}

void ToolSelectorForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.setTool(m_tool);
    if (d.exec()) {
        setTool(d.tool());
        m_tool.setId(-1);
    }
}

void ToolSelectorForm::updateForm()
{
    lblPixmap->setPixmap(m_tool.icon().pixmap({ 22, 22 }));
    lblName->setText(m_tool.name());
    setToolTip(m_tool.note());
    emit updateName();
}

void ToolSelectorForm::readTool()
{
    QFile file(m_toolFileName);
    if (file.open(QIODevice::ReadOnly)) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QJsonDocument loadDoc(QJsonDocument::fromBinaryData(file.readAll()));
#else
        QJsonDocument loadDoc(QJsonDocument::fromJson(file.readAll()));
#endif
        QJsonObject json = loadDoc.object();
        m_tool.read(json);
    } else {
        qWarning("Couldn't open tools file.");
    }
    updateForm();
}

void ToolSelectorForm::writeTool() const
{
    QFile file(m_toolFileName);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject json;
        m_tool.write(json);
        QJsonDocument saveDoc(json);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        file.write(saveDoc.toBinaryData());
#else
        file.write(saveDoc.toJson());
#endif
    } else {
        qWarning("Couldn't open tools file.");
    }
}

QLabel* ToolSelectorForm::label() const
{
    return m_label;
}

void ToolSelectorForm::setupUi(QWidget* ToolSelectorForm)
{
    if (ToolSelectorForm->objectName().isEmpty())
        ToolSelectorForm->setObjectName(QString::fromUtf8("ToolSelectorForm"));
    ToolSelectorForm->resize(236, 180);

    auto gridLayout = new QGridLayout(ToolSelectorForm);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    {
        m_label = new QLabel(ToolSelectorForm);
        m_label->setObjectName(QString::fromUtf8("label"));

        {
            QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(0);
            sizePolicy.setHeightForWidth(m_label->sizePolicy().hasHeightForWidth());
            m_label->setSizePolicy(sizePolicy);
        }

        QFont font;
        font.setBold(true);
        font.setWeight(75);
        m_label->setFont(font);

        QFontMetrics fm(font);

        m_label->setMinimumWidth(std::max(
            fm.horizontalAdvance(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr)),
            fm.horizontalAdvance(QCoreApplication::translate("DepthForm", "Depth:", nullptr))));
        m_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        gridLayout->addWidget(m_label, 0, 0, 1, 1);
    }

    {
        auto horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        lblPixmap = new QLabel(ToolSelectorForm);
        lblPixmap->setObjectName(QString::fromUtf8("lblPixmap"));
        horizontalLayout->addWidget(lblPixmap);

        lblName = new QLabel(ToolSelectorForm);
        lblName->setObjectName(QString::fromUtf8("lblName"));
        horizontalLayout->addWidget(lblName);

        horizontalLayout->setStretch(1, 1);
        gridLayout->addLayout(horizontalLayout, 0, 1, 1, 2);
    }
    {
        pbSelect = new QPushButton(ToolSelectorForm);
        pbSelect->setObjectName(QString::fromUtf8("pbSelect"));
        pbSelect->setIcon(QIcon::fromTheme("view-form"));
        gridLayout->addWidget(pbSelect, 1, 1, 1, 1);
    }

    {
        pbEdit = new QPushButton(ToolSelectorForm);
        pbEdit->setObjectName(QString::fromUtf8("pbEdit"));
        pbEdit->setIcon(QIcon::fromTheme("document-edit"));
        gridLayout->addWidget(pbEdit, 1, 2, 1, 1);
    }

    retranslateUi(ToolSelectorForm);

    QMetaObject::connectSlotsByName(ToolSelectorForm);
}

void ToolSelectorForm::retranslateUi(QWidget* ToolSelectorForm)
{
    ToolSelectorForm->setWindowTitle(QCoreApplication::translate("ToolSelectorForm", "Form", nullptr));
    if (counter > 1)
        m_label->setText(QCoreApplication::translate("ToolSelectorForm", "Tool %1:", nullptr).arg(counter));
    else
        m_label->setText(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr));
    pbEdit->setText(QCoreApplication::translate("ToolSelectorForm", "Edit", nullptr));
    pbSelect->setText(QCoreApplication::translate("ToolSelectorForm", "Select", nullptr));
}
