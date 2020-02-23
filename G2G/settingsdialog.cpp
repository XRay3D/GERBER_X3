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
    chbOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

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
    QSettings settings;
    settings.beginGroup("Viewer");
    chbAntialiasing->setChecked(settings.value("Antialiasing").toBool());
    chbOpenGl->setChecked(settings.value("OpenGl").toBool());
    chbSmoothScSh->setChecked(m_guiSmoothScSh = settings.value("SmoothScSh").toBool());
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        m_guiColor[i].setNamedColor(settings.value(QString("%1").arg(i), m_guiColor[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Gerber");
    chbxCleanPolygons->setChecked(m_gbrCleanPolygons = settings.value("CleanPolygons", false).toBool());
    chbxSimplifyRegions->setChecked(m_gbrSimplifyRegions = settings.value("SimplifyRegions", false).toBool());
    chbxSkipDuplicates->setChecked(m_gbrSkipDuplicates = settings.value("SkipDuplicates", false).toBool());
    dsbxMinCircleSegmentLength->setValue(m_gbrGcMinCircleSegmentLength = settings.value("MinCircleSegmentLenght", 0.5).toDouble());
    sbxMinCircleSegments->setValue(m_gbrGcMinCircleSegments = settings.value("MinCircleSegments", 36).toInt());
    settings.endGroup();

    settings.beginGroup("GCode");
    chbxInfo->setChecked(m_gcInfo = settings.value("Info", false).toBool());
    chbxSameGFolder->setChecked(m_gcSameFolder = settings.value("SameGCodeFolder", true).toBool());
    leFileExtension->setText(m_gcFileExtension = settings.value("FileExtension", "tap").toString());
    leFormat->setText(m_gcFormat = settings.value("Format", "G?X?Y?Z?F?S?").toString());
    leLaserCPC->setText(m_gcLaserConstOn = settings.value("LaserConstOn", "M3").toString());
    leLaserDPC->setText(m_gcLaserDynamOn = settings.value("LaserDynamOn", "M4").toString());
    leSpindleCC->setText(m_gcSpindleOn = settings.value("SpindleOn", "M3").toString());
    leSpindleLaserOff->setText(m_gcSpindleLaserOff = settings.value("SpindleLaserOff", "M5").toString());
    pteEnd->setPlainText(m_gcEnd = settings.value("End", "M5\nM30").toString());
    pteStart->setPlainText(m_gcStart = settings.value("Start", "G21 G17 G90\nM3").toString());
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    settings.beginGroup("Home");
    m_mrkHomeOffset = settings.value("homeOffset").toPointF();
    m_mrkPinOffset = settings.value("pinOffset", QPointF(6, 6)).toPointF();
    m_mrkZeroOffset = settings.value("zeroOffset").toPointF();
    dsbxHomeX->setValue(m_mrkHomeOffset.x());
    dsbxHomeY->setValue(m_mrkHomeOffset.y());
    dsbxPinX->setValue(m_mrkPinOffset.x());
    dsbxPinY->setValue(m_mrkPinOffset.y());
    dsbxZeroX->setValue(m_mrkZeroOffset.x());
    dsbxZeroY->setValue(m_mrkZeroOffset.y());
    cbxHomePos->setCurrentIndex(m_mrkHomePos = settings.value("homePos", HomePosition::BottomLeft).toInt());
    cbxZeroPos->setCurrentIndex(m_mrkZeroPos = settings.value("zeroPos", HomePosition::BottomLeft).toInt());
    settings.endGroup();

    settings.beginGroup("SettingsDialog");
    if (isVisible())
        settings.setValue("geometry", saveGeometry());
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    settings.endGroup();
}

void SettingsDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Viewer");
    if (settings.value("OpenGl").toBool() != chbOpenGl->isChecked()) {
        GraphicsView::m_instance->setViewport(chbOpenGl->isChecked()
                ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
                : new QWidget);
        GraphicsView::m_instance->viewport()->setObjectName("viewport");
        GraphicsView::m_instance->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("OpenGl", chbOpenGl->isChecked());
    }
    if (settings.value("Antialiasing").toBool() != chbAntialiasing->isChecked()) {
        GraphicsView::m_instance->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("Antialiasing", chbAntialiasing->isChecked());
    }
    settings.setValue("SmoothScSh", m_guiSmoothScSh = chbSmoothScSh->isChecked());
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        settings.setValue(QString("%1").arg(i), m_guiColor[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Gerber");
    settings.setValue("CleanPolygons", (m_gbrCleanPolygons = chbxCleanPolygons->isChecked()));
    settings.setValue("MinCircleSegmentLenght", (m_gbrGcMinCircleSegmentLength = dsbxMinCircleSegmentLength->value()));
    settings.setValue("MinCircleSegments", (m_gbrGcMinCircleSegments = sbxMinCircleSegments->value()));
    settings.setValue("SimplifyRegions", (m_gbrSimplifyRegions = chbxSimplifyRegions->isChecked()));
    settings.setValue("SkipDuplicates", (m_gbrSkipDuplicates = chbxSkipDuplicates->isChecked()));
    settings.endGroup();

    settings.beginGroup("GCode");
    settings.setValue("End", m_gcEnd = pteEnd->toPlainText());
    settings.setValue("FileExtension", m_gcFileExtension = leFileExtension->text());
    settings.setValue("Format", m_gcFormat = leFormat->text());
    settings.setValue("Info", (m_gcInfo = chbxInfo->isChecked()));
    settings.setValue("LaserConstOn", m_gcLaserConstOn = leLaserCPC->text());
    settings.setValue("LaserDynamOn", m_gcLaserDynamOn = leLaserDPC->text());
    settings.setValue("SameGCodeFolder", m_gcSameFolder = chbxSameGFolder->isChecked());
    settings.setValue("SpindleLaserOff", m_gcSpindleLaserOff = leSpindleLaserOff->text());
    settings.setValue("SpindleOn", m_gcSpindleOn = leSpindleCC->text());
    settings.setValue("Start", m_gcStart = pteStart->toPlainText());
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    settings.beginGroup("Home");
    settings.setValue("homeOffset", m_mrkHomeOffset = QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    settings.setValue("homePos", m_mrkHomePos = cbxHomePos->currentIndex());
    settings.setValue("pinOffset", m_mrkPinOffset = QPointF(dsbxPinX->value(), dsbxPinY->value()));
    settings.setValue("zeroOffset", m_mrkZeroOffset = QPointF(dsbxZeroX->value(), dsbxZeroY->value()));
    settings.setValue("zeroPos", m_mrkZeroPos = cbxZeroPos->currentIndex());
    settings.endGroup();

    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
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
