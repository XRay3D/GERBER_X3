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

class AbstractFileSettings : public QWidget {
public:
    AbstractFileSettings(QWidget* parent)
        : QWidget{parent} {
    }
    virtual ~AbstractFileSettings()                  = default;
    virtual void readSettings(MySettings& settings)  = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

class AbstractFilePlugin : public QObject, public PluginData {
    Q_OBJECT

public:
    explicit AbstractFilePlugin(QObject* parent = nullptr)
        : QObject{parent} { App{}; } // init global App
    virtual ~AbstractFilePlugin() = default;

    [[nodiscard]] virtual AbstractFileSettings* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };
    [[nodiscard]] virtual QString folderName() const                        = 0;
    [[nodiscard]] virtual QIcon icon() const                                = 0;
    [[nodiscard]] virtual AbstractFile* loadFile(QDataStream& stream) const = 0;
    [[nodiscard]] virtual bool thisIsIt(const QString& fileName)            = 0;
    [[nodiscard]] virtual uint32_t type() const                             = 0;
    [[nodiscard]] virtual QString extension() const { return {}; } // = 0;

    virtual void createMainMenu(
        [[maybe_unused]] QMenu& menu,
        [[maybe_unused]] FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme(u"document-close"_s), tr("&Close All Files"), [tv] {
            if(QMessageBox::question(tv, {}, tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };

    virtual void updateFileModel([[maybe_unused]] AbstractFile* file) { };

    void createProgress(const QString& fileName, int max) { emit fileProgress_(fileName, max, 0); }
    void updateProgressVal(const QString& fileName, int value) { emit fileProgress_(fileName, 0, value); }
    void updateProgressMax(const QString& fileName, int max) { emit fileProgress_(fileName, max, 1); }
    void closeProgress(const QString& fileName) { emit fileProgress_(fileName, 0, 0); }

signals:
    void fileError(const QString& fileName, const QString& error);
    void fileWarning(const QString& fileName, const QString& warning);
    void fileProgress_(const QString& fileName, int max, int value);
    void fileReady(AbstractFile* file);

public slots:
    virtual AbstractFile* parseFile(const QString& fileName, uint32_t type) = 0;
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFilePlugin"

Q_DECLARE_INTERFACE(AbstractFilePlugin, ParserInterface_iid)
