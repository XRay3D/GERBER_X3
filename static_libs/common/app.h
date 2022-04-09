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
#include "settings.h"

#include <QDebug>
#include <QObject>
#include <QSharedMemory>
#include <map>
#include <mvector.h>

class DrillForm;
class FilePlugin;
class GCodePlugin;
class GCodePropertiesForm;
class GraphicsView;
class LayoutFrames;
class MainWindow;
class Project;
class Scene;
class ShapePlugin;
class SplashScreen;

namespace FileTree {
class View;
class Model;
} // namespace FileTree

namespace Shapes {
class Handler;
} // namespace Shapes

using Handlers = mvector<Shapes::Handler*>;

union PIF {
    FilePlugin* plug;
    QObject* obj;
};
union PIS {
    ShapePlugin* plug;
    QObject* obj;
};
union PIG {
    GCodePlugin* plug;
    QObject* obj;
};

template <class T>
union Plug {
    Plug(T* plug)
        : plug { plug } { }
    T* plug;
    QObject* obj;
};

#if __cplusplus > 201703L
using FileInterfacesMap = std::map<int, PIF>;
using ShapeInterfacesMap = std::map<int, PIS>;
using GCodeInterfaceMap = std::map<int, PIG>;
#else
struct FileInterfacesMap : std::map<int, PIF> {
    bool contains(int key) const { return find(key) != end(); }
};
struct ShapeInterfacesMap : std::map<int, PIS> {
    bool contains(int key) const { return find(key) != end(); }
};
struct GCodeInterfaceMap : std::map<int, PIG> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif
class App {
    inline static App* app_ = nullptr;

    DrillForm* drillForm_ = nullptr;
    FileTree::Model* fileModel_ = nullptr;
    FileTree::View* fileTreeView_ = nullptr;
    GCodePropertiesForm* gCodePropertiesForm_ = nullptr;
    GraphicsView* graphicsView_ = nullptr;
    LayoutFrames* layoutFrames_ = nullptr;
    MainWindow* mainWindow_ = nullptr;
    Project* project_ = nullptr;
    Scene* scene_ = nullptr;
    SplashScreen* splashScreen_ = nullptr;

    FileInterfacesMap filePlugins_;
    ShapeInterfacesMap shapePlugin_;
    GCodeInterfaceMap gCodePlugin_;

    AppSettings appSettings_;
    ToolHolder toolHolder_;
    Handlers handlers_;
    QSettings settings_;

    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;
    App(App&&) = delete;
    App(const App&) = delete;

    QSharedMemory sharedMemory { "AppSettings" };

public:
    explicit App() {
        if (sharedMemory.create(sizeof(nullptr), QSharedMemory::ReadWrite))
            app_ = *reinterpret_cast<App**>(sharedMemory.data()) = this;
        else if (sharedMemory.attach(QSharedMemory::ReadOnly))
            app_ = *reinterpret_cast<App**>(sharedMemory.data());
        else
            qDebug() << app_ << sharedMemory.errorString();
    }
    ~App() { }

    static DrillForm* drillForm() { return app_->drillForm_; }
    static FileTree::Model* fileModel() { return app_->fileModel_; }
    static FileTree::View* fileTreeView() { return app_->fileTreeView_; }
    static GCodePropertiesForm* gCodePropertiesForm() { return app_->gCodePropertiesForm_; }
    static GraphicsView* graphicsView() { return app_->graphicsView_; }
    static LayoutFrames* layoutFrames() { return app_->layoutFrames_; }
    static MainWindow* mainWindow() { return app_->mainWindow_; }
    static Project* project() { return app_->project_; }
    static Scene* scene() { return app_->scene_; }
    static SplashScreen* splashScreen() { return app_->splashScreen_; }

    static void setDrillForm(DrillForm* drillForm) { (app_->drillForm_ && drillForm) ? exit(-1) : (app_->drillForm_ = drillForm, void()); }
    static void setFileModel(FileTree::Model* fileModel) { (app_->fileModel_ && fileModel) ? exit(-2) : (app_->fileModel_ = fileModel, void()); }
    static void setFileTreeView(FileTree::View* fileTreeView) { (app_->fileTreeView_ && fileTreeView) ? exit(-3) : (app_->fileTreeView_ = fileTreeView, void()); }
    static void setGCodePropertiesForm(GCodePropertiesForm* gCodePropertiesForm) { (app_->gCodePropertiesForm_ && gCodePropertiesForm) ? exit(-4) : (app_->gCodePropertiesForm_ = gCodePropertiesForm, void()); }
    static void setGraphicsView(GraphicsView* graphicsView) { (app_->graphicsView_ && graphicsView) ? exit(-5) : (app_->graphicsView_ = graphicsView, void()); }
    static void setLayoutFrames(LayoutFrames* layoutFrames) { (app_->layoutFrames_ && layoutFrames) ? exit(-6) : (app_->layoutFrames_ = layoutFrames, void()); }
    static void setMainWindow(MainWindow* mainWindow) { (app_->mainWindow_ && mainWindow) ? exit(-7) : (app_->mainWindow_ = mainWindow, void()); }
    static void setProject(Project* project) { (app_->project_ && project) ? exit(-8) : (app_->project_ = project, void()); }
    static void setScene(Scene* scene) { (app_->scene_ && scene) ? exit(-9) : (app_->scene_ = scene, void()); }
    static void setSplashScreen(SplashScreen* splashScreen) { (app_->splashScreen_ && splashScreen) ? exit(-10) : (app_->splashScreen_ = splashScreen, void()); }

    static FilePlugin* filePlugin(int type) {
        return app_->filePlugins_.contains(type) ? app_->filePlugins_[type].plug : nullptr;
    }
    static FileInterfacesMap& filePlugins() { return app_->filePlugins_; }

    static ShapePlugin* shapePlugin(int type) {
        return app_->shapePlugin_.contains(type) ? app_->shapePlugin_[type].plug : nullptr;
    }
    static ShapeInterfacesMap& shapePlugins() { return app_->shapePlugin_; }

    static GCodePlugin* gCodePlugin(int type) {
        return app_->gCodePlugin_.contains(type) ? app_->gCodePlugin_[type].plug : nullptr;
    }
    static GCodeInterfaceMap& gCodePlugins() { return app_->gCodePlugin_; }

    static Handlers& shapeHandlers() { return app_->handlers_; }

    static AppSettings& settings() { return app_->appSettings_; }
    static ToolHolder& toolHolder() { return app_->toolHolder_; }
    static QSettings* qSettings() { return &app_->settings_; }
};
