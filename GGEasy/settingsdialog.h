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
#pragma once
#include "doublespinbox.h"
#include "mvector.h"
#include "settings.h"
#include <QtCore>
#include <QtWidgets>

class SettingsTabInterface;

class SettingsDialog : public QDialog {
    Q_OBJECT

    int langIndex;
    MySettings settings;
    mvector<SettingsTabInterface*> tabs;
    QPushButton* button;

public:
    explicit SettingsDialog(QWidget* parent = nullptr, int tab = -1);
    ~SettingsDialog() override;
    void readSettings();
    void saveSettings();
    void readSettingsDialog();
    void saveSettingsDialog();
    /////////////////////
    static void translator(QApplication* app, const QString& path);
    enum {
        Ui,
        Utils,
        GCode,
        Gerber,
    };

public slots:
    void reject() override;
    void accept() override;

    // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) override;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;

private:
    class Ui {
    public:
        QGridLayout* gridLayout_2;
        QDialogButtonBox* buttonBox;
        QTabWidget* tabwMain;
        QWidget* tabGui;
        QVBoxLayout* verticalLayout_8;
        QGroupBox* groupBox;
        QGridLayout* gridLayout;
        QComboBox* cbxFontSize;
        QComboBox* cbxLanguage;
        QLabel* label_17;
        QLabel* fontSizeLabel;
        QComboBox* cbxTheme;
        QLabel* label;
        QGroupBox* gbViewer;
        QVBoxLayout* verticalLayout_2;
        QCheckBox* chbxOpenGl;
        QCheckBox* chbxAntialiasing;
        QCheckBox* chbxSmoothScSh;
        QCheckBox* chbxAnimSelection;
        QGroupBox* gbxColor;
        QFormLayout* formLayout;
        QWidget* tabUtils;
        QVBoxLayout* verticalLayout;
        QGroupBox* groupBox_5;
        QGridLayout* gridLayout_3;
        QLabel* label_6;
        QLabel* label_8;
        QLabel* label_9;
        QLabel* label_7;
        DoubleSpinBox* dsbxZeroX;
        DoubleSpinBox* dsbxZeroY;
        QComboBox* cbxZeroPos;
        QLabel* label_4;
        DoubleSpinBox* dsbxHomeX;
        DoubleSpinBox* dsbxHomeY;
        QComboBox* cbxHomePos;
        QLabel* label_5;
        DoubleSpinBox* dsbxPinX;
        DoubleSpinBox* dsbxPinY;
        QLabel* labelAPIcon;
        QCheckBox* chbxScaleHZMarkers;
        QCheckBox* chbxScalePinMarkers;
        QGroupBox* groupBox_4;
        QFormLayout* formLayout_3;
        QLabel* minimumCircleSegmentsLabel;
        QSpinBox* sbxMinCircleSegments;
        QLabel* minimumCircleSegmentLengthLabel;
        DoubleSpinBox* dsbxMinCircleSegmentLength;
        QSpacerItem* verticalSpacer_2;

        void setupUi(QDialog* SettingsDialog); // setupUi

        void retranslateUi(QDialog* SettingsDialog); // retranslateUi

    } ui;
};
