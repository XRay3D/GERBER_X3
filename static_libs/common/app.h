/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "../tooldatabase/tool.h"
#include "settings.h"

#include <QDebug>
#include <QObject>
#include <map>

class DrillForm;
class FileModel;
class FilePluginInterface;
class FileTreeView;
class GCodePropertiesForm;
class GraphicsView;
class LayoutFrames;
class MainWindow;
class Project;
class Scene;
class ShapePluginInterface;
class SplashScreen;

using PIF = std::tuple<FilePluginInterface*, QObject*>;
using PIS = std::tuple<ShapePluginInterface*, QObject*>;
#if __cplusplus > 201709L
using FileInterfacesMap = std::map<int, PI>;
using ShapeInterfacesMap = std::map<int, PI>;
#else
struct FileInterfacesMap : std::map<int, PIF> {
    bool contains(int key) const { return find(key) != end(); }
};
struct ShapeInterfacesMap : std::map<int, PIS> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif
class App {
    friend class DrillForm;
    friend class FileModel;
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
    GCodePropertiesForm* m_gCodePropertiesForm = nullptr;
    GraphicsView* m_graphicsView = nullptr;
    LayoutFrames* m_layoutFrames = nullptr;
    MainWindow* m_mainWindow = nullptr;
    Project* m_project = nullptr;
    Scene* m_scene = nullptr;
    SplashScreen* m_splashScreen = nullptr;
    FileTreeView* m_fileTreeView = nullptr;

    FileInterfacesMap m_fileInterfaces;
    ShapeInterfacesMap m_shapeInterfaces;

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
    static GCodePropertiesForm* gCodePropertiesForm() { return m_app->m_gCodePropertiesForm; }
    static GraphicsView* graphicsView() { return m_app->m_graphicsView; }
    static LayoutFrames* layoutFrames() { return m_app->m_layoutFrames; }
    static MainWindow* mainWindow() { return m_app->m_mainWindow; }
    static Project* project() { return m_app->m_project; }
    static Scene* scene() { return m_app->m_scene; }
    static SplashScreen* splashScreen() { return m_app->m_splashScreen; }
    static FileTreeView* fileTreeView() { return m_app->m_fileTreeView; }

    static FilePluginInterface* fileInterface(int type) { return m_app->m_fileInterfaces.contains(type)
            ? std::get<FilePluginInterface*>(m_app->m_fileInterfaces[type])
            : nullptr; }
    static FileInterfacesMap& fileInterfaces() { return m_app->m_fileInterfaces; }

    static ShapePluginInterface* shapeInterface(int type) { return m_app->m_shapeInterfaces.contains(type)
            ? std::get<ShapePluginInterface*>(m_app->m_shapeInterfaces[type])
            : nullptr; }
    static ShapeInterfacesMap& shapeInterfaces() { return m_app->m_shapeInterfaces; }

    static AppSettings& settings() { return m_app->m_appSettings; }
    static ToolHolder& toolHolder() { return m_app->m_toolHolder; }
};
