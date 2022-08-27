// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "settingsdialog.h"
#include "colorselector.h"
#include "file_plugin.h"
#include "graphicsview.h"

#include <QDesktopServices>
#include <QtWidgets>
#include <mainwindow.h>

const int gridColor = 100;

const QColor defaultColor[GuiColors::Count] {
    QColor(),                                     // Background
    QColor(255, 255, 0, 120),                     // Pin
    QColor(Qt::gray),                             // CutArea
    QColor(gridColor, gridColor, gridColor, 50),  // Grid1
    QColor(gridColor, gridColor, gridColor, 100), // Grid5
    QColor(gridColor, gridColor, gridColor, 200), // Grid10
    QColor(),                                     // Hole
    QColor(0, 255, 0, 120),                       // Home
    QColor(Qt::black),                            // ToolPath
    QColor(255, 0, 0, 120),                       // Zero
    QColor(Qt::red)                               // G0
};

const QString colorName[GuiColors::Count] {
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

class ModelSettings : public QAbstractListModel {
    std::vector<QWidget*> data_;

public:
    ModelSettings(QObject* parent = nullptr)
        : QAbstractListModel {parent} {
    }
    virtual ~ModelSettings() { }

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& /*parent*/) const override { return data_.size(); }
    int columnCount(const QModelIndex& /*parent*/) const override { return 1; }
    QVariant data(const QModelIndex& index, int role) const override {
        if (role == Qt::DisplayRole)
            return data_[index.row()]->windowTitle();
        return {};
    }
    void addWidget(QWidget* w) { data_.emplace_back(w); }
};

////////////////////////////////////////////////////////////////////////////
/// \brief SettingsDialog::SettingsDialog
/// \param parent
/// \param tab
///
SettingsDialog::SettingsDialog(QWidget* parent, int tab)
    : QDialog(parent) {
    setupUi(this);

    chbxOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

    for (int i = 0; i < GuiColors::Count; ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(App::settings().guiColor_[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }

    connect(cbxFontSize, &QComboBox::currentTextChanged, [](const QString& fontSize) {
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegularExpression(R"(font-size:\s*\d+)"), "font-size: " + fontSize));
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
    });

    // Language
    cbxLanguage->addItem("English", "en");
    cbxLanguage->addItem("Русский", "ru");

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
        settings.beginGroup("MainWindow");
        settings.setValue("locale", locale);
        settings.endGroup();
    });

    labelAPIcon->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(labelAPIcon->size()));

    tabs.reserve(App::filePlugins().size());

    //    auto model = new ModelSettings(listView);

    for (auto& [type, ptr] : App::filePlugins()) {
        auto tab = ptr->createSettingsTab(this);
        if (!tab)
            continue;

        //        model->addWidget(tab);
        //        auto gbx = new QGroupBox(tab->windowTitle(), this);
        //        auto lay = new QHBoxLayout(gbx);
        //        lay->addWidget(tab);
        //        verticalLayout_3->addWidget(gbx);

        tabs.push_back(tab);
        tabwMain->addTab(tab, tab->windowTitle());
        tab->readSettings(settings);
    }

    //    listView->setModel(model);

    readSettings();
    readSettingsDialog();
    if (tab > -1)
        tabwMain->setCurrentIndex(tab);

    { // Open Settings Folder
        button = new QPushButton(tr("Open Settings Folder"), buttonBox);
        button->setIcon(QIcon::fromTheme("folder"));
        button->setMinimumWidth(QFontMetrics(font()).boundingRect(button->text()).width() + 32);
        buttonBox->addButton(button, QDialogButtonBox::NRoles);
        connect(button, &QPushButton::clicked, [] { QDesktopServices::openUrl(QUrl(App::settingsPath(), QUrl::TolerantMode)); });
    }

    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme("dialog-cancel"));
}

SettingsDialog::~SettingsDialog() { saveSettingsDialog(); }

void SettingsDialog::readSettings() {
    /*GUI*/
    settings.beginGroup("Viewer");
    settings.getValue(chbxAntialiasing);
    settings.getValue(chbxOpenGl);
    settings.getValue(chbxScaleHZMarkers, App::settings().scaleHZMarkers_);
    settings.getValue(chbxScalePinMarkers, App::settings().scalePinMarkers_);
    settings.getValue(chbxSmoothScSh, App::settings().guiSmoothScSh_);
    settings.getValue(chbxAnimSelection, App::settings().animSelection_);
    settings.getValue(cbxTheme, App::settings().theme_);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < GuiColors::Count; ++i)
        App::settings().guiColor_[i].setNamedColor(settings.value(QString("%1").arg(i), App::settings().guiColor_[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    /*Clipper*/
    settings.beginGroup("Clipper");
    settings.getValue(dsbxMinCircleSegmentLength, App::settings().clpMinCircleSegmentLength_);
    settings.getValue(sbxMinCircleSegments, App::settings().clpMinCircleSegments_);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    settings.getValue("homeOffset", App::settings().mrkHomeOffset_);
    settings.getValue("pinOffset", App::settings().mrkPinOffset_, QPointF(6, 6));
    settings.getValue("zeroOffset", App::settings().mrkZeroOffset_);
    dsbxHomeX->setValue(App::settings().mrkHomeOffset_.x());
    dsbxHomeY->setValue(App::settings().mrkHomeOffset_.y());
    dsbxPinX->setValue(App::settings().mrkPinOffset_.x());
    dsbxPinY->setValue(App::settings().mrkPinOffset_.y());
    dsbxZeroX->setValue(App::settings().mrkZeroOffset_.x());
    dsbxZeroY->setValue(App::settings().mrkZeroOffset_.y());
    settings.getValue(cbxHomePos, HomePosition::TopLeft);
    settings.getValue(cbxZeroPos, HomePosition::TopLeft);
    settings.endGroup();
    /*Other*/
    settings.getValue(App::settings().inch_, "inch", false);
    settings.getValue(App::settings().snap_, "snap", false);
    for (auto tab : tabs)
        tab->readSettings(settings);
}

void SettingsDialog::saveSettings() {
    /*GUI*/
    settings.beginGroup("Viewer");
    if (settings.value("chbxOpenGl").toBool() != chbxOpenGl->isChecked()) {
        App::graphicsView()->setOpenGL(chbxOpenGl->isChecked());
        App::graphicsView()->viewport()->setObjectName("viewport");
        App::graphicsView()->setRenderHint(QPainter::Antialiasing, chbxAntialiasing->isChecked());
        settings.setValue(chbxOpenGl);
    }
    if (settings.value("chbxAntialiasing").toBool() != chbxAntialiasing->isChecked()) {
        App::graphicsView()->setRenderHint(QPainter::Antialiasing, chbxAntialiasing->isChecked());
        settings.setValue(chbxAntialiasing);
    }
    App::settings().animSelection_ = settings.setValue(chbxAnimSelection);
    App::settings().guiSmoothScSh_ = settings.setValue(chbxSmoothScSh);
    App::settings().scaleHZMarkers_ = settings.setValue(chbxScaleHZMarkers);
    App::settings().scalePinMarkers_ = settings.setValue(chbxScalePinMarkers);
    App::settings().theme_ = settings.setValue(cbxTheme);
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < GuiColors::Count; ++i)
        settings.setValue(QString("%1").arg(i), App::settings().guiColor_[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    /*Clipper*/
    settings.beginGroup("Clipper");
    App::settings().clpMinCircleSegmentLength_ = settings.setValue(dsbxMinCircleSegmentLength);
    App::settings().clpMinCircleSegments_ = settings.setValue(sbxMinCircleSegments);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    App::settings().mrkHomeOffset_ = settings.setValue("homeOffset", QPointF(dsbxHomeX->value(), dsbxHomeY->value()));
    App::settings().mrkHomePos_ = settings.setValue(cbxHomePos);
    App::settings().mrkPinOffset_ = settings.setValue("pinOffset", QPointF(dsbxPinX->value(), dsbxPinY->value()));
    App::settings().mrkZeroOffset_ = settings.setValue("zeroOffset", QPointF(dsbxZeroX->value(), dsbxZeroY->value()));
    App::settings().mrkZeroPos_ = settings.setValue(cbxZeroPos);
    settings.endGroup();

    /*Other*/
    settings.setValue(App::settings().inch_, "inch");
    settings.setValue(App::settings().snap_, "snap");
    for (auto tab : tabs)
        tab->writeSettings(settings);
}

void SettingsDialog::readSettingsDialog() {
    settings.beginGroup("SettingsDialog");
    if (auto geometry {settings.value("geometry").toByteArray()}; geometry.size())
        restoreGeometry(geometry);
    settings.getValue(tabwMain);
    settings.endGroup();
}

void SettingsDialog::saveSettingsDialog() {
    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue(tabwMain);
    settings.endGroup();
}

void SettingsDialog::translator(QApplication* app, const QString& path) {
    if (QFile::exists(path)) {
        QTranslator* pTranslator = new QTranslator(qApp);
        if (pTranslator->load(path))
            app->installTranslator(pTranslator);
        else
            delete pTranslator;
    }
}

void SettingsDialog::reject() {
    readSettings();

    if (!isVisible())
        MainWindow::updateTheme();

    QDialog::reject();
}

void SettingsDialog::accept() {
    if (isVisible() && !buttonBox->button(QDialogButtonBox::Ok)->hasFocus())
        return;

    saveSettings();

    if (langIndex != cbxLanguage->currentIndex()) {
        QMessageBox::information(this, "", tr("The complete translation of the application will take\n"
                                              "effect after restarting the application."));
    }
    MainWindow::updateTheme();
    QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent* event) {
    int width = 0;
    for (int i = 0; i < tabwMain->tabBar()->count(); ++i)
        width += tabwMain->tabBar()->tabRect(i).width();
    resize(width + 20, 10);
    button->setMaximumHeight(buttonBox->button(QDialogButtonBox::Ok)->height());

    QDialog::showEvent(event);
}

bool SettingsDialog::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::KeyPress)
        return false;
    return QDialog::eventFilter(watched, event);
}
