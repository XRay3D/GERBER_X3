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

using DrillPreviewGiMap = std::map<int, mvector<std::shared_ptr<AbstractDrillPrGI>>>;

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
    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;
    [[nodiscard]] virtual bool thisIsIt(const QString& fileName) = 0;
    [[nodiscard]] virtual QObject* getObject() = 0;
    [[nodiscard]] virtual int type() const = 0;
    [[nodiscard]] virtual NodeInterface* createNode(FileInterface* file) = 0;
    [[nodiscard]] virtual std::shared_ptr<FileInterface> createFile() = 0;
    [[nodiscard]] virtual SettingsTab createSettingsTab(QWidget* parent)
    {
        Q_UNUSED(parent)
        return { nullptr, "" };
    };
    [[nodiscard]] virtual DrillPreviewGiMap createtDrillPreviewGi(FileInterface* file, mvector<Row>& data)
    {
        Q_UNUSED(file)
        Q_UNUSED(data)
        return {};
    };
    virtual void addToDrillForm(FileInterface* file, QComboBox* cbx)
    {
        Q_UNUSED(file)
        Q_UNUSED(cbx)
    };

    virtual void setupInterface(App* a) = 0;
    virtual void createMainMenu(QMenu& menu, FileTreeView* tv)
    {
        Q_UNUSED(menu)
        Q_UNUSED(tv)
    };
    virtual void updateFileModel(FileInterface* file)
    {
        Q_UNUSED(file)
    };

    //signals:
    virtual void fileReady(FileInterface* file) = 0;
    virtual void fileProgress(const QString& fileName, int max, int value) = 0;
    virtual void fileError(const QString& fileName, const QString& error) = 0;

protected:
    FileInterface* m_file = nullptr;
    App app;
    enum {
        IconSize = 24,
    };
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.FilePluginInterface"

Q_DECLARE_INTERFACE(FilePluginInterface, ParserInterface_iid)
