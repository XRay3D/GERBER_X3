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
#include "settingsdialog.h"
#include "colorselector.h"
#include "graphicsview.h"
#include <QGLWidget>
#include <QtWidgets>
#include <mainwindow.h>

#include "leakdetector.h"

const int gridColor = 100;

const QColor defaultColor[static_cast<size_t>(Colors::Count)] {
    QColor(), //Background
    QColor(255, 255, 0, 120), //Pin
    QColor(Qt::gray), //CutArea
    QColor(gridColor, gridColor, gridColor, 50), //Grid1
    QColor(gridColor, gridColor, gridColor, 100), //Grid5
    QColor(gridColor, gridColor, gridColor, 200), //Grid10
    QColor(), //Hole
    QColor(0, 255, 0, 120), //Home
    QColor(Qt::black), //ToolPath
    QColor(255, 0, 0, 120), //Zero
    QColor(Qt::red) //G0
};

const QString colorName[static_cast<size_t>(Colors::Count)] {
    QObject::tr("Background"),
    QObject::tr("Pin"),
    QObject::tr("CutArea"),
    QObject::tr("Grid 0.1"),
    QObject::tr("Grid 0.5"),
    QObject::tr("Grid 1.0"),
    QObject::tr("Hole"),
    QObject::tr("Home"),
    QObject::tr("ToolPath"),
    QObject::tr("Zero"),
    QObject::tr("G0"),
};

SettingsDialog::SettingsDialog(QWidget* parent, int tab)
    : QDialog(parent)
{
    setupUi(this);
    chbxOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(m_settings->m_guiColor[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }

    connect(cbxFontSize, &QComboBox::currentTextChanged, [](const QString& fontSize) {
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegExp("font-size:\\s*\\d+"), "font-size: " + fontSize));
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
    });

    { // Language
        cbxLanguage->addItem("English", "en");
        cbxLanguage->addItem("Русский", "ru");

        QSettings settings;
        settings.beginGroup("MainWindow");
        QString locale(settings.value("locale").toString());
        settings.endGroup();

        for (int i = 0; i < cbxLanguage->count(); ++i) {
            if (cbxLanguage->itemData(i).toString() == locale) {
                cbxLanguage->setCurrentIndex(i);
                langIndex = i;
                break;
            }
        }

        connect(cbxLanguage, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
            const QString locale(cbxLanguage->itemData(index).toString());
            MainWindow::translate(locale);

            QSettings settings;
            settings.beginGroup("MainWindow");
            settings.setValue("locale", locale);
            settings.endGroup();
        });
    }

    labelAPIcon->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(labelAPIcon->size()));
    readSettings();
    resize(10, 10);
    if (tab > -1)
        tabWidget->setCurrentIndex(tab);
}

void SettingsDialog::readSettings()
{
    MySettings settings;

    /*GUI*/
    settings.beginGroup("Viewer");
    settings.getValue(chbxOpenGl);
    settings.getValue(chbxAntialiasing);
    m_settings->m_guiSmoothScSh = settings.getValue(chbxSmoothScSh, m_settings->m_guiSmoothScSh);
    m_settings->m_animSelection = settings.getValue(chbxAnimSelection, m_settings->m_animSelection);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        m_settings->m_guiColor[i].setNamedColor(settings.value(QString("%1").arg(i), m_settings->m_guiColor[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    /*Gerber*/
    settings.beginGroup("Gerber");
    m_settings->m_gbrCleanPolygons = settings.getValue(chbxCleanPolygons, m_settings->m_gbrCleanPolygons);
    m_settings->m_gbrSimplifyRegions = settings.getValue(chbxSimplifyRegions, m_settings->m_gbrSimplifyRegions);
    m_settings->m_gbrSkipDuplicates = settings.getValue(chbxSkipDuplicates, m_settings->m_gbrSkipDuplicates);
    m_settings->m_gbrGcMinCircleSegmentLength = settings.getValue(dsbxMinCircleSegmentLength, m_settings->m_gbrGcMinCircleSegmentLength);
    m_settings->m_gbrGcMinCircleSegments = settings.getValue(sbxMinCircleSegments, m_settings->m_gbrGcMinCircleSegments);
    settings.endGroup();

    /*G-Code*/
    settings.beginGroup("GCode");
    m_settings->m_gcInfo = settings.getValue(chbxInfo, m_settings->m_gcInfo);
    m_settings->m_gcSameFolder = settings.getValue(chbxSameGFolder, m_settings->m_gcSameFolder);
    m_settings->m_gcFileExtension = settings.getValue(leFileExtension, m_settings->m_gcFileExtension);
    m_settings->m_gcFormat = settings.getValue(leFormat, m_settings->m_gcFormat);
    m_settings->m_gcLaserConstOn = settings.getValue(leLaserCPC, m_settings->m_gcLaserConstOn);
    m_settings->m_gcLaserDynamOn = settings.getValue(leLaserDPC, m_settings->m_gcLaserDynamOn);
    m_settings->m_gcSpindleOn = settings.getValue(leSpindleCC, m_settings->m_gcSpindleOn);
    m_settings->m_gcSpindleLaserOff = settings.getValue(leSpindleLaserOff, m_settings->m_gcSpindleLaserOff);

    m_settings->m_gcEnd = settings.getValue(pteEnd, m_settings->m_gcEnd);
    m_settings->m_gcStart = settings.getValue(pteStart, m_settings->m_gcStart);

    m_settings->m_gcLaserEnd = settings.getValue(pteLaserEnd, m_settings->m_gcLaserEnd);
    m_settings->m_gcLaserStart = settings.getValue(pteLaserStart, m_settings->m_gcLaserStart);
    settings.endGroup();

    /*DXF*/
    settings.beginGroup("Dxf");
    m_settings->m_dxfDefaultFont = settings.getValue(fcbxDxfDefaultFont, "Arial");
    m_settings->m_dxfBoldFont = settings.getValue(chbxDxfBoldFont, false);
    m_settings->m_dxfItalicFont = settings.getValue(chbxDxfItalicFont, false);
    m_settings->m_dxfOverrideFonts = settings.getValue(chbxDxfOverrideFonts, false);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    settings.getValue("homeOffset", m_settings->m_mrkHomeOffset);
    settings.getValue("pinOffset", m_settings->m_mrkPinOffset, QPointF(6, 6));
    settings.getValue("zeroOffset", m_settings->m_mrkZeroOffset);
    dsbxHomeX->setValue(m_settings->m_mrkHomeOffset.x());
    dsbxHomeY->setValue(m_settings->m_mrkHomeOffset.y());
    dsbxPinX->setValue(m_settings->m_mrkPinOffset.x());
    dsbxPinY->setValue(m_settings->m_mrkPinOffset.y());
    dsbxZeroX->setValue(m_settings->m_mrkZeroOffset.x());
    dsbxZeroY->setValue(m_settings->m_mrkZeroOffset.y());
    m_settings->m_mrkHomePos = settings.getValue(cbxHomePos, HomePosition::TopLeft);
    m_settings->m_mrkZeroPos = settings.getValue(cbxZeroPos, HomePosition::TopLeft);
    settings.endGroup();

    /*SettingsDialog*/
    settings.beginGroup("SettingsDialog");
    //    if (isVisible())
    //        settings.setValue("geometry", saveGeometry());
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    settings.getValue(tabWidget);
    settings.endGroup();
}

void SettingsDialog::writeSettings()
{
    MySettings settings;

    /*GUI*/
    settings.beginGroup("Viewer");
    if (settings.value("chbxOpenGl").toBool() != chbxOpenGl->isChecked()) {
        App::graphicsView()->setViewport(chbxOpenGl->isChecked()
                ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
                : new QWidget);
        App::graphicsView()->viewport()->setObjectName("viewport");
        App::graphicsView()->setRenderHint(QPainter::Antialiasing, chbxAntialiasing->isChecked());
        settings.setValue(chbxOpenGl);
    }
    if (settings.value("chbxAntialiasing").toBool() != chbxAntialiasing->isChecked()) {
        App::graphicsView()->setRenderHint(QPainter::Antialiasing, chbxAntialiasing->isChecked());
        settings.setValue(chbxAntialiasing);
    }
    m_settings->m_guiSmoothScSh = settings.setValue(chbxSmoothScSh);
    m_settings->m_animSelection = settings.setValue(chbxAnimSelection);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        settings.setValue(QString("%1").arg(i), m_settings->m_guiColor[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    /*Gerber*/
    settings.beginGroup("Gerber");
    m_settings->m_gbrCleanPolygons = settings.setValue(chbxCleanPolygons);
    m_settings->m_gbrGcMinCircleSegmentLength = settings.setValue(dsbxMinCircleSegmentLength);
    m_settings->m_gbrGcMinCircleSegments = settings.setValue(sbxMinCircleSegments);
    m_settings->m_gbrSimplifyRegions = settings.setValue(chbxSimplifyRegions);
    m_settings->m_gbrSkipDuplicates = settings.setValue(chbxSkipDuplicates);
    settings.endGroup();

    /*G-Code*/
    settings.beginGroup("GCode");
    m_settings->m_gcFileExtension = settings.setValue(leFileExtension);
    m_settings->m_gcFormat = settings.setValue(leFormat);
    m_settings->m_gcInfo = settings.setValue(chbxInfo);
    m_settings->m_gcLaserConstOn = settings.setValue(leLaserCPC);
    m_settings->m_gcLaserDynamOn = settings.setValue(leLaserDPC);
    m_settings->m_gcSameFolder = settings.setValue(chbxSameGFolder);
    m_settings->m_gcSpindleLaserOff = settings.setValue(leSpindleLaserOff);
    m_settings->m_gcSpindleOn = settings.setValue(leSpindleCC);

    m_settings->m_gcStart = settings.setValue(pteStart);
    m_settings->m_gcEnd = settings.setValue(pteEnd);

    m_settings->m_gcLaserStart = settings.setValue(pteLaserStart);
    m_settings->m_gcLaserEnd = settings.setValue(pteLaserEnd);
    settings.endGroup();

    /*DXF*/
    settings.beginGroup("Dxf");
    m_settings->m_dxfDefaultFont = settings.setValue(fcbxDxfDefaultFont);
    m_settings->m_dxfBoldFont = settings.setValue(chbxDxfBoldFont);
    m_settings->m_dxfItalicFont = settings.setValue(chbxDxfItalicFont);
    m_settings->m_dxfOverrideFonts = settings.setValue(chbxDxfOverrideFonts);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    m_settings->m_mrkHomeOffset = settings.setValue("homeOffset", QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    m_settings->m_mrkHomePos = settings.setValue(cbxHomePos);
    m_settings->m_mrkPinOffset = settings.setValue("pinOffset", QPointF(dsbxPinX->value(), dsbxPinY->value()));
    m_settings->m_mrkZeroOffset = settings.setValue("zeroOffset", QPointF(dsbxZeroX->value(), dsbxZeroY->value()));
    m_settings->m_mrkZeroPos = settings.setValue(cbxZeroPos);
    settings.endGroup();

    /*SettingsDialog*/
    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue(tabWidget);
    settings.endGroup();
}

void SettingsDialog::translator(QApplication* app, const QString& path)
{
    if (QFile::exists(path)) {
        QTranslator* pTranslator = new QTranslator(qApp);
        if (pTranslator->load(path))
            app->installTranslator(pTranslator);
        else
            delete pTranslator;
    }
}

void SettingsDialog::reject()
{
    readSettings();
    QDialog::reject();
}

void SettingsDialog::accept()
{
    if (langIndex != cbxLanguage->currentIndex()) {
        QMessageBox::information(this, "", tr("The complete translation of the application will take\n"
                                              "effect after restarting the application."));
    }

    writeSettings();
    QDialog::accept();
}
