#include "settingsdialog.h"
#include "colorselector.h"

#include <QApplication>
#include <QGLWidget>
#include <QtWidgets>
#include <graphicsview.h>

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

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    chbxOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(m_guiColor[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }

    connect(cbxFontSize, &QComboBox::currentTextChanged, [](const QString& fontSize) {
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegExp("font-size:\\s*\\d+"), "font-size: " + fontSize));
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
    });

    labelAPIcon->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(labelAPIcon->size()));
    readSettings();
}

void SettingsDialog::readSettings()
{
    MySettings settings;
    settings.beginGroup("Viewer");
    settings.getValue(chbxOpenGl);
    settings.getValue(chbxAntialiasing);
    m_guiSmoothScSh = settings.getValue(chbxSmoothScSh);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        m_guiColor[i].setNamedColor(settings.value(QString("%1").arg(i), m_guiColor[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Gerber");
    m_gbrCleanPolygons = settings.getValue(chbxCleanPolygons, m_gbrCleanPolygons);
    m_gbrSimplifyRegions = settings.getValue(chbxSimplifyRegions, m_gbrSimplifyRegions);
    m_gbrSkipDuplicates = settings.getValue(chbxSkipDuplicates, m_gbrSkipDuplicates);
    m_gbrGcMinCircleSegmentLength = settings.getValue(dsbxMinCircleSegmentLength, m_gbrGcMinCircleSegmentLength);
    m_gbrGcMinCircleSegments = settings.getValue(sbxMinCircleSegments, m_gbrGcMinCircleSegments);
    settings.endGroup();

    settings.beginGroup("GCode");
    m_gcInfo = settings.getValue(chbxInfo, m_gcInfo);
    m_gcSameFolder = settings.getValue(chbxSameGFolder, m_gcSameFolder);
    m_gcFileExtension = settings.getValue(leFileExtension, m_gcFileExtension);
    m_gcFormat = settings.getValue(leFormat, m_gcFormat);
    m_gcLaserConstOn = settings.getValue(leLaserCPC, m_gcLaserConstOn);
    m_gcLaserDynamOn = settings.getValue(leLaserDPC, m_gcLaserDynamOn);
    m_gcSpindleOn = settings.getValue(leSpindleCC, m_gcSpindleOn);
    m_gcSpindleLaserOff = settings.getValue(leSpindleLaserOff, m_gcSpindleLaserOff);
    m_gcEnd = settings.getValue(pteEnd, m_gcEnd);
    m_gcStart = settings.getValue(pteStart, m_gcStart);
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    settings.beginGroup("Home");
    settings.getValue("homeOffset", m_mrkHomeOffset);
    settings.getValue("pinOffset", m_mrkPinOffset, QPointF(6, 6));
    settings.getValue("zeroOffset", m_mrkZeroOffset);
    dsbxHomeX->setValue(m_mrkHomeOffset.x());
    dsbxHomeY->setValue(m_mrkHomeOffset.y());
    dsbxPinX->setValue(m_mrkPinOffset.x());
    dsbxPinY->setValue(m_mrkPinOffset.y());
    dsbxZeroX->setValue(m_mrkZeroOffset.x());
    dsbxZeroY->setValue(m_mrkZeroOffset.y());
    m_mrkHomePos = settings.getValue(cbxHomePos, HomePosition::TopLeft);
    m_mrkZeroPos = settings.getValue(cbxZeroPos, HomePosition::TopLeft);
    settings.endGroup();

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
    m_guiSmoothScSh = settings.setValue(chbxSmoothScSh);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        settings.setValue(QString("%1").arg(i), m_guiColor[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Gerber");
    m_gbrCleanPolygons = settings.setValue(chbxCleanPolygons);
    m_gbrGcMinCircleSegmentLength = settings.setValue(dsbxMinCircleSegmentLength);
    m_gbrGcMinCircleSegments = settings.setValue(sbxMinCircleSegments);
    m_gbrSimplifyRegions = settings.setValue(chbxSimplifyRegions);
    m_gbrSkipDuplicates = settings.setValue(chbxSkipDuplicates);
    settings.endGroup();

    settings.beginGroup("GCode");
    m_gcEnd = settings.setValue(pteEnd);
    m_gcFileExtension = settings.setValue(leFileExtension);
    m_gcFormat = settings.setValue(leFormat);
    m_gcInfo = settings.setValue(chbxInfo);
    m_gcLaserConstOn = settings.setValue(leLaserCPC);
    m_gcLaserDynamOn = settings.setValue(leLaserDPC);
    m_gcSameFolder = settings.setValue(chbxSameGFolder);
    m_gcSpindleLaserOff = settings.setValue(leSpindleLaserOff);
    m_gcSpindleOn = settings.setValue(leSpindleCC);
    m_gcStart = settings.setValue(pteStart);
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    settings.beginGroup("Home");
    m_mrkHomeOffset = settings.setValue("homeOffset", QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    m_mrkHomePos = settings.setValue(cbxHomePos);
    m_mrkPinOffset = settings.setValue("pinOffset", QPointF(dsbxPinX->value(), dsbxPinY->value()));
    m_mrkZeroOffset = settings.setValue("zeroOffset", QPointF(dsbxZeroX->value(), dsbxZeroY->value()));
    m_mrkZeroPos = settings.setValue(cbxZeroPos);
    settings.endGroup();

    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue(tabWidget);
    settings.endGroup();
}

void SettingsDialog::reject()
{
    readSettings();
    QDialog::reject();
}

void SettingsDialog::accept()
{
    writeSettings();
    QDialog::accept();
}
