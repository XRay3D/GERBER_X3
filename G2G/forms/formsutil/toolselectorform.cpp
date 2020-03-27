#include "toolselectorform.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWidgets>

#include <tooldatabase/tooldatabase.h>
#include <tooldatabase/tooleditdialog.h>

ToolSelectorForm::ToolSelectorForm(QWidget* parent)
    : QWidget(parent)
    , m_parentName(parent->objectName())
{
    setupUi(this);
    m_toolFileName = qApp->applicationDirPath()
        + "/XrSoft/"
        //+ QString("%1").arg(qHash(m_parentName + QString::number(parent->children().indexOf(this))), sizeof(uint) * 2, 16, QChar('0')).toUpper()
        + m_parentName + QString::number(parent->children().indexOf(this))
        + ".dat";
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
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving, Tool::Laser });
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
        QJsonDocument loadDoc(QJsonDocument::fromBinaryData(file.readAll()));
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
        file.write(saveDoc.toBinaryData());
    } else {
        qWarning("Couldn't open tools file.");
    }
}

QLabel *ToolSelectorForm::label() const
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
            QFont font;
            font.setBold(true);
            font.setWeight(75);
            m_label->setFont(font);
        }

        {
            QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(0);
            sizePolicy.setHeightForWidth(m_label->sizePolicy().hasHeightForWidth());
            m_label->setSizePolicy(sizePolicy);
        }

        m_label->setMidLineWidth(std::max(
            QFontMetrics(font()).horizontalAdvance(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr)),
            QFontMetrics(font()).horizontalAdvance(QCoreApplication::translate("DepthForm", "Depth:  ", nullptr))));
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
    m_label->setText(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr));
    pbEdit->setText(QCoreApplication::translate("ToolSelectorForm", "Edit", nullptr));
    pbSelect->setText(QCoreApplication::translate("ToolSelectorForm", "Select", nullptr));
}
