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

class AbstractFile;
class GCodePlugin;

class AbstractFileSettings : public QWidget {
public:
    AbstractFileSettings(QWidget* parent)
        : QWidget(parent) {
    }
    virtual ~AbstractFileSettings() = default;
    virtual void readSettings(MySettings& settings) = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

class AbstractFilePlugin : public QObject, public PluginData {
    Q_OBJECT

public:
    explicit AbstractFilePlugin(QObject* parent = nullptr)
        : QObject {parent} { }
    virtual ~AbstractFilePlugin() = default;

    virtual bool thisIsIt(const QString& fileName) = 0;
    virtual int type() const = 0;
    virtual QString folderName() const = 0;

    [[nodiscard]] virtual std::any createPreviewGi(AbstractFile* file, GCodePlugin* plugin, std::any param = {}) { return {}; };

    [[nodiscard]] virtual AbstractFileSettings* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };
    [[nodiscard]] virtual AbstractFile* loadFile(QDataStream& stream) = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;

    virtual void addToGcForm([[maybe_unused]] AbstractFile* file, [[maybe_unused]] QComboBox* cbx) {};

    virtual void createMainMenu(
        [[maybe_unused]] QMenu& menu,
        [[maybe_unused]] FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close All Files"), [tv] {
            if (QMessageBox::question(tv, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };

    virtual void updateFileModel([[maybe_unused]] AbstractFile* file) {};

signals:
    void fileError(const QString& fileName, const QString& error);
    void fileWarning([[maybe_unused]] const QString& fileName, [[maybe_unused]] const QString& warning);
    void fileProgress(const QString& fileName, int max, int value);
    void fileReady(AbstractFile* file);

public slots:
    virtual AbstractFile* parseFile(const QString& fileName, int type) = 0;
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFilePlugin"

Q_DECLARE_INTERFACE(AbstractFilePlugin, ParserInterface_iid)
