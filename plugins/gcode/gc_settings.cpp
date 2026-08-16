/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_settings.h"
#include "gc_file.h"
#include "gc_plugin.h"
#include "gc_post.h"
#include "gc_types.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>

namespace GCode {

//------------------------------------------------------------------------------
// PluginTab

QString PluginTab::defaultScriptPath(const QString& gcName) {
    return QCoreApplication::applicationDirPath() + u"/scripts/"_s + gcName.toLower() + u".js"_s;
}

PluginTab::PluginTab(Plugin* plugin, QWidget* parent)
    : AbstractFileSettings{parent}
    , gcName_{plugin->gcName()} {
    setWindowTitle(gcName_);

    auto vLayout = new QVBoxLayout{this};
    vLayout->setContentsMargins(6, 6, 6, 6);

    vLayout->addWidget(new QLabel{QApplication::translate("GCodeSettings", "G-code generation script:", nullptr), this});

    auto hLayout = new QHBoxLayout;
    leScript_ = new QLineEdit{this};
    leScript_->setObjectName(u"script_"_s + gcName_);
    leScript_->setPlaceholderText(defaultScriptPath(gcName_));
    hLayout->addWidget(leScript_);

    auto btnBrowse = new QPushButton{QApplication::translate("GCodeSettings", "Browse...", nullptr), this};
    QObject::connect(btnBrowse, &QPushButton::clicked, leScript_, [this] {
        QString path = QFileDialog::getOpenFileName(
            this,
            QObject::tr("Select Script"),
            leScript_->text().isEmpty() ? defaultScriptPath(gcName_) : leScript_->text(),
            QObject::tr("JavaScript (*.js);;All files (*)"));
        if(!path.isEmpty()) leScript_->setText(path);
    });
    hLayout->addWidget(btnBrowse);

    auto btnDefault = new QPushButton{QApplication::translate("GCodeSettings", "Default", nullptr), this};
    QObject::connect(btnDefault, &QPushButton::clicked, leScript_, [this] {
        leScript_->setText(defaultScriptPath(gcName_));
    });
    hLayout->addWidget(btnDefault);
    vLayout->addLayout(hLayout);

    if(inner_ = plugin->createSettingsTab(this); inner_)
        vLayout->addWidget(inner_);
    vLayout->addStretch();
}

void PluginTab::readSettings(MySettings& settings) {
    settings.beginGroup(u"ScriptPaths"_s);
    QString path = settings.value(leScript_->objectName(), QString{}).toString();
    settings.endGroup();
    if(path.isEmpty()) path = defaultScriptPath(gcName_);
    leScript_->setText(path);
    App::gcSettings().scriptPaths_[gcName_] = path;
    if(inner_) inner_->readSettings(settings);
}

void PluginTab::writeSettings(MySettings& settings) {
    settings.beginGroup(u"ScriptPaths"_s);
    QString path = settings.setValue(leScript_);
    settings.endGroup();
    if(path.isEmpty()) path = defaultScriptPath(gcName_);
    App::gcSettings().scriptPaths_[gcName_] = path;
    if(inner_) inner_->writeSettings(settings);
}

//------------------------------------------------------------------------------
// Tab

Tab::Tab(QWidget* parent)
    : AbstractFileSettings{parent} {
    setWindowTitle(u"G-Code"_s);
    setObjectName(u"tabGCode"_s);

    tabWidget = new QTabWidget{this};
    { // рамещение tabWidget
        auto vLayout = new QVBoxLayout{this};
        vLayout->setContentsMargins(6, 6, 6, 6);
        vLayout->addWidget(tabWidget);
    }

    auto tabCommon = new QWidget{tabWidget};
    tabWidget->addTab(tabCommon, QApplication::translate("GCodeSettings", "Common Settings", nullptr));

    auto vLayout = new QVBoxLayout{tabCommon};
    vLayout->setContentsMargins(6, 6, 6, 6);

    // Постпроцессор: диалект станка -- файл scripts/posts/<id>.js.
    vLayout->addWidget(new QLabel{QApplication::translate("GCodeSettings", "Postprocessor (CNC controller dialect):", nullptr), tabCommon});
    {
        auto hLayout = new QHBoxLayout;
        cbxPost = new QComboBox{tabCommon};
        cbxPost->setObjectName(u"cbxPost"_s);
        hLayout->addWidget(cbxPost, 1);
        auto btnFolder = new QPushButton{QApplication::translate("GCodeSettings", "Open scripts folder", nullptr), tabCommon};
        QObject::connect(btnFolder, &QPushButton::clicked, tabCommon, [] {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + u"/scripts"_s));
        });
        hLayout->addWidget(btnFolder);
        vLayout->addLayout(hLayout);
    }
    lblPostDescription = new QLabel{tabCommon};
    lblPostDescription->setWordWrap(true);
    lblPostDescription->setTextInteractionFlags(Qt::TextSelectableByMouse);
    vLayout->addWidget(lblPostDescription);
    QObject::connect(cbxPost, &QComboBox::currentIndexChanged, tabCommon, [this] { showPostDescription(); });

    // Папка
    chbxSameGFolder = new QCheckBox{tabCommon};
    chbxSameGFolder->setObjectName(u"chbxSameGFolder"_s);
    chbxSameGFolder->setText(QApplication::translate("GCodeSettings", "Save the G-Code to the project folder.", nullptr));
    vLayout->addWidget(chbxSameGFolder);

    // Инфо в УП
    chbxInfo = new QCheckBox{tabCommon};
    chbxInfo->setObjectName(u"chbxInfo"_s);
    chbxInfo->setText(QApplication::translate("GCodeSettings", "Add a comment with the parameters G-Code", nullptr));
    vLayout->addWidget(chbxInfo);
    vLayout->addStretch();

    // Вкладка на каждый gcode-плагин: скрипт + собственные настройки.
    for(auto& [type, ptr]: App::gCodePlugins()) {
        if(ptr->gcName().isEmpty()) continue;
        auto tab = new PluginTab{ptr, tabWidget};
        pluginTabs_.push_back(tab);
        tabWidget->addTab(tab, tab->windowTitle());
    }
}

Tab::~Tab() { }

void Tab::fillPosts() {
    const QString current = cbxPost->currentData().toString();
    QSignalBlocker blocker{cbxPost};
    cbxPost->clear();
    for(const auto& info: Post::available())
        cbxPost->addItem(info.name, info.id);
    if(int idx = cbxPost->findData(current); idx >= 0) cbxPost->setCurrentIndex(idx);
}

void Tab::showPostDescription() {
    // Описание -- из самого поста: грузим его отдельно, текущий (в
    // App::gcSettings()) не трогаем, пока не нажат OK.
    Post post;
    post.load(cbxPost->currentData().toString());
    lblPostDescription->setText(post.description().isEmpty()
            ? QApplication::translate("GCodeSettings", "File extension: .%1", nullptr).arg(post.extension())
            : post.description() + u"\n"_s + QApplication::translate("GCodeSettings", "File extension: .%1", nullptr).arg(post.extension()));
}

void Tab::readSettings(MySettings& settings) {
    settings.beginGroup(u"GCode"_s);
    App::gcSettings().info_ = settings.getValue(chbxInfo, App::gcSettings().info_);
    App::gcSettings().sameFolder_ = settings.getValue(chbxSameGFolder, App::gcSettings().sameFolder_);

    // Ensure scripts and posts are on disk before listing them
    File::ensureDefaultScripts();

    App::gcSettings().postId_ = settings.value(u"Post"_s, App::gcSettings().postId_).toString();
    fillPosts();
    if(int idx = cbxPost->findData(App::gcSettings().postId_); idx >= 0)
        cbxPost->setCurrentIndex(idx);
    else if(cbxPost->count()) {
        cbxPost->setCurrentIndex(0);
        App::gcSettings().postId_ = cbxPost->currentData().toString();
    }
    showPostDescription();
    App::gcSettings().post();

    for(auto tab: pluginTabs_)
        tab->readSettings(settings);

    settings.endGroup();
}

void Tab::writeSettings(MySettings& settings) {
    settings.beginGroup(u"GCode"_s);
    App::gcSettings().info_ = settings.setValue(chbxInfo);
    App::gcSettings().sameFolder_ = settings.setValue(chbxSameGFolder);

    App::gcSettings().postId_ = cbxPost->currentData().toString();
    settings.QSettings::setValue(u"Post"_s, App::gcSettings().postId_);
    App::gcSettings().post();

    for(auto tab: pluginTabs_)
        tab->writeSettings(settings);

    settings.endGroup();
}

} // namespace GCode
