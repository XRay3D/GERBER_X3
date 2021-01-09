/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "../tooldatabase/tool.h"
#include "app.h"
#include "settings.h"

#include "forms/drillform/drillmodel.h"

#include <QObject>
#include <QWidget>
#include <memory>

class FileInterface;
class NodeInterface;

class AbstractDrillPrGI;
class App;
class AppSettings;
class FileModel;
class FileTreeView;
class QMenu;
class ThParam2;
class AbstractThermPrGi;

using DrillPreviewGiMap = std::map<int, mvector<std::shared_ptr<AbstractDrillPrGI>>>;
using ThermalPreviewGiVec = mvector<std::shared_ptr<AbstractThermPrGi>>;

class SettingsTabInterface : public QWidget {
public:
    SettingsTabInterface(QWidget* parent)
        : QWidget(parent)
    {
    }
    virtual ~SettingsTabInterface() = default;
    virtual void readSettings(MySettings& settings) = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

using SettingsTab = std::pair<SettingsTabInterface*, QString>;

class FilePluginInterface {
public:
    explicit FilePluginInterface() { }
    virtual ~FilePluginInterface() { }
    virtual QObject* getObject() = 0;
    virtual bool thisIsIt(const QString& fileName) = 0;
    virtual int type() const = 0;

    [[nodiscard]] virtual DrillPreviewGiMap createDrillPreviewGi(
        [[maybe_unused]] FileInterface* file,
        [[maybe_unused]] mvector<Row>& data) { return {}; };
    [[nodiscard]] virtual ThermalPreviewGiVec createThermalPreviewGi(
        [[maybe_unused]] FileInterface* file,
        [[maybe_unused]] const ThParam2& param,
        [[maybe_unused]] Tool& tool) { return {}; };
    [[nodiscard]] virtual NodeInterface* createNode(FileInterface* file) = 0;
    [[nodiscard]] virtual SettingsTab createSettingsTab([[maybe_unused]] QWidget* parent) { return { nullptr, "" }; };
    [[nodiscard]] virtual std::shared_ptr<FileInterface> createFile() = 0;
    [[nodiscard]] virtual QJsonObject info() const = 0;

    virtual void addToDrillForm([[maybe_unused]] FileInterface* file, [[maybe_unused]] QComboBox* cbx) {};
    virtual void createMainMenu([[maybe_unused]] QMenu& menu, [[maybe_unused]] FileTreeView* tv) {};
    virtual void setupInterface(App* a) = 0;
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

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.FilePluginInterface"

Q_DECLARE_INTERFACE(FilePluginInterface, ParserInterface_iid)
