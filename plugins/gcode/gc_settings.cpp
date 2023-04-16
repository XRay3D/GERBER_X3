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
#include "gc_settings.h"
#include "gc_plugin.h"
#include "gc_types.h"

GCode::Tab::Tab(QWidget* parent)
    : AbstractFileSettings(parent) {
    setWindowTitle("G-Code");
    setObjectName(QString::fromUtf8("tabGCode"));

    tabWidget = new QTabWidget(this);
    { // рамещение tabWidget
        auto vLayout = new QVBoxLayout(this);
        vLayout->setContentsMargins(6, 6, 6, 6);
        vLayout->addWidget(tabWidget);
    }

    auto tabCommon = new QWidget(tabWidget);
    tabWidget->addTab(tabCommon, QApplication::translate("GCodeSettings", "Common Settings", nullptr));

    auto vLayout = new QVBoxLayout(tabCommon);
    vLayout->setContentsMargins(6, 6, 6, 6);
    // Папка
    chbxSameGFolder = new QCheckBox(tabCommon);
    chbxSameGFolder->setObjectName(QString::fromUtf8("chbxSameGFolder"));
    chbxSameGFolder->setText(QApplication::translate("GCodeSettings", "Save the G-Code to the project folder.", nullptr));
    vLayout->addWidget(chbxSameGFolder);

    // Инфо в УП
    chbxInfo = new QCheckBox(tabCommon);
    chbxInfo->setObjectName(QString::fromUtf8("chbxInfo"));
    chbxInfo->setText(QApplication::translate("GCodeSettings", "Add a comment with the parameters G-\320\241ode", nullptr));
    vLayout->addWidget(chbxInfo);

    // File Extension
    auto lbl = new QLabel(QApplication::translate("GCodeSettings", "File Extension:", nullptr), tabCommon);
    vLayout->addWidget(lbl);
    leFileExtension = new QLineEdit(tabCommon);
    leFileExtension->setObjectName(QString::fromUtf8("leFileExtension"));
    vLayout->addWidget(leFileExtension);
    {
        auto tabWidget = new QTabWidget(this);
        { // Tab Milling

            auto tab = new QWidget();
            auto verticalLayoutM = new QVBoxLayout(tab);
            verticalLayoutM->setContentsMargins(6, 6, 6, 6);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "Start with:", nullptr), tab);
            verticalLayoutM->addWidget(lbl);
            pteStart = new QPlainTextEdit(tab);
            pteStart->setObjectName(QString::fromUtf8("pteStart"));
            verticalLayoutM->addWidget(pteStart);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "Finish with:", nullptr), tab);
            verticalLayoutM->addWidget(lbl);
            pteEnd = new QPlainTextEdit(tab);
            pteEnd->setObjectName(QString::fromUtf8("pteEnd"));
            verticalLayoutM->addWidget(pteEnd);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "The format of the line with the coordinates:", nullptr), tab);
            verticalLayoutM->addWidget(lbl);
            leFormatMilling = new QLineEdit(tab);
            leFormatMilling->setObjectName(QString::fromUtf8("leFormatMilling"));
            leFormatMilling->setToolTip(QApplication::translate("GCodeSettings", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
            verticalLayoutM->addWidget(leFormatMilling);
            tabWidget->addTab(tab, QApplication::translate("GCodeSettings", "Milling", nullptr));
        }
        { // Tab Laser
            auto tab = new QWidget();
            auto verticalLayoutL = new QVBoxLayout(tab);
            verticalLayoutL->setContentsMargins(6, 6, 6, 6);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "Start with:", nullptr), tab);
            verticalLayoutL->addWidget(lbl);
            pteLaserStart = new QPlainTextEdit(tab);
            pteLaserStart->setObjectName(QString::fromUtf8("pteLaserStart"));
            verticalLayoutL->addWidget(pteLaserStart);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "Finish with:", nullptr), tab);
            verticalLayoutL->addWidget(lbl);
            pteLaserEnd = new QPlainTextEdit(tab);
            pteLaserEnd->setObjectName(QString::fromUtf8("pteLaserEnd"));
            verticalLayoutL->addWidget(pteLaserEnd);

            lbl = new QLabel(QApplication::translate("GCodeSettings", "The format of the line with the coordinates:", nullptr), tab);
            verticalLayoutL->addWidget(lbl);
            leFormatLaser = new QLineEdit(tab);
            leFormatLaser->setObjectName(QString::fromUtf8("leFormatLaser"));
            leFormatLaser->setToolTip(QApplication::translate("GCodeSettings", "<html><head/><body><p>Default <span style=\" font-weight:600;\">G?X?Y?Z?F?S?</span></p><p><span style=\" font-weight:600;\">?</span> - only if the value has changed.</p><p><span style=\" font-weight:600;\">+</span> - always.</p><p>If one of the commands <span style=\" font-weight:600;\">G, X, Y, Z, F</span> and<span style=\" font-weight:600;\"> S</span> is missing, it will not be inserted into the G-code.</p><p>If there is a space between the teams, then it will also be inserted into the G-code.</p><p><br/></p></body></html>", nullptr));
            verticalLayoutL->addWidget(leFormatLaser);

            tabWidget->addTab(tab, QApplication::translate("GCodeSettings", "Laser", nullptr));
        }
        vLayout->addWidget(tabWidget);
    }

    //    { // Tab HLDI
    //        auto tabHldi = new QWidget();
    //        tabHldi->setObjectName(QString::fromUtf8("tabHldi"));
    //        chbxSimplifyHldi = new QCheckBox(QApplication::translate("GCodeSettings", "Simplify Hldi", nullptr), tabHldi);
    //        chbxSimplifyHldi->setObjectName(QString::fromUtf8("chbxSimplifyHldi"));
    //        auto verticalLayoutPS = new QVBoxLayout(tabHldi);
    //        verticalLayoutPS->setObjectName(QString::fromUtf8("verticalLayoutPS"));
    //        verticalLayoutPS->setContentsMargins(6, 6, 6, 6);
    //        verticalLayoutPS->addWidget(chbxSimplifyHldi);
    //        //                lbl = new QLabel(tabHldi);
    //        //                lbl->setObjectName(QString::fromUtf8("lbl"));
    //        //                lbl->setText(QApplication::translate("GCodeSettings", "Milling sequence:", nullptr));
    //        //                verticalLayoutPS->addWidget(lbl);
    //        //                cbxProfileSort = new QComboBox(tabHldi);
    //        //                cbxProfileSort->setObjectName(QString::fromUtf8("cbxProfileSort"));
    //        //                cbxProfileSort->addItem("Grouping by nesting");
    //        //                cbxProfileSort->addItem("Grouping by nesting depth");
    //        //                verticalLayoutPS->addWidget(cbxProfileSort);
    //        verticalLayoutPS->addStretch();
    //        tabwIndividualSettings->addTab(tabHldi, QApplication::translate("GCodeSettings", "HLDI", nullptr));
    //    }

    auto grbxSpindle = new QGroupBox(tabCommon);
    grbxSpindle->setTitle(QApplication::translate("GCodeSettings", "Spindle / Laser Control Code", nullptr));
    auto formLayout = new QFormLayout(grbxSpindle);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(6, 9, 6, 6);

    lbl = new QLabel(QApplication::translate("GCodeSettings", "Spindle On:", nullptr), grbxSpindle);
    leSpindleCC = new QLineEdit(grbxSpindle);
    leSpindleCC->setObjectName(QString::fromUtf8("leSpindleCC"));
    formLayout->addRow(lbl, leSpindleCC);

    lbl = new QLabel(QApplication::translate("GCodeSettings", "Constant Laser Power Mode On:", nullptr), grbxSpindle);
    leLaserCPC = new QLineEdit(grbxSpindle);
    leLaserCPC->setObjectName(QString::fromUtf8("leLaserCPC"));
    formLayout->addRow(lbl, leLaserCPC);

    lbl = new QLabel(QApplication::translate("GCodeSettings", "Dynamic Laser Power Mode On:", nullptr), grbxSpindle);
    leLaserDPC = new QLineEdit(grbxSpindle);
    leLaserDPC->setObjectName(QString::fromUtf8("leLaserDPC"));
    formLayout->addRow(lbl, leLaserDPC);

    lbl = new QLabel(QApplication::translate("GCodeSettings", "Spindle/Laser Off:", nullptr), grbxSpindle);
    leSpindleLaserOff = new QLineEdit(grbxSpindle);
    leSpindleLaserOff->setObjectName(QString::fromUtf8("leSpindleLaserOff"));
    formLayout->addRow(lbl, leSpindleLaserOff);

    vLayout->addWidget(grbxSpindle);

    for(auto& [type, ptr]: App::gCodePlugins())
        if(auto tab = ptr->createSettingsTab(tabCommon); tab)
            tabWidget->addTab(tab, tab->windowTitle());
}

GCode::Tab::~Tab() { }

void GCode::Tab::readSettings(MySettings& settings) {
    settings.beginGroup("GCode");
    App::gcSettings().info_ = settings.getValue(chbxInfo, App::gcSettings().info_);
    App::gcSettings().sameFolder_ = settings.getValue(chbxSameGFolder, App::gcSettings().sameFolder_);
    App::gcSettings().fileExtension_ = settings.getValue(leFileExtension, App::gcSettings().fileExtension_);
    App::gcSettings().formatMilling_ = settings.getValue(leFormatMilling, App::gcSettings().formatMilling_);
    App::gcSettings().formatLaser_ = settings.getValue(leFormatLaser, App::gcSettings().formatLaser_);
    App::gcSettings().laserConstOn_ = settings.getValue(leLaserCPC, App::gcSettings().laserConstOn_);
    App::gcSettings().laserDynamOn_ = settings.getValue(leLaserDPC, App::gcSettings().laserDynamOn_);
    App::gcSettings().spindleOn_ = settings.getValue(leSpindleCC, App::gcSettings().spindleOn_);
    App::gcSettings().spindleLaserOff_ = settings.getValue(leSpindleLaserOff, App::gcSettings().spindleLaserOff_);

    App::gcSettings().end_ = settings.getValue(pteEnd, App::gcSettings().end_);
    App::gcSettings().start_ = settings.getValue(pteStart, App::gcSettings().start_);

    App::gcSettings().laserEnd_ = settings.getValue(pteLaserEnd, App::gcSettings().laserEnd_);
    App::gcSettings().laserStart_ = settings.getValue(pteLaserStart, App::gcSettings().laserStart_);

    for(int i{1}; i < tabWidget->count(); ++i) {
        auto tab = static_cast<AbstractFileSettings*>(tabWidget->widget(i));
        tab->readSettings(settings);
    }

    settings.endGroup();
}

void GCode::Tab::writeSettings(MySettings& settings) {
    settings.beginGroup("GCode");
    App::gcSettings().fileExtension_ = settings.setValue(leFileExtension);
    App::gcSettings().formatMilling_ = settings.setValue(leFormatMilling);
    App::gcSettings().formatLaser_ = settings.setValue(leFormatLaser);
    App::gcSettings().info_ = settings.setValue(chbxInfo);
    App::gcSettings().laserConstOn_ = settings.setValue(leLaserCPC);
    App::gcSettings().laserDynamOn_ = settings.setValue(leLaserDPC);
    App::gcSettings().sameFolder_ = settings.setValue(chbxSameGFolder);
    App::gcSettings().spindleLaserOff_ = settings.setValue(leSpindleLaserOff);
    App::gcSettings().spindleOn_ = settings.setValue(leSpindleCC);

    App::gcSettings().start_ = settings.setValue(pteStart);
    App::gcSettings().end_ = settings.setValue(pteEnd);

    App::gcSettings().laserStart_ = settings.setValue(pteLaserStart);
    App::gcSettings().laserEnd_ = settings.setValue(pteLaserEnd);

    for(int i{1}; i < tabWidget->count(); ++i) {
        auto tab = static_cast<AbstractFileSettings*>(tabWidget->widget(i));
        tab->writeSettings(settings);
    }
    /*
         bool simplifyHldi_ {false};
         int profileSort_ = 0;
        bool simplifyHldi() { return simplifyHldi_; }
        int profileSort() { return profileSort_; }
    */
    settings.endGroup();
}
