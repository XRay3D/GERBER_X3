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
#include "../settings/settings.h"

#include <QDebug>
#include <QObject>
#include <map>

namespace GCode {
class Creator;
class PocketCreator;
class ProfileCreator;
class VoronoiCreator;
class ThermalCreator;
}

class DrillForm;
class FileModel;
class GCodePropertiesForm;
class GraphicsView;
class LayoutFrames;
class MainWindow;
class Project;
class Scene;
class SplashScreen;
class FileTreeView;
class FilePluginInterface;

using ParserInterfaces = std::map<int, std::pair<FilePluginInterface*, QObject*>>;

class App {
    friend class DrillForm;
    friend class FileModel;
    friend class GCode::Creator;
    friend class GCode::PocketCreator;
    friend class GCode::ProfileCreator;
    friend class GCode::ThermalCreator;
    friend class GCode::VoronoiCreator;
    friend class GCodePropertiesForm;
    friend class GraphicsView;
    friend class LayoutFrames;
    friend class MainWindow;
    friend class Project;
    friend class Scene;
    friend class SplashScreen;
    friend class FileTreeView;

    inline static App* m_app = nullptr;

    DrillForm* m_drillForm = nullptr;
    FileModel* m_fileModel = nullptr;
    GCode::Creator* m_creator = nullptr;
    GCodePropertiesForm* m_gCodePropertiesForm = nullptr;
    GraphicsView* m_graphicsView = nullptr;
    LayoutFrames* m_layoutFrames = nullptr;
    MainWindow* m_mainWindow = nullptr;
    Project* m_project = nullptr;
    Scene* m_scene = nullptr;
    SplashScreen* m_splashScreen = nullptr;
    FileTreeView* m_fileTreeView = nullptr;
    ParserInterfaces m_parserInterfaces;
    AppSettings m_appSettings;
    ToolHolder m_toolHolder;

public:
    explicit App()
    {
        if (!m_app)
            m_app = this;
    }
    App(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;
    ~App() { }

    static App* get() { return m_app; }
    static void set(App* app) { m_app = app; }
    static DrillForm* drillForm() { return m_app->m_drillForm; }
    static FileModel* fileModel() { return m_app->m_fileModel; }
    static GCode::Creator* creator() { return m_app->m_creator; }
    static GCodePropertiesForm* gCodePropertiesForm() { return m_app->m_gCodePropertiesForm; }
    static GraphicsView* graphicsView() { return m_app->m_graphicsView; }
    static LayoutFrames* layoutFrames() { return m_app->m_layoutFrames; }
    static MainWindow* mainWindow() { return m_app->m_mainWindow; }
    static Project* project() { return m_app->m_project; }
    static Scene* scene() { return m_app->m_scene; }
    static SplashScreen* splashScreen() { return m_app->m_splashScreen; }
    static FileTreeView* fileTreeView() { return m_app->m_fileTreeView; }
    static FilePluginInterface* parserInterface(int type) { return m_app->m_parserInterfaces.contains(type) ? m_app->m_parserInterfaces[type].first : nullptr; }
    static ParserInterfaces& parserInterfaces() { return m_app->m_parserInterfaces; }

    static AppSettings& settings() { return m_app->m_appSettings; }
    static ToolHolder& toolHolder() { return m_app->m_toolHolder; }
};
