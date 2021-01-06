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
#include "gcplugin.h"

#include "gcfile.h"
#include "gcnode.h"
#include "interfaces/file.h"
#include "treeview.h"

#include <QMessageBox>
#include <QtWidgets>

namespace GCode {

Plugin::Plugin(QObject* parent)
    : QObject(parent)
{
}

bool Plugin::thisIsIt(const QString& /*fileName*/) { return false; }

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::GCode); }

NodeInterface* Plugin::createNode(FileInterface* file) { return new Node(file->id()); }

std::shared_ptr<FileInterface> Plugin::createFile() { return std::make_shared<File>(); }

void Plugin::setupInterface(App*) { }

void Plugin::createMainMenu(QMenu& menu, FileTreeView* tv)
{
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        tv, &FileTreeView::saveSelectedGCodeFiles);
}

std::pair<SettingsTabInterface*, QString> Plugin::createSettingsTab(QWidget* parent)
{
    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxInfo;
        QCheckBox* chbxSameGFolder;
        QLineEdit* leFileExtension;
        QLineEdit* leFormat;
        QLineEdit* leLaserCPC;
        QLineEdit* leLaserDPC;
        QLineEdit* leSpindleCC;
        QLineEdit* leSpindleLaserOff;
        QPlainTextEdit* pteEnd;
        QPlainTextEdit* pteLaserEnd;
        QPlainTextEdit* pteLaserStart;
        QPlainTextEdit* pteStart;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent)
        {
            setObjectName(QString::fromUtf8("tabGCode"));
            auto verticalLayout1 = new QVBoxLayout(this);
            verticalLayout1->setObjectName(QString::fromUtf8("verticalLayout1"));
            verticalLayout1->setContentsMargins(6, 6, 6, 6);

            chbxSameGFolder = new QCheckBox(this);
            chbxSameGFolder->setObjectName(QString::fromUtf8("chbxSameGFolder"));

            verticalLayout1->addWidget(chbxSameGFolder);

            chbxInfo = new QCheckBox(this);
            chbxInfo->setObjectName(QString::fromUtf8("chbxInfo"));

            verticalLayout1->addWidget(chbxInfo);

            auto label_14 = new QLabel(this);
            label_14->setObjectName(QString::fromUtf8("label_14"));

            verticalLayout1->addWidget(label_14);

            leFileExtension = new QLineEdit(this);
            leFileExtension->setObjectName(QString::fromUtf8("leFileExtension"));

            verticalLayout1->addWidget(leFileExtension);

            auto tabWidget_2 = new QTabWidget(this);
            tabWidget_2->setObjectName(QString::fromUtf8("tabWidget_2"));
            auto tabMilling = new QWidget();
            tabMilling->setObjectName(QString::fromUtf8("tabMilling"));
            auto verticalLayout_5 = new QVBoxLayout(tabMilling);
            verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
            verticalLayout_5->setContentsMargins(6, 6, 6, 6);
            auto label = new QLabel(tabMilling);
            label->setObjectName(QString::fromUtf8("label"));

            verticalLayout_5->addWidget(label);

            pteStart = new QPlainTextEdit(tabMilling);
            pteStart->setObjectName(QString::fromUtf8("pteStart"));

            verticalLayout_5->addWidget(pteStart);

            auto label_2 = new QLabel(tabMilling);
            label_2->setObjectName(QString::fromUtf8("label_2"));

            verticalLayout_5->addWidget(label_2);

            pteEnd = new QPlainTextEdit(tabMilling);
            pteEnd->setObjectName(QString::fromUtf8("pteEnd"));

            verticalLayout_5->addWidget(pteEnd);

            tabWidget_2->addTab(tabMilling, QString());
            auto tabLaser = new QWidget();
            tabLaser->setObjectName(QString::fromUtf8("tabLaser"));
            auto verticalLayout_6 = new QVBoxLayout(tabLaser);
            verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
            verticalLayout_6->setContentsMargins(6, 6, 6, 6);
            auto label_15 = new QLabel(tabLaser);
            label_15->setObjectName(QString::fromUtf8("label_15"));

            verticalLayout_6->addWidget(label_15);

            pteLaserStart = new QPlainTextEdit(tabLaser);
            pteLaserStart->setObjectName(QString::fromUtf8("pteLaserStart"));

            verticalLayout_6->addWidget(pteLaserStart);

            auto label_16 = new QLabel(tabLaser);
            label_16->setObjectName(QString::fromUtf8("label_16"));

            verticalLayout_6->addWidget(label_16);

            pteLaserEnd = new QPlainTextEdit(tabLaser);
            pteLaserEnd->setObjectName(QString::fromUtf8("pteLaserEnd"));

            verticalLayout_6->addWidget(pteLaserEnd);

            tabWidget_2->addTab(tabLaser, QString());

            verticalLayout1->addWidget(tabWidget_2);

            auto label_3 = new QLabel(this);
            label_3->setObjectName(QString::fromUtf8("label_3"));

            verticalLayout1->addWidget(label_3);

            leFormat = new QLineEdit(this);
            leFormat->setObjectName(QString::fromUtf8("leFormat"));

            verticalLayout1->addWidget(leFormat);

            auto groupBox_6 = new QGroupBox(this);
            groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
            auto formLayout_2 = new QFormLayout(groupBox_6);
            formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
            formLayout_2->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            formLayout_2->setContentsMargins(6, 9, 6, 6);
            auto label_10 = new QLabel(groupBox_6);
            label_10->setObjectName(QString::fromUtf8("label_10"));
            label_10->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

            formLayout_2->setWidget(0, QFormLayout::LabelRole, label_10);

            leSpindleCC = new QLineEdit(groupBox_6);
            leSpindleCC->setObjectName(QString::fromUtf8("leSpindleCC"));

            formLayout_2->setWidget(0, QFormLayout::FieldRole, leSpindleCC);

            auto label_11 = new QLabel(groupBox_6);
            label_11->setObjectName(QString::fromUtf8("label_11"));
            label_11->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

            formLayout_2->setWidget(1, QFormLayout::LabelRole, label_11);

            leLaserCPC = new QLineEdit(groupBox_6);
            leLaserCPC->setObjectName(QString::fromUtf8("leLaserCPC"));

            formLayout_2->setWidget(1, QFormLayout::FieldRole, leLaserCPC);

            auto label_12 = new QLabel(groupBox_6);
            label_12->setObjectName(QString::fromUtf8("label_12"));
            label_12->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

            formLayout_2->setWidget(2, QFormLayout::LabelRole, label_12);

            leLaserDPC = new QLineEdit(groupBox_6);
            leLaserDPC->setObjectName(QString::fromUtf8("leLaserDPC"));

            formLayout_2->setWidget(2, QFormLayout::FieldRole, leLaserDPC);

            auto label_13 = new QLabel(groupBox_6);
            label_13->setObjectName(QString::fromUtf8("label_13"));

            formLayout_2->setWidget(3, QFormLayout::LabelRole, label_13);

            leSpindleLaserOff = new QLineEdit(groupBox_6);
            leSpindleLaserOff->setObjectName(QString::fromUtf8("leSpindleLaserOff"));

            formLayout_2->setWidget(3, QFormLayout::FieldRole, leSpindleLaserOff);

            verticalLayout1->addWidget(groupBox_6);

            chbxInfo->setText(QApplication::translate("SettingsDialog", "Add a comment with the parameters G-\320\241ode", nullptr));
            groupBox_6->setTitle(QApplication::translate("SettingsDialog", "Spindle / Laser Control Code", nullptr));
            label->setText(QApplication::translate("SettingsDialog", "Start with:", nullptr));
            label_10->setText(QApplication::translate("SettingsDialog", "Spindle On:", nullptr));
            label_11->setText(QApplication::translate("SettingsDialog", "Constant Laser Power Mode On:", nullptr));
            label_12->setText(QApplication::translate("SettingsDialog", "Dynamic Laser Power Mode On:", nullptr));
            label_13->setText(QApplication::translate("SettingsDialog", "Spindle/Laser Off:", nullptr));
            label_14->setText(QApplication::translate("SettingsDialog", "File Extension:", nullptr));
            label_15->setText(QApplication::translate("SettingsDialog", "Start with:", nullptr));
            label_16->setText(QApplication::translate("SettingsDialog", "Finish with:", nullptr));
            label_2->setText(QApplication::translate("SettingsDialog", "Finish with:", nullptr));
            label_3->setText(QApplication::translate("SettingsDialog", "The format of the line with the coordinates:", nullptr));
            leFormat->setToolTip(QApplication::translate("SettingsDialog", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
            tabWidget_2->setTabText(tabWidget_2->indexOf(tabLaser), QApplication::translate("SettingsDialog", "Laser", nullptr));
            tabWidget_2->setTabText(tabWidget_2->indexOf(tabMilling), QApplication::translate("SettingsDialog", "Milling", nullptr));
        }
        virtual ~Tab() override { qDebug(__FUNCTION__); }

        virtual void readSettings(MySettings& settings) override
        {
            settings.beginGroup("GCode");
            m_info = settings.getValue(chbxInfo, m_info);
            m_sameFolder = settings.getValue(chbxSameGFolder, m_sameFolder);
            m_fileExtension = settings.getValue(leFileExtension, m_fileExtension);
            m_format = settings.getValue(leFormat, m_format);
            m_laserConstOn = settings.getValue(leLaserCPC, m_laserConstOn);
            m_laserDynamOn = settings.getValue(leLaserDPC, m_laserDynamOn);
            m_spindleOn = settings.getValue(leSpindleCC, m_spindleOn);
            m_spindleLaserOff = settings.getValue(leSpindleLaserOff, m_spindleLaserOff);

            m_end = settings.getValue(pteEnd, m_end);
            m_start = settings.getValue(pteStart, m_start);

            m_laserEnd = settings.getValue(pteLaserEnd, m_laserEnd);
            m_laserStart = settings.getValue(pteLaserStart, m_laserStart);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override
        {
            settings.beginGroup("GCode");
            m_fileExtension = settings.setValue(leFileExtension);
            m_format = settings.setValue(leFormat);
            m_info = settings.setValue(chbxInfo);
            m_laserConstOn = settings.setValue(leLaserCPC);
            m_laserDynamOn = settings.setValue(leLaserDPC);
            m_sameFolder = settings.setValue(chbxSameGFolder);
            m_spindleLaserOff = settings.setValue(leSpindleLaserOff);
            m_spindleOn = settings.setValue(leSpindleCC);

            m_start = settings.setValue(pteStart);
            m_end = settings.setValue(pteEnd);

            m_laserStart = settings.setValue(pteLaserStart);
            m_laserEnd = settings.setValue(pteLaserEnd);
            settings.endGroup();
        }
    };
    return { new Tab(parent), "G-Code" };
}

FileInterface* Plugin::parseFile(const QString& /*fileName*/, int /*type*/) { return nullptr; }

}
