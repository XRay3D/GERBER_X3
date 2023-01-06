/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "ft_view.h"
#include "plugindata.h"
// #include "settings.h"
// #include "app.h"
// #include "myclipper.h"
// #include "tool.h"

#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <any>
#include <memory>

class FileInterface;
class GCodePlugin;

class SettingsTabInterface : public QWidget {
public:
    SettingsTabInterface(QWidget* parent)
        : QWidget(parent) {
    }
    virtual ~SettingsTabInterface() = default;
    virtual void readSettings(MySettings& settings) = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

class FilePlugin : public QObject, public PluginData {
    Q_OBJECT

public:
    explicit FilePlugin(QObject* parent = nullptr)
        : QObject {parent} { }
    virtual ~FilePlugin() = default;

    virtual bool thisIsIt(const QString& fileName) = 0;
    virtual int type() const = 0;
    virtual QString folderName() const = 0;

    [[nodiscard]] virtual std::any createPreviewGi(FileInterface* file, GCodePlugin* plugin, std::any param = {}) { return {}; };

    [[nodiscard]] virtual SettingsTabInterface* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };
    [[nodiscard]] virtual FileInterface* createFile() = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;

    virtual void addToGcForm([[maybe_unused]] FileInterface* file, [[maybe_unused]] QComboBox* cbx) {};

    virtual void createMainMenu(
        [[maybe_unused]] QMenu& menu,
        [[maybe_unused]] FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close All Files"), [tv] {
            if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };

    virtual void updateFileModel([[maybe_unused]] FileInterface* file) {};

signals:
    void fileError(const QString& fileName, const QString& error);
    void fileWarning([[maybe_unused]] const QString& fileName, [[maybe_unused]] const QString& warning);
    void fileProgress(const QString& fileName, int max, int value);
    void fileReady(FileInterface* file);

public slots:
    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.FilePlugin"

Q_DECLARE_INTERFACE(FilePlugin, ParserInterface_iid)
