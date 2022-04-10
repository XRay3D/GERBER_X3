/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "../tooldatabase/tool.h"
#include "app.h"
#include "ft_view.h"
#include "settings.h"

#include "drill/gc_drillmodel.h"

#include <QMenu>
#include <QMessageBox>

#include <memory>

class FileInterface;

class AbstractDrillPrGI;
class AbstractThermPrGi;
class ThParam2;

using DrillPreviewGiMap = std::map<int, mvector<std::shared_ptr<AbstractDrillPrGI>>>;
using ThermalPreviewGiVec = mvector<std::shared_ptr<AbstractThermPrGi>>;

class SettingsTabInterface : public QWidget {
public:
    SettingsTabInterface(QWidget* parent)
        : QWidget(parent) {
    }
    virtual ~SettingsTabInterface() = default;
    virtual void readSettings(MySettings& settings) = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

class FilePlugin {
public:
    explicit FilePlugin() { }
    virtual ~FilePlugin() { }
    virtual QObject* getObject() = 0;
    virtual bool thisIsIt(const QString& fileName) = 0;
    virtual int type() const = 0;
    virtual QString folderName() const = 0;

    [[nodiscard]] virtual DrillPreviewGiMap createDrillPreviewGi(
        [[maybe_unused]] FileInterface* file,
        [[maybe_unused]] mvector<Row>& data) { return {}; };

    [[nodiscard]] virtual ThermalPreviewGiVec createThermalPreviewGi(
        [[maybe_unused]] FileInterface* file,
        [[maybe_unused]] const ThParam2& param,
        [[maybe_unused]] Tool& tool) { return {}; };

    [[nodiscard]] virtual SettingsTabInterface* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };

    [[nodiscard]] virtual FileInterface* createFile() = 0;

    [[nodiscard]] virtual QJsonObject info() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;

    virtual void addToDrillForm(
        [[maybe_unused]] FileInterface* file,
        [[maybe_unused]] QComboBox* cbx) {};

    virtual void createMainMenu(
        [[maybe_unused]] QMenu& menu,
        [[maybe_unused]] FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close All Files"), [tv] {
            if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };

    virtual void updateFileModel([[maybe_unused]] FileInterface* file) {};

    // signals:
    virtual void fileError(const QString& fileName, const QString& error) = 0;
    virtual void fileWarning([[maybe_unused]] const QString& fileName, [[maybe_unused]] const QString& warning) {};
    virtual void fileProgress(const QString& fileName, int max, int value) = 0;
    virtual void fileReady(FileInterface* file) = 0;

    // slots:
    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;

protected:
    App app;
    enum { IconSize = 24 };
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.FilePlugin"

Q_DECLARE_INTERFACE(FilePlugin, ParserInterface_iid)
