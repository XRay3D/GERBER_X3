/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_fileplugin.h"
#include "file.h"
#include "ft_view.h"
#include "gc_file.h"
#include "gc_node.h"

#include <QMessageBox>
#include <QtWidgets>

namespace GCode {

Plugin::Plugin(QObject* parent)
    : FilePlugin(parent) {
    info_ = {
        { "Name", "GCode" },
        { "Version", "1.1" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "GCode is a static plugin always included with GGEasy." }
    };
}

bool Plugin::thisIsIt(const QString& /*fileName*/) { return false; }

int Plugin::type() const { return int(FileType::GCode); }

QString Plugin::folderName() const { return tr("Tool Paths"); }

FileInterface* Plugin::createFile() { return new File(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'G'); }

void Plugin::createMainMenu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete All Toolpaths"), [tv] {
        if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            tv->closeFiles();
    });
    menu.addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        tv, &FileTree::View::saveSelectedGCodeFiles);
}

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent) {

    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxInfo;
        QCheckBox* chbxSameGFolder;
        QCheckBox* chbxSimplifyHldi;
        QComboBox* cbxProfileSort;
        QLineEdit* leFileExtension;
        QLineEdit* leFormatMilling;
        QLineEdit* leFormatLaser;
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
            : SettingsTabInterface(parent) {
            setObjectName(QString::fromUtf8("tabGCode"));
            qDebug() << this;
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

            auto tabwIndividualSettings = new QTabWidget(this);
            tabwIndividualSettings->setObjectName(QString::fromUtf8("tabwIndividualSettings"));
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

                auto lblFormatM = new QLabel(tabMilling);
                lblFormatM->setObjectName(QString::fromUtf8("lblFormatM"));
                lblFormatM->setText(QApplication::translate("SettingsDialog", "The format of the line with the coordinates:", nullptr));
                verticalLayoutM->addWidget(lblFormatM);
                leFormatMilling = new QLineEdit(tabMilling);
                leFormatMilling->setObjectName(QString::fromUtf8("leFormatMilling"));
                leFormatMilling->setToolTip(QApplication::translate("SettingsDialog", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
                verticalLayoutM->addWidget(leFormatMilling);

                tabwIndividualSettings->addTab(tabMilling, QApplication::translate("SettingsDialog", "Milling", nullptr));
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

                auto lblFormatL = new QLabel(tabLaser);
                lblFormatL->setObjectName(QString::fromUtf8("lblFormatL"));
                lblFormatL->setText(QApplication::translate("SettingsDialog", "The format of the line with the coordinates:", nullptr));
                verticalLayoutL->addWidget(lblFormatL);
                leFormatLaser = new QLineEdit(tabLaser);
                leFormatLaser->setObjectName(QString::fromUtf8("leFormatLaser"));
                leFormatLaser->setToolTip(QApplication::translate("SettingsDialog", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
                verticalLayoutL->addWidget(leFormatLaser);

                tabwIndividualSettings->addTab(tabLaser, QApplication::translate("SettingsDialog", "Laser", nullptr));
            }
            { // Tab Profile
                auto tabProfile = new QWidget();
                tabProfile->setObjectName(QString::fromUtf8("tabProfile"));
                auto verticalLayoutPS = new QVBoxLayout(tabProfile);
                verticalLayoutPS->setObjectName(QString::fromUtf8("verticalLayoutPS"));
                verticalLayoutPS->setContentsMargins(6, 6, 6, 6);

                auto lblProfileSort = new QLabel(tabProfile);
                lblProfileSort->setObjectName(QString::fromUtf8("lblProfileSort"));
                lblProfileSort->setText(QApplication::translate("SettingsDialog", "Milling sequence:", nullptr));
                verticalLayoutPS->addWidget(lblProfileSort);
                cbxProfileSort = new QComboBox(tabProfile);
                cbxProfileSort->setObjectName(QString::fromUtf8("cbxProfileSort"));
                cbxProfileSort->addItem("Grouping by nesting");
                cbxProfileSort->addItem("Grouping by nesting depth");
                verticalLayoutPS->addWidget(cbxProfileSort);

                verticalLayoutPS->addStretch();

                tabwIndividualSettings->addTab(tabProfile, QApplication::translate("SettingsDialog", "Profile", nullptr));
            }
            { // Tab HLDI
                auto tabHldi = new QWidget();
                tabHldi->setObjectName(QString::fromUtf8("tabHldi"));

                chbxSimplifyHldi = new QCheckBox(QApplication::translate("SettingsDialog", "Simplify Hldi", nullptr), tabHldi);
                chbxSimplifyHldi->setObjectName(QString::fromUtf8("chbxSimplifyHldi"));

                auto verticalLayoutPS = new QVBoxLayout(tabHldi);
                verticalLayoutPS->setObjectName(QString::fromUtf8("verticalLayoutPS"));
                verticalLayoutPS->setContentsMargins(6, 6, 6, 6);
                verticalLayoutPS->addWidget(chbxSimplifyHldi);

                //                auto lblProfileSort = new QLabel(tabHldi);
                //                lblProfileSort->setObjectName(QString::fromUtf8("lblProfileSort"));
                //                lblProfileSort->setText(QApplication::translate("SettingsDialog", "Milling sequence:", nullptr));
                //                verticalLayoutPS->addWidget(lblProfileSort);
                //                cbxProfileSort = new QComboBox(tabHldi);
                //                cbxProfileSort->setObjectName(QString::fromUtf8("cbxProfileSort"));
                //                cbxProfileSort->addItem("Grouping by nesting");
                //                cbxProfileSort->addItem("Grouping by nesting depth");
                //                verticalLayoutPS->addWidget(cbxProfileSort);

                verticalLayoutPS->addStretch();

                tabwIndividualSettings->addTab(tabHldi, QApplication::translate("SettingsDialog", "HLDI", nullptr));
            }
            /**/ verticalLayout1->addWidget(tabwIndividualSettings);

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
        virtual void readSettings(MySettings& settings) override {
            settings.beginGroup("GCode");
            info_ = settings.getValue(chbxInfo, info_);
            sameFolder_ = settings.getValue(chbxSameGFolder, sameFolder_);
            fileExtension_ = settings.getValue(leFileExtension, fileExtension_);
            formatMilling_ = settings.getValue(leFormatMilling, formatMilling_);
            formatLaser_ = settings.getValue(leFormatLaser, formatLaser_);
            laserConstOn_ = settings.getValue(leLaserCPC, laserConstOn_);
            laserDynamOn_ = settings.getValue(leLaserDPC, laserDynamOn_);
            spindleOn_ = settings.getValue(leSpindleCC, spindleOn_);
            spindleLaserOff_ = settings.getValue(leSpindleLaserOff, spindleLaserOff_);

            end_ = settings.getValue(pteEnd, end_);
            start_ = settings.getValue(pteStart, start_);

            laserEnd_ = settings.getValue(pteLaserEnd, laserEnd_);
            laserStart_ = settings.getValue(pteLaserStart, laserStart_);

            simplifyHldi_ = settings.getValue(chbxSimplifyHldi, simplifyHldi_);

            profileSort_ = settings.getValue(cbxProfileSort, profileSort_);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup("GCode");
            fileExtension_ = settings.setValue(leFileExtension);
            formatMilling_ = settings.setValue(leFormatMilling);
            formatLaser_ = settings.setValue(leFormatLaser);
            info_ = settings.setValue(chbxInfo);
            laserConstOn_ = settings.setValue(leLaserCPC);
            laserDynamOn_ = settings.setValue(leLaserDPC);
            sameFolder_ = settings.setValue(chbxSameGFolder);
            spindleLaserOff_ = settings.setValue(leSpindleLaserOff);
            spindleOn_ = settings.setValue(leSpindleCC);

            start_ = settings.setValue(pteStart);
            end_ = settings.setValue(pteEnd);

            laserStart_ = settings.setValue(pteLaserStart);
            laserEnd_ = settings.setValue(pteLaserEnd);

            simplifyHldi_ = settings.setValue(chbxSimplifyHldi);

            profileSort_ = settings.setValue(cbxProfileSort);
            settings.endGroup();
        }
    };
    auto tab = new Tab(parent);
    tab->setWindowTitle("G-Code");
    return tab;
}

FileInterface* Plugin::parseFile(const QString& /*fileName*/, int /*type*/) { return nullptr; }

} // namespace GCode
