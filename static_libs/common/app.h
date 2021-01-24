/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
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
#include <QSharedMemory>
#include <map>
#include <mvector.h>

class DrillForm;
class FilePluginInterface;
namespace FileTree {
class View;
class Model;
}
class GCodePropertiesForm;
class GraphicsView;
class LayoutFrames;
class MainWindow;
class Project;
class Scene;
class ShapePluginInterface;
class SplashScreen;
namespace Shapes {
class Handler;
}

using Handlers = mvector<Shapes::Handler*>;

using PIF = std::tuple<FilePluginInterface*, QObject*>;
using PIS = std::tuple<ShapePluginInterface*, QObject*>;
#if _MSVC_LANG >= 201705L
using FileInterfacesMap = std::map<int, PIF>;
using ShapeInterfacesMap = std::map<int, PIS>;
#else
struct FileInterfacesMap : std::map<int, PIF> {
    bool contains(int key) const { return find(key) != end(); }
};
struct ShapeInterfacesMap : std::map<int, PIS> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif
class App {
    inline static App* m_app = nullptr;

    DrillForm* m_drillForm = nullptr;
    FileTree::Model* m_fileModel = nullptr;
    FileTree::View* m_fileTreeView = nullptr;
    GCodePropertiesForm* m_gCodePropertiesForm = nullptr;
    GraphicsView* m_graphicsView = nullptr;
    LayoutFrames* m_layoutFrames = nullptr;
    MainWindow* m_mainWindow = nullptr;
    Project* m_project = nullptr;
    Scene* m_scene = nullptr;
    SplashScreen* m_splashScreen = nullptr;

    FileInterfacesMap m_filePlugins;
    ShapeInterfacesMap m_shapePlugin;

    AppSettings m_appSettings;
    ToolHolder m_toolHolder;
    Handlers m_handlers;

    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;
    App(App&&) = delete;
    App(const App&) = delete;

    QSharedMemory sm { "AppSettings" };

public:
    explicit App()
    {
        if (sm.create(sizeof(nullptr), QSharedMemory::ReadWrite))
            m_app = *reinterpret_cast<App**>(sm.data()) = this;
        else if (sm.attach(QSharedMemory::ReadOnly))
            m_app = *reinterpret_cast<App**>(sm.data());
        else
            qDebug() << m_app << sm.errorString();
    }
    ~App() { }

    static DrillForm* drillForm() { return m_app->m_drillForm; }
    static FileTree::Model* fileModel() { return m_app->m_fileModel; }
    static FileTree::View* fileTreeView() { return m_app->m_fileTreeView; }
    static GCodePropertiesForm* gCodePropertiesForm() { return m_app->m_gCodePropertiesForm; }
    static GraphicsView* graphicsView() { return m_app->m_graphicsView; }
    static LayoutFrames* layoutFrames() { return m_app->m_layoutFrames; }
    static MainWindow* mainWindow() { return m_app->m_mainWindow; }
    static Project* project() { return m_app->m_project; }
    static Scene* scene() { return m_app->m_scene; }
    static SplashScreen* splashScreen() { return m_app->m_splashScreen; }

    static void setDrillForm(DrillForm* drillForm) { (m_app->m_drillForm && drillForm) ? exit(-1) : (m_app->m_drillForm = drillForm, void()); }
    static void setFileModel(FileTree::Model* fileModel) { (m_app->m_fileModel && fileModel) ? exit(-2) : (m_app->m_fileModel = fileModel, void()); }
    static void setFileTreeView(FileTree::View* fileTreeView) { (m_app->m_fileTreeView && fileTreeView) ? exit(-3) : (m_app->m_fileTreeView = fileTreeView, void()); }
    static void setGCodePropertiesForm(GCodePropertiesForm* gCodePropertiesForm) { (m_app->m_gCodePropertiesForm && gCodePropertiesForm) ? exit(-4) : (m_app->m_gCodePropertiesForm = gCodePropertiesForm, void()); }
    static void setGraphicsView(GraphicsView* graphicsView) { (m_app->m_graphicsView && graphicsView) ? exit(-5) : (m_app->m_graphicsView = graphicsView, void()); }
    static void setLayoutFrames(LayoutFrames* layoutFrames) { (m_app->m_layoutFrames && layoutFrames) ? exit(-6) : (m_app->m_layoutFrames = layoutFrames, void()); }
    static void setMainWindow(MainWindow* mainWindow) { (m_app->m_mainWindow && mainWindow) ? exit(-7) : (m_app->m_mainWindow = mainWindow, void()); }
    static void setProject(Project* project) { (m_app->m_project && project) ? exit(-8) : (m_app->m_project = project, void()); }
    static void setScene(Scene* scene) { (m_app->m_scene && scene) ? exit(-9) : (m_app->m_scene = scene, void()); }
    static void setSplashScreen(SplashScreen* splashScreen) { (m_app->m_splashScreen && splashScreen) ? exit(-10) : (m_app->m_splashScreen = splashScreen, void()); }

    static FilePluginInterface* filePlugin(int type)
    {
        return m_app->m_filePlugins.contains(type) ? std::get<FilePluginInterface*>(m_app->m_filePlugins[type])
                                                   : nullptr;
    }
    static FileInterfacesMap& filePlugins() { return m_app->m_filePlugins; }

    static ShapePluginInterface* shapePlugin(int type)
    {
        return m_app->m_shapePlugin.contains(type) ? std::get<ShapePluginInterface*>(m_app->m_shapePlugin[type])
                                                   : nullptr;
    }
    static ShapeInterfacesMap& shapePlugins() { return m_app->m_shapePlugin; }

    static Handlers& shapeHandlers() { return m_app->m_handlers; }

    static AppSettings& settings() { return m_app->m_appSettings; }
    static ToolHolder& toolHolder() { return m_app->m_toolHolder; }
};
