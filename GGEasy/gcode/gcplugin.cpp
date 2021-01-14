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

FileInterface* Plugin::createFile() { return new File(); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "GCode" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "GCode is a static plugin always included with GGEasy." }
    };
}

void Plugin::createMainMenu(QMenu& menu, FileTreeView* tv)
{
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        tv, &FileTreeView::saveSelectedGCodeFiles);
}

SettingsTab Plugin::createSettingsTab(QWidget* parent)
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
            auto /**/ verticalLayout1 = new QVBoxLayout(this);
            /**/ verticalLayout1->setObjectName(QString::fromUtf8("/**/verticalLayout1"));
            /**/ verticalLayout1->setContentsMargins(6, 6, 6, 6);
            // chbxSameGFolder
            chbxSameGFolder = new QCheckBox(this);
            chbxSameGFolder->setObjectName(QString::fromUtf8("chbxSameGFolder"));
            chbxSameGFolder->setText(QApplication::translate("SettingsDialog", "Save the G-Code to the project folder.", nullptr));
            /**/ verticalLayout1->addWidget(chbxSameGFolder);

            // chbxInfo
            chbxInfo = new QCheckBox(this);
            chbxInfo->setObjectName(QString::fromUtf8("chbxInfo"));
            chbxInfo->setText(QApplication::translate("SettingsDialog", "Add a comment with the parameters G-\320\241ode", nullptr));
            /**/ verticalLayout1->addWidget(chbxInfo);

            // File Extension
            auto lblFileExtension = new QLabel(this);
            lblFileExtension->setObjectName(QString::fromUtf8("lblFileExtension"));
            lblFileExtension->setText(QApplication::translate("SettingsDialog", "File Extension:", nullptr));
            /**/ verticalLayout1->addWidget(lblFileExtension);
            leFileExtension = new QLineEdit(this);
            leFileExtension->setObjectName(QString::fromUtf8("leFileExtension"));
            /**/ verticalLayout1->addWidget(leFileExtension);

            auto tabwStart = new QTabWidget(this);
            tabwStart->setObjectName(QString::fromUtf8("tabwStart"));
            { // Tab Milling

                auto tabMilling = new QWidget();
                tabMilling->setObjectName(QString::fromUtf8("tabMilling"));
                auto verticalLayoutM = new QVBoxLayout(tabMilling);
                verticalLayoutM->setObjectName(QString::fromUtf8("verticalLayoutM"));
                verticalLayoutM->setContentsMargins(6, 6, 6, 6);

                auto lblStartM = new QLabel(tabMilling);
                lblStartM->setObjectName(QString::fromUtf8("lblStartM"));
                lblStartM->setText(QApplication::translate("SettingsDialog", "Start with:", nullptr));
                verticalLayoutM->addWidget(lblStartM);
                pteStart = new QPlainTextEdit(tabMilling);
                pteStart->setObjectName(QString::fromUtf8("pteStart"));
                verticalLayoutM->addWidget(pteStart);

                auto lblEndM = new QLabel(tabMilling);
                lblEndM->setObjectName(QString::fromUtf8("lblEndM"));
                lblEndM->setText(QApplication::translate("SettingsDialog", "Finish with:", nullptr));
                verticalLayoutM->addWidget(lblEndM);
                pteEnd = new QPlainTextEdit(tabMilling);
                pteEnd->setObjectName(QString::fromUtf8("pteEnd"));
                verticalLayoutM->addWidget(pteEnd);

                tabwStart->addTab(tabMilling, QString());
                tabwStart->setTabText(tabwStart->indexOf(tabMilling), QApplication::translate("SettingsDialog", "Milling", nullptr));
            }
            { // Tab Laser
                auto tabLaser = new QWidget();
                tabLaser->setObjectName(QString::fromUtf8("tabLaser"));
                auto verticalLayoutL = new QVBoxLayout(tabLaser);
                verticalLayoutL->setObjectName(QString::fromUtf8("verticalLayoutL"));
                verticalLayoutL->setContentsMargins(6, 6, 6, 6);

                auto lblStartL = new QLabel(tabLaser);
                lblStartL->setObjectName(QString::fromUtf8("lblStartL"));
                lblStartL->setText(QApplication::translate("SettingsDialog", "Start with:", nullptr));
                verticalLayoutL->addWidget(lblStartL);
                pteLaserStart = new QPlainTextEdit(tabLaser);
                pteLaserStart->setObjectName(QString::fromUtf8("pteLaserStart"));
                verticalLayoutL->addWidget(pteLaserStart);

                auto lblEndL = new QLabel(tabLaser);
                lblEndL->setObjectName(QString::fromUtf8("lblStartL"));
                lblEndL->setText(QApplication::translate("SettingsDialog", "Finish with:", nullptr));
                verticalLayoutL->addWidget(lblEndL);
                pteLaserEnd = new QPlainTextEdit(tabLaser);
                pteLaserEnd->setObjectName(QString::fromUtf8("pteLaserEnd"));
                verticalLayoutL->addWidget(pteLaserEnd);

                tabwStart->addTab(tabLaser, QString());
                tabwStart->setTabText(tabwStart->indexOf(tabLaser), QApplication::translate("SettingsDialog", "Laser", nullptr));
            }
            /**/ verticalLayout1->addWidget(tabwStart);

            auto lblFormat = new QLabel(this);
            lblFormat->setObjectName(QString::fromUtf8("lblFormat"));
            lblFormat->setText(QApplication::translate("SettingsDialog", "The format of the line with the coordinates:", nullptr));
            /**/ verticalLayout1->addWidget(lblFormat);
            leFormat = new QLineEdit(this);
            leFormat->setObjectName(QString::fromUtf8("leFormat"));
            leFormat->setToolTip(QApplication::translate("SettingsDialog", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
            /**/ verticalLayout1->addWidget(leFormat);

            auto grbxSpindle = new QGroupBox(this);
            grbxSpindle->setObjectName(QString::fromUtf8("grbxSpindle"));
            grbxSpindle->setTitle(QApplication::translate("SettingsDialog", "Spindle / Laser Control Code", nullptr));
            auto formLayoutgrbxSpindle = new QFormLayout(grbxSpindle);
            formLayoutgrbxSpindle->setObjectName(QString::fromUtf8("formLayoutgrbxSpindle"));
            formLayoutgrbxSpindle->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            formLayoutgrbxSpindle->setContentsMargins(6, 9, 6, 6);

            auto lblSpindleCC = new QLabel(grbxSpindle);
            lblSpindleCC->setObjectName(QString::fromUtf8("lblSpindleCC"));
            lblSpindleCC->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            lblSpindleCC->setText(QApplication::translate("SettingsDialog", "Spindle On:", nullptr));
            formLayoutgrbxSpindle->setWidget(0, QFormLayout::LabelRole, lblSpindleCC);
            leSpindleCC = new QLineEdit(grbxSpindle);
            leSpindleCC->setObjectName(QString::fromUtf8("leSpindleCC"));
            formLayoutgrbxSpindle->setWidget(0, QFormLayout::FieldRole, leSpindleCC);

            auto lblLaserCPC = new QLabel(grbxSpindle);
            lblLaserCPC->setObjectName(QString::fromUtf8("lblLaserCPC"));
            lblLaserCPC->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            lblLaserCPC->setText(QApplication::translate("SettingsDialog", "Constant Laser Power Mode On:", nullptr));
            formLayoutgrbxSpindle->setWidget(1, QFormLayout::LabelRole, lblLaserCPC);
            leLaserCPC = new QLineEdit(grbxSpindle);
            leLaserCPC->setObjectName(QString::fromUtf8("leLaserCPC"));
            formLayoutgrbxSpindle->setWidget(1, QFormLayout::FieldRole, leLaserCPC);

            auto lblLaserDPC = new QLabel(grbxSpindle);
            lblLaserDPC->setObjectName(QString::fromUtf8("lblLaserDPC"));
            lblLaserDPC->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            lblLaserDPC->setText(QApplication::translate("SettingsDialog", "Dynamic Laser Power Mode On:", nullptr));
            formLayoutgrbxSpindle->setWidget(2, QFormLayout::LabelRole, lblLaserDPC);
            leLaserDPC = new QLineEdit(grbxSpindle);
            leLaserDPC->setObjectName(QString::fromUtf8("leLaserDPC"));
            formLayoutgrbxSpindle->setWidget(2, QFormLayout::FieldRole, leLaserDPC);

            auto lblSpindleLaserOff = new QLabel(grbxSpindle);
            lblSpindleLaserOff->setObjectName(QString::fromUtf8("lblSpindleLaserOff"));
            lblSpindleLaserOff->setText(QApplication::translate("SettingsDialog", "Spindle/Laser Off:", nullptr));
            formLayoutgrbxSpindle->setWidget(3, QFormLayout::LabelRole, lblSpindleLaserOff);
            leSpindleLaserOff = new QLineEdit(grbxSpindle);
            leSpindleLaserOff->setObjectName(QString::fromUtf8("leSpindleLaserOff"));
            formLayoutgrbxSpindle->setWidget(3, QFormLayout::FieldRole, leSpindleLaserOff);

            /**/ verticalLayout1->addWidget(grbxSpindle);
        }
        virtual ~Tab() override { }
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
