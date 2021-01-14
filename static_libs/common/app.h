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
#include <QSharedMemory>
#include <map>
#include <mvector.h>

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
namespace Shapes {
class Handler;
}

using Handlers = mvector<Shapes::Handler*>;

using PIF = std::tuple<FilePluginInterface*, QObject*>;
using PIS = std::tuple<ShapePluginInterface*, QObject*>;
#if __cplusplus > 201709L
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
    Handlers m_handlers;

    App(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;

    QSharedMemory sm { "AppSettings" };

public:
    explicit App()
    {
        if (sm.create(sizeof(nullptr), QSharedMemory::ReadWrite)) {
            m_app = *reinterpret_cast<App**>(sm.data()) = this;
            qDebug() << __FUNCTION__ << "create" << m_app;
        } else if (sm.attach(QSharedMemory::ReadOnly)) {
            m_app = *reinterpret_cast<App**>(sm.data());
            qDebug() << __FUNCTION__ << "attach" << m_app;
        } else {
            qDebug() << __FUNCTION__ << m_app << sm.errorString();
        }
    }
    ~App() { }

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

    static void setDrillForm(DrillForm* drillForm)
    {
        (m_app->m_drillForm && drillForm)
            ? exit(-1)
            : (m_app->m_drillForm = drillForm, void());
    }
    static void setFileModel(FileModel* fileModel)
    {
        (m_app->m_fileModel && fileModel)
            ? exit(-2)
            : (m_app->m_fileModel = fileModel, void());
    }
    static void setFileTreeView(FileTreeView* fileTreeView)
    {
        (m_app->m_fileTreeView && fileTreeView)
            ? exit(-3)
            : (m_app->m_fileTreeView = fileTreeView, void());
    }
    static void setGCodePropertiesForm(GCodePropertiesForm* gCodePropertiesForm)
    {
        (m_app->m_gCodePropertiesForm && gCodePropertiesForm)
            ? exit(-4)
            : (m_app->m_gCodePropertiesForm = gCodePropertiesForm, void());
    }
    static void setGraphicsView(GraphicsView* graphicsView)
    {
        (m_app->m_graphicsView && graphicsView)
            ? exit(-5)
            : (m_app->m_graphicsView = graphicsView, void());
    }
    static void setLayoutFrames(LayoutFrames* layoutFrames)
    {
        (m_app->m_layoutFrames && layoutFrames)
            ? exit(-6)
            : (m_app->m_layoutFrames = layoutFrames, void());
    }
    static void setMainWindow(MainWindow* mainWindow)
    {
        (m_app->m_mainWindow && mainWindow)
            ? exit(-7)
            : (m_app->m_mainWindow = mainWindow, void());
    }
    static void setProject(Project* project)
    {
        (m_app->m_project && project)
            ? exit(-8)
            : (m_app->m_project = project, void());
    }
    static void setScene(Scene* scene)
    {
        (m_app->m_scene && scene)
            ? exit(-9)
            : (m_app->m_scene = scene, void());
    }
    static void setSplashScreen(SplashScreen* splashScreen)
    {
        (m_app->m_splashScreen && splashScreen)
            ? exit(-10)
            : (m_app->m_splashScreen = splashScreen, void());
    }

    static FilePluginInterface* fileInterface(int type) { return m_app->m_fileInterfaces.contains(type)
            ? std::get<FilePluginInterface*>(m_app->m_fileInterfaces[type])
            : nullptr; }
    static FileInterfacesMap& fileInterfaces() { return m_app->m_fileInterfaces; }

    static ShapePluginInterface* shapeInterface(int type) { return m_app->m_shapeInterfaces.contains(type)
            ? std::get<ShapePluginInterface*>(m_app->m_shapeInterfaces[type])
            : nullptr; }
    static ShapeInterfacesMap& shapeInterfaces() { return m_app->m_shapeInterfaces; }
    static Handlers& shapeHandlers() { return m_app->m_handlers; }

    static AppSettings& settings() { return m_app->m_appSettings; }
    static ToolHolder& toolHolder() { return m_app->m_toolHolder; }
};
