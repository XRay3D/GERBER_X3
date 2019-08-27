#include "settingsdialog.h"
#include "colorselector.h"

#include <QApplication>
#include <QGLWidget>
#include <QtWidgets>
#include <graphicsview.h>

const int gridColor = 100;

const QColor defaultColor[(size_t)Colors::Count] {
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

const QString colorName[(size_t)Colors::Count] {
    "Background",
    "Pin",
    "CutArea",
    "Grid1",
    "Grid5",
    "Grid10",
    "Hole",
    "Home",
    "ToolPath",
    "Zero",
    "G0",
};

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    chbOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(m_color[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }

    connect(cbxFontSize, &QComboBox::currentTextChanged, [](const QString& fontSize) {
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegExp("font-size:\\s*\\d+"), "font-size: " + fontSize));
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
    });

    readSettings();
}

void SettingsDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Viewer");
    chbOpenGl->setChecked(settings.value("OpenGl").toBool());
    chbAntialiasing->setChecked(settings.value("Antialiasing").toBool());
    chbSmoothScSh->setChecked(m_smoothScSh = settings.value("SmoothScSh").toBool());
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        m_color[i].setNamedColor(settings.value(QString("%1").arg(i), m_color[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Gerber");
    sbxMinCircleSegments->setValue(m_minCircleSegments = settings.value("MinCircleSegments", 36).toInt());
    dsbxMinCircleSegmentLength->setValue(m_minCircleSegmentLength = settings.value("MinCircleSegmentLenght", 0.5).toDouble());
    chbxCleanPolygons->setChecked(m_cleanPolygons = settings.value("CleanPolygons", true).toBool());
    chbxSkipDuplicates->setChecked(m_skipDuplicates = settings.value("SkipDuplicates", false).toBool());
    chbxInfo->setChecked(m_gcinfo = settings.value("GCInfo", false).toBool());
    settings.endGroup();

    settings.beginGroup("GCode");
    leFormat->setText(m_GCode = settings.value("GCodeFormat", "G?X?Y?Z?F?S?").toString());
    pteStart->setPlainText(m_startGCode = settings.value("GCodeStart", "G21 G17 G90|M3").toString().replace('|', '\n'));
    pteEnd->setPlainText(m_endGCode = settings.value("GCodeEnd", "M5|M30").toString().replace('|', '\n'));
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    settings.beginGroup("Home");
    m_pinOffset = settings.value("pinOffset", QPointF(6, 6)).toPointF();
    m_homeOffset = settings.value("homeOffset").toPointF();
    dsbxPinX->setValue(m_pinOffset.x());
    dsbxPinY->setValue(m_pinOffset.y());
    dsbxHomeX->setValue(m_homeOffset.x());
    dsbxHomeY->setValue(m_homeOffset.y());
    cbxHomePos->setCurrentIndex(m_homePos = settings.value("homePos").toInt());
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
        GraphicsView::self->setViewport(chbOpenGl->isChecked()
                ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
                : new QWidget);
        GraphicsView::self->viewport()->setObjectName("viewport");
        GraphicsView::self->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("OpenGl", chbOpenGl->isChecked());
    }
    if (settings.value("Antialiasing").toBool() != chbAntialiasing->isChecked()) {
        GraphicsView::self->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("Antialiasing", chbAntialiasing->isChecked());
    }
    settings.setValue("SmoothScSh", m_smoothScSh = chbSmoothScSh->isChecked());
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i)
        settings.setValue(QString("%1").arg(i), m_color[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Gerber");
    settings.setValue("MinCircleSegments", (m_minCircleSegments = sbxMinCircleSegments->value()));
    settings.setValue("MinCircleSegmentLenght", (m_minCircleSegmentLength = dsbxMinCircleSegmentLength->value()));
    settings.setValue("CleanPolygons", (m_cleanPolygons = chbxCleanPolygons->isChecked()));
    settings.setValue("SkipDuplicates", (m_skipDuplicates = chbxSkipDuplicates->isChecked()));
    settings.setValue("GCInfo", (m_gcinfo = chbxInfo->isChecked()));
    settings.endGroup();

    settings.beginGroup("GCode");
    settings.setValue("GCodeFormat", m_GCode = leFormat->text());
    m_startGCode = pteStart->toPlainText();
    settings.setValue("GCodeStart", pteStart->toPlainText().replace('\n', '|'));
    m_endGCode = pteEnd->toPlainText();
    settings.setValue("GCodeEnd", pteEnd->toPlainText().replace('\n', '|'));
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    settings.beginGroup("Home");
    settings.setValue("pinOffset", m_pinOffset = QPointF(dsbxPinX->value(), dsbxPinY->value()));
    settings.setValue("homeOffset", m_homeOffset = QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    settings.setValue("homePos", m_homePos = cbxHomePos->currentIndex());
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
