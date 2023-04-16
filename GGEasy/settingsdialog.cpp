// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

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
// #include "a_pch.h "
#include "settingsdialog.h"
#include "abstract_fileplugin.h"
#include "colorselector.h"
#include "gc_settings.h"
#include "graphicsview.h"
#include "mainwindow.h"

#include <QDesktopServices>
#include <QtWidgets>

const int gridColor = 100;

const QColor defaultColor[GuiColors::Count]{
    QColor(), // Background
    QColor(255, 255, 0, 120), // Pin
    QColor(Qt::gray), // CutArea
    QColor(gridColor, gridColor, gridColor, 50), // Grid1
    QColor(gridColor, gridColor, gridColor, 100), // Grid5
    QColor(gridColor, gridColor, gridColor, 200), // Grid10
    QColor(), // Hole
    QColor(0, 255, 0, 120), // Home
    QColor(Qt::black), // ToolPath
    QColor(255, 0, 0, 120), // Zero
    QColor(Qt::red) // G0
};

const QString colorName[GuiColors::Count]{
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
        : QAbstractListModel{parent} { }

    virtual ~ModelSettings() { }

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& /*parent*/) const override { return data_.size(); }
    int columnCount(const QModelIndex& /*parent*/) const override { return 1; }
    QVariant data(const QModelIndex& index, int role) const override {
        if(role == Qt::DisplayRole)
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
    ui.setupUi(this);

    ui.chbxOpenGl->setEnabled(QOpenGLContext::supportsThreadedOpenGL());

    for(int i = 0; i < GuiColors::Count; ++i) {
        ui.formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(App::settings().guiColor_[i], defaultColor[i], ui.gbxColor));
        ui.formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", ui.gbxColor));
    }

    connect(ui.cbxFontSize, &QComboBox::currentTextChanged, [](const QString& fontSize) {
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegularExpression(R"(font-size:\s*\d+)"), "font-size: " + fontSize));
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
    });

    // Language
    ui.cbxLanguage->addItem("English", "en");
    ui.cbxLanguage->addItem("Русский", "ru");

    settings.beginGroup("MainWindow");
    QString locale(settings.value("locale").toString());
    settings.endGroup();

    for(int i = 0; i < ui.cbxLanguage->count(); ++i) {
        if(ui.cbxLanguage->itemData(i).toString() == locale) {
            ui.cbxLanguage->setCurrentIndex(i);
            langIndex = i;
            break;
        }
    }

    connect(ui.cbxLanguage, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
        const QString locale(ui.cbxLanguage->itemData(index).toString());
        MainWindow::translate(locale);
        settings.beginGroup("MainWindow");
        settings.setValue("locale", locale);
        settings.endGroup();
    });

    ui.labelAPIcon->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(ui.labelAPIcon->size()));

    tabs.reserve(App::filePlugins().size() + 1); // NOTE +1 for GCode

    //    auto model = new ModelSettings(listView);
    {
        auto tab = new GCode::Tab(this);
        tabs.push_back(tab);
        ui.tabwMain->addTab(tab, tab->windowTitle());
        tab->readSettings(settings);
    }

    for(auto& [type, ptr]: App::filePlugins()) {
        auto tab = ptr->createSettingsTab(this);
        if(!tab)
            continue;

        //        model->addWidget(tab);
        //        auto gbx = new QGroupBox(tab->windowTitle(), this);
        //        auto lay = new QHBoxLayout(gbx);
        //        lay->addWidget(tab);
        //        verticalLayout_3->addWidget(gbx);

        tabs.push_back(tab);
        ui.tabwMain->addTab(tab, tab->windowTitle());
        tab->readSettings(settings);
    }

    //    listView->setModel(model);

    readSettings();
    readSettingsDialog();
    if(tab > -1)
        ui.tabwMain->setCurrentIndex(tab);

    { // Open Settings Folder
        button = new QPushButton(tr("Open Settings Folder"), ui.buttonBox);
        button->setIcon(QIcon::fromTheme("folder"));
        button->setMinimumWidth(QFontMetrics(font()).boundingRect(button->text()).width() + 32);
        ui.buttonBox->addButton(button, QDialogButtonBox::ResetRole);
        connect(button, &QPushButton::clicked, [] { QDesktopServices::openUrl(QUrl(App::settingsPath(), QUrl::TolerantMode)); });
    }

    ui.buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme("dialog-cancel"));
}

SettingsDialog::~SettingsDialog() { saveSettingsDialog(); }

void SettingsDialog::readSettings() {
    /*GUI*/
    settings.beginGroup("Viewer");
    settings.getValue(ui.chbxAntialiasing);
    settings.getValue(ui.chbxOpenGl);
    settings.getValue(ui.chbxScaleHZMarkers, App::settings().scaleHZMarkers_);
    settings.getValue(ui.chbxScalePinMarkers, App::settings().scalePinMarkers_);
    settings.getValue(ui.chbxSmoothScSh, App::settings().guiSmoothScSh_);
    settings.getValue(ui.chbxAnimSelection, App::settings().animSelection_);
    settings.getValue(ui.cbxTheme, App::settings().theme_);
    settings.endGroup();

    settings.beginGroup("Color");
    for(int i = 0; i < GuiColors::Count; ++i)
        App::settings().guiColor_[i].setNamedColor(settings.value(QString("%1").arg(i), App::settings().guiColor_[i].name(QColor::HexArgb)).toString());
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet("QWidget {font-size: " + fontSize + "pt}");
    ui.cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    /*Clipper*/
    settings.beginGroup("Clipper");
    settings.getValue(ui.dsbxMinCircleSegmentLength, App::settings().clpMinCircleSegmentLength_);
    settings.getValue(ui.sbxMinCircleSegments, App::settings().clpMinCircleSegments_);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    settings.getValue("homeOffset", App::settings().mrkHomeOffset_);
    settings.getValue("pinOffset", App::settings().mrkPinOffset_, QPointF(6, 6));
    settings.getValue("zeroOffset", App::settings().mrkZeroOffset_);
    ui.dsbxHomeX->setValue(App::settings().mrkHomeOffset_.x());
    ui.dsbxHomeY->setValue(App::settings().mrkHomeOffset_.y());
    ui.dsbxPinX->setValue(App::settings().mrkPinOffset_.x());
    ui.dsbxPinY->setValue(App::settings().mrkPinOffset_.y());
    ui.dsbxZeroX->setValue(App::settings().mrkZeroOffset_.x());
    ui.dsbxZeroY->setValue(App::settings().mrkZeroOffset_.y());
    settings.getValue(ui.cbxHomePos, HomePosition::TopLeft);
    settings.getValue(ui.cbxZeroPos, HomePosition::TopLeft);
    settings.endGroup();
    /*Other*/
    settings.getValue(App::settings().inch_, "inch", false);
    settings.getValue(App::settings().snap_, "snap", false);
    for(auto tab: tabs)
        tab->readSettings(settings);
}

void SettingsDialog::saveSettings() {
    /*GUI*/
    settings.beginGroup("Viewer");
    if(settings.value("chbxOpenGl").toBool() != ui.chbxOpenGl->isChecked()) {
        App::graphicsView().setOpenGL(ui.chbxOpenGl->isChecked());
        App::graphicsView().viewport()->setObjectName("viewport");
        App::graphicsView().setRenderHint(QPainter::Antialiasing, ui.chbxAntialiasing->isChecked());
        settings.setValue(ui.chbxOpenGl);
    }
    if(settings.value("chbxAntialiasing").toBool() != ui.chbxAntialiasing->isChecked()) {
        App::graphicsView().setRenderHint(QPainter::Antialiasing, ui.chbxAntialiasing->isChecked());
        settings.setValue(ui.chbxAntialiasing);
    }
    App::settings().animSelection_ = settings.setValue(ui.chbxAnimSelection);
    App::settings().guiSmoothScSh_ = settings.setValue(ui.chbxSmoothScSh);
    App::settings().scaleHZMarkers_ = settings.setValue(ui.chbxScaleHZMarkers);
    App::settings().scalePinMarkers_ = settings.setValue(ui.chbxScalePinMarkers);
    App::settings().theme_ = settings.setValue(ui.cbxTheme);
    settings.endGroup();

    settings.beginGroup("Color");
    for(int i = 0; i < GuiColors::Count; ++i)
        settings.setValue(QString("%1").arg(i), App::settings().guiColor_[i].name(QColor::HexArgb));
    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", ui.cbxFontSize->currentText());
    settings.endGroup();

    /*Clipper*/
    settings.beginGroup("Clipper");
    App::settings().clpMinCircleSegmentLength_ = settings.setValue(ui.dsbxMinCircleSegmentLength);
    App::settings().clpMinCircleSegments_ = settings.setValue(ui.sbxMinCircleSegments);
    settings.endGroup();

    /*Markers*/
    settings.beginGroup("Home");
    App::settings().mrkHomeOffset_ = settings.setValue("homeOffset", QPointF(ui.dsbxHomeX->value(), ui.dsbxHomeY->value()));
    App::settings().mrkHomePos_ = settings.setValue(ui.cbxHomePos);
    App::settings().mrkPinOffset_ = settings.setValue("pinOffset", QPointF(ui.dsbxPinX->value(), ui.dsbxPinY->value()));
    App::settings().mrkZeroOffset_ = settings.setValue("zeroOffset", QPointF(ui.dsbxZeroX->value(), ui.dsbxZeroY->value()));
    App::settings().mrkZeroPos_ = settings.setValue(ui.cbxZeroPos);
    settings.endGroup();

    /*Other*/
    settings.setValue(App::settings().inch_, "inch");
    settings.setValue(App::settings().snap_, "snap");
    for(auto tab: tabs)
        tab->writeSettings(settings);
}

void SettingsDialog::readSettingsDialog() {
    settings.beginGroup("SettingsDialog");
    if(auto geometry{settings.value("geometry").toByteArray()}; geometry.size())
        restoreGeometry(geometry);
    settings.getValue(ui.tabwMain);
    settings.endGroup();
}

void SettingsDialog::saveSettingsDialog() {
    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.setValue(ui.tabwMain);
    settings.endGroup();
}

void SettingsDialog::translator(QApplication* app, const QString& path) {
    if(QFile::exists(path)) {
        QTranslator* pTranslator = new QTranslator(qApp);
        if(pTranslator->load(path))
            app->installTranslator(pTranslator);
        else
            delete pTranslator;
    }
}

void SettingsDialog::reject() {
    readSettings();

    if(!isVisible())
        MainWindow::updateTheme();

    QDialog::reject();
}

void SettingsDialog::accept() {
    if(isVisible() && !ui.buttonBox->button(QDialogButtonBox::Ok)->hasFocus())
        return;

    saveSettings();

    if(langIndex != ui.cbxLanguage->currentIndex())
        QMessageBox::information(this, "", tr("The complete translation of the application will take\n"
                                              "effect after restarting the application."));
    MainWindow::updateTheme();
    QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent* event) {
    int width = 0;
    for(int i = 0; i < ui.tabwMain->tabBar()->count(); ++i)
        width += ui.tabwMain->tabBar()->tabRect(i).width();
    resize(width + 20, 10);
    button->setMaximumHeight(ui.buttonBox->button(QDialogButtonBox::Ok)->height());

    QDialog::showEvent(event);
}

bool SettingsDialog::eventFilter(QObject* watched, QEvent* event) {
    if(event->type() == QEvent::KeyPress)
        return false;
    return QDialog::eventFilter(watched, event);
}

void SettingsDialog::Ui::setupUi(QDialog* SettingsDialog) {
    if(SettingsDialog->objectName().isEmpty())
        SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
    SettingsDialog->resize(392, 517);
    gridLayout_2 = new QGridLayout(SettingsDialog);
    gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
    gridLayout_2->setContentsMargins(6, 6, 6, 6);
    buttonBox = new QDialogButtonBox(SettingsDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    gridLayout_2->addWidget(buttonBox, 1, 1, 1, 1);

    tabwMain = new QTabWidget(SettingsDialog);
    tabwMain->setObjectName(QString::fromUtf8("tabwMain"));
    tabGui = new QWidget();
    tabGui->setObjectName(QString::fromUtf8("tabGui"));
    verticalLayout_8 = new QVBoxLayout(tabGui);
    verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
    verticalLayout_8->setContentsMargins(6, 6, 6, 6);
    groupBox = new QGroupBox(tabGui);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout = new QGridLayout(groupBox);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setContentsMargins(6, 9, 6, 6);

    cbxFontSize = new QComboBox(groupBox);
    cbxFontSize->addItems({"7", "8", "9", "10", "11", "12", "13", "14"});
    cbxFontSize->setObjectName(QString::fromUtf8("cbxFontSize"));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(cbxFontSize->sizePolicy().hasHeightForWidth());
    cbxFontSize->setSizePolicy(sizePolicy);

    gridLayout->addWidget(cbxFontSize, 0, 1, 1, 1);

    cbxLanguage = new QComboBox(groupBox);
    cbxLanguage->setObjectName(QString::fromUtf8("cbxLanguage"));

    gridLayout->addWidget(cbxLanguage, 1, 1, 1, 1);

    label_17 = new QLabel(groupBox);
    label_17->setObjectName(QString::fromUtf8("label_17"));
    label_17->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout->addWidget(label_17, 1, 0, 1, 1);

    fontSizeLabel = new QLabel(groupBox);
    fontSizeLabel->setObjectName(QString::fromUtf8("fontSizeLabel"));
    fontSizeLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout->addWidget(fontSizeLabel, 0, 0, 1, 1);

    cbxTheme = new QComboBox(groupBox);
    cbxTheme->setObjectName(QString::fromUtf8("cbxTheme"));

    gridLayout->addWidget(cbxTheme, 2, 1, 1, 1);

    label = new QLabel(groupBox);
    label->setObjectName(QString::fromUtf8("label"));
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout->addWidget(label, 2, 0, 1, 1);

    verticalLayout_8->addWidget(groupBox);

    gbViewer = new QGroupBox(tabGui);
    gbViewer->setObjectName(QString::fromUtf8("gbViewer"));
    verticalLayout_2 = new QVBoxLayout(gbViewer);
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    verticalLayout_2->setContentsMargins(6, 9, 6, 6);
    chbxOpenGl = new QCheckBox(gbViewer);
    chbxOpenGl->setObjectName(QString::fromUtf8("chbxOpenGl"));

    verticalLayout_2->addWidget(chbxOpenGl);

    chbxAntialiasing = new QCheckBox(gbViewer);
    chbxAntialiasing->setObjectName(QString::fromUtf8("chbxAntialiasing"));

    verticalLayout_2->addWidget(chbxAntialiasing);

    chbxSmoothScSh = new QCheckBox(gbViewer);
    chbxSmoothScSh->setObjectName(QString::fromUtf8("chbxSmoothScSh"));

    verticalLayout_2->addWidget(chbxSmoothScSh);

    chbxAnimSelection = new QCheckBox(gbViewer);
    chbxAnimSelection->setObjectName(QString::fromUtf8("chbxAnimSelection"));

    verticalLayout_2->addWidget(chbxAnimSelection);

    gbxColor = new QGroupBox(gbViewer);
    gbxColor->setObjectName(QString::fromUtf8("gbxColor"));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(gbxColor->sizePolicy().hasHeightForWidth());
    gbxColor->setSizePolicy(sizePolicy1);
    formLayout = new QFormLayout(gbxColor);
    formLayout->setObjectName(QString::fromUtf8("formLayout"));
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(6, 9, 6, 6);

    verticalLayout_2->addWidget(gbxColor);

    verticalLayout_8->addWidget(gbViewer);

    tabwMain->addTab(tabGui, QString());
    tabUtils = new QWidget();
    tabUtils->setObjectName(QString::fromUtf8("tabUtils"));
    verticalLayout = new QVBoxLayout(tabUtils);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    groupBox_5 = new QGroupBox(tabUtils);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    gridLayout_3 = new QGridLayout(groupBox_5);
    gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
    gridLayout_3->setContentsMargins(6, 9, 6, 6);
    label_6 = new QLabel(groupBox_5);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    gridLayout_3->addWidget(label_6, 0, 1, 1, 1);

    label_8 = new QLabel(groupBox_5);
    label_8->setObjectName(QString::fromUtf8("label_8"));
    label_8->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    gridLayout_3->addWidget(label_8, 0, 2, 1, 1);

    label_9 = new QLabel(groupBox_5);
    label_9->setObjectName(QString::fromUtf8("label_9"));
    label_9->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    gridLayout_3->addWidget(label_9, 0, 3, 1, 1);

    label_7 = new QLabel(groupBox_5);
    label_7->setObjectName(QString::fromUtf8("label_7"));
    label_7->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout_3->addWidget(label_7, 1, 0, 1, 1);

    dsbxZeroX = new DoubleSpinBox(groupBox_5);
    dsbxZeroX->setObjectName(QString::fromUtf8("dsbxZeroX"));
    dsbxZeroX->setMinimumSize(QSize(100, 0));
    dsbxZeroX->setDecimals(3);
    dsbxZeroX->setMinimum(-1000.000000000000000);
    dsbxZeroX->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxZeroX, 1, 1, 1, 1);

    dsbxZeroY = new DoubleSpinBox(groupBox_5);
    dsbxZeroY->setObjectName(QString::fromUtf8("dsbxZeroY"));
    dsbxZeroY->setMinimumSize(QSize(100, 0));
    dsbxZeroY->setDecimals(3);
    dsbxZeroY->setMinimum(-1000.000000000000000);
    dsbxZeroY->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxZeroY, 1, 2, 1, 1);

    cbxZeroPos = new QComboBox(groupBox_5);
    cbxZeroPos->setObjectName(QString::fromUtf8("cbxZeroPos"));
    sizePolicy.setHeightForWidth(cbxZeroPos->sizePolicy().hasHeightForWidth());
    cbxZeroPos->setSizePolicy(sizePolicy);

    gridLayout_3->addWidget(cbxZeroPos, 1, 3, 1, 1);

    label_4 = new QLabel(groupBox_5);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout_3->addWidget(label_4, 2, 0, 1, 1);

    dsbxHomeX = new DoubleSpinBox(groupBox_5);
    dsbxHomeX->setObjectName(QString::fromUtf8("dsbxHomeX"));
    dsbxHomeX->setMinimumSize(QSize(100, 0));
    dsbxHomeX->setDecimals(3);
    dsbxHomeX->setMinimum(-1000.000000000000000);
    dsbxHomeX->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxHomeX, 2, 1, 1, 1);

    dsbxHomeY = new DoubleSpinBox(groupBox_5);
    dsbxHomeY->setObjectName(QString::fromUtf8("dsbxHomeY"));
    dsbxHomeY->setMinimumSize(QSize(100, 0));
    dsbxHomeY->setDecimals(3);
    dsbxHomeY->setMinimum(-1000.000000000000000);
    dsbxHomeY->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxHomeY, 2, 2, 1, 1);

    cbxHomePos = new QComboBox(groupBox_5);
    cbxHomePos->setObjectName(QString::fromUtf8("cbxHomePos"));
    sizePolicy.setHeightForWidth(cbxHomePos->sizePolicy().hasHeightForWidth());
    cbxHomePos->setSizePolicy(sizePolicy);

    gridLayout_3->addWidget(cbxHomePos, 2, 3, 1, 1);

    label_5 = new QLabel(groupBox_5);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    gridLayout_3->addWidget(label_5, 3, 0, 1, 1);

    dsbxPinX = new DoubleSpinBox(groupBox_5);
    dsbxPinX->setObjectName(QString::fromUtf8("dsbxPinX"));
    dsbxPinX->setDecimals(3);
    dsbxPinX->setMinimum(-1000.000000000000000);
    dsbxPinX->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxPinX, 3, 1, 1, 1);

    dsbxPinY = new DoubleSpinBox(groupBox_5);
    dsbxPinY->setObjectName(QString::fromUtf8("dsbxPinY"));
    dsbxPinY->setDecimals(3);
    dsbxPinY->setMinimum(-1000.000000000000000);
    dsbxPinY->setMaximum(1000.000000000000000);

    gridLayout_3->addWidget(dsbxPinY, 3, 2, 1, 1);

    labelAPIcon = new QLabel(groupBox_5);
    labelAPIcon->setObjectName(QString::fromUtf8("labelAPIcon"));

    gridLayout_3->addWidget(labelAPIcon, 0, 0, 1, 1);

    verticalLayout->addWidget(groupBox_5);

    chbxScaleHZMarkers = new QCheckBox(tabUtils);
    chbxScaleHZMarkers->setObjectName(QString::fromUtf8("chbxScaleHZMarkers"));

    verticalLayout->addWidget(chbxScaleHZMarkers);

    chbxScalePinMarkers = new QCheckBox(tabUtils);
    chbxScalePinMarkers->setObjectName(QString::fromUtf8("chbxScalePinMarkers"));

    verticalLayout->addWidget(chbxScalePinMarkers);

    groupBox_4 = new QGroupBox(tabUtils);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
    formLayout_3 = new QFormLayout(groupBox_4);
    formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
    formLayout_3->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout_3->setContentsMargins(6, 9, 6, 6);
    minimumCircleSegmentsLabel = new QLabel(groupBox_4);
    minimumCircleSegmentsLabel->setObjectName(QString::fromUtf8("minimumCircleSegmentsLabel"));

    formLayout_3->setWidget(0, QFormLayout::LabelRole, minimumCircleSegmentsLabel);

    sbxMinCircleSegments = new QSpinBox(groupBox_4);
    sbxMinCircleSegments->setObjectName(QString::fromUtf8("sbxMinCircleSegments"));
    sbxMinCircleSegments->setMinimum(12);
    sbxMinCircleSegments->setMaximum(360);

    formLayout_3->setWidget(0, QFormLayout::FieldRole, sbxMinCircleSegments);

    minimumCircleSegmentLengthLabel = new QLabel(groupBox_4);
    minimumCircleSegmentLengthLabel->setObjectName(QString::fromUtf8("minimumCircleSegmentLengthLabel"));

    formLayout_3->setWidget(1, QFormLayout::LabelRole, minimumCircleSegmentLengthLabel);

    dsbxMinCircleSegmentLength = new DoubleSpinBox(groupBox_4);
    dsbxMinCircleSegmentLength->setObjectName(QString::fromUtf8("dsbxMinCircleSegmentLength"));
    dsbxMinCircleSegmentLength->setDecimals(2);
    dsbxMinCircleSegmentLength->setMinimum(0.010000000000000);
    dsbxMinCircleSegmentLength->setMaximum(10.000000000000000);
    dsbxMinCircleSegmentLength->setSingleStep(0.010000000000000);
    dsbxMinCircleSegmentLength->setValue(0.500000000000000);

    formLayout_3->setWidget(1, QFormLayout::FieldRole, dsbxMinCircleSegmentLength);

    verticalLayout->addWidget(groupBox_4);

    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer_2);

    tabwMain->addTab(tabUtils, QString());

    gridLayout_2->addWidget(tabwMain, 0, 0, 1, 2);

    retranslateUi(SettingsDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), SettingsDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), SettingsDialog, SLOT(reject()));

    tabwMain->setCurrentIndex(1);

    QMetaObject::connectSlotsByName(SettingsDialog);
}

void SettingsDialog::Ui::retranslateUi(QDialog* SettingsDialog) {
    SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "Settings[*]", nullptr));
    groupBox->setTitle(QCoreApplication::translate("SettingsDialog", "UI", nullptr));
    label_17->setText(QCoreApplication::translate("SettingsDialog", "Language:", nullptr));
    fontSizeLabel->setText(QCoreApplication::translate("SettingsDialog", "Font Size:", nullptr));
    cbxTheme->clear();
    cbxTheme->addItems({
        QCoreApplication::translate("SettingsDialog", "System", nullptr),
        QCoreApplication::translate("SettingsDialog", "Light Blue", nullptr),
        QCoreApplication::translate("SettingsDialog", "Light Red", nullptr),
        QCoreApplication::translate("SettingsDialog", "Dark Blue", nullptr),
        QCoreApplication::translate("SettingsDialog", "Dark Red", nullptr),
    });
    label->setText(QCoreApplication::translate("SettingsDialog", "Theme:", nullptr));
    gbViewer->setTitle(QCoreApplication::translate("SettingsDialog", "Viewer", nullptr));
    chbxOpenGl->setText(QCoreApplication::translate("SettingsDialog", "Open GL", nullptr));
    chbxAntialiasing->setText(QCoreApplication::translate("SettingsDialog", "Anti aliasing", nullptr));
    chbxSmoothScSh->setText(QCoreApplication::translate("SettingsDialog", "Smooth scaling / shearing", nullptr));
    chbxAnimSelection->setText(QCoreApplication::translate("SettingsDialog", "Animated selection for paths", nullptr));
    gbxColor->setTitle(QCoreApplication::translate("SettingsDialog", "Colors", nullptr));
    tabwMain->setTabText(tabwMain->indexOf(tabGui), QCoreApplication::translate("SettingsDialog", "UI", nullptr));
    groupBox_5->setTitle(QCoreApplication::translate("SettingsDialog", "Auto-placement for pins and markers", nullptr));
    label_6->setText(QCoreApplication::translate("SettingsDialog", "X offset", nullptr));
    label_8->setText(QCoreApplication::translate("SettingsDialog", "Y offset", nullptr));
    label_9->setText(QCoreApplication::translate("SettingsDialog", "Place", nullptr));
    label_7->setText(QCoreApplication::translate("SettingsDialog", "Zero:", nullptr));
    dsbxZeroX->setPrefix(QString());
    dsbxZeroX->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    dsbxZeroY->setPrefix(QString());
    dsbxZeroY->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));

    QStringList list{
        QCoreApplication::translate("SettingsDialog", "Top Left", nullptr),
        QCoreApplication::translate("SettingsDialog", "Top Right", nullptr),
        QCoreApplication::translate("SettingsDialog", "Bottom Left", nullptr),
        QCoreApplication::translate("SettingsDialog", "Bottom Right", nullptr),
        QCoreApplication::translate("SettingsDialog", "Center", nullptr),
        QCoreApplication::translate("SettingsDialog", "Always Zero", nullptr),
    };

    cbxZeroPos->clear();
    cbxZeroPos->addItems(list);
    label_4->setText(QCoreApplication::translate("SettingsDialog", "Home:", nullptr));
    dsbxHomeX->setPrefix(QString());
    dsbxHomeX->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    dsbxHomeY->setPrefix(QString());
    dsbxHomeY->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    cbxHomePos->clear();
    cbxHomePos->addItems(list);
    label_5->setText(QCoreApplication::translate("SettingsDialog", "Pins:", nullptr));
    dsbxPinX->setPrefix(QString());
    dsbxPinX->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    dsbxPinY->setPrefix(QString());
    dsbxPinY->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    labelAPIcon->setText(QString());
    chbxScaleHZMarkers->setText(QCoreApplication::translate("SettingsDialog", "Scale Home/Zero Markers", nullptr));
    chbxScalePinMarkers->setText(QCoreApplication::translate("SettingsDialog", "Scale Pin Markers", nullptr));
    groupBox_4->setTitle(QCoreApplication::translate("SettingsDialog", "Clipper2Lib arcs aprox", nullptr));
    minimumCircleSegmentsLabel->setText(QCoreApplication::translate("SettingsDialog", "Minimum points of circle aproximation:", nullptr));
    minimumCircleSegmentLengthLabel->setText(QCoreApplication::translate("SettingsDialog", "The minimum length of circle aproximation:", nullptr));
    dsbxMinCircleSegmentLength->setSuffix(QCoreApplication::translate("SettingsDialog", " mm", nullptr));
    tabwMain->setTabText(tabwMain->indexOf(tabUtils), QCoreApplication::translate("SettingsDialog", "Utils", nullptr));
}

#include "moc_settingsdialog.cpp"
