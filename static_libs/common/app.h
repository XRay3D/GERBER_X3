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

#include "settings.h"
#include "tool.h"
#include "mvector.h"

#include <QDebug>
#include <QObject>
#include <QSharedMemory>

#include <assert.h>
#include <map>

class GCodePlugin;

class FilePlugin;
namespace Shapes {
class Plugin;
class Handle;
} // namespace Shapes

namespace FileTree {
class View;
class Model;
} // namespace FileTree

namespace DrillPlugin {
class Form;
} // namespace DrillPlugin

using Handlers = mvector<Shapes::Handle*>;

using FileInterfacesMap = std::map<int, FilePlugin*>;
using ShapeInterfacesMap = std::map<int, Shapes::Plugin*>;
using GCodeInterfaceMap = std::map<int, GCodePlugin*>;

#define HOLDER(TYPE, SET, NAME, EXIT_CODE)                                          \
private:                                                                            \
    class TYPE* NAME##_ = nullptr;                                                  \
                                                                                    \
public:                                                                             \
    static auto* NAME() { return app_->NAME##_; }                                   \
    static void set##SET(TYPE* NAME) {                                              \
        (app_->NAME##_ && NAME) ? exit(EXIT_CODE) : (app_->NAME##_ = NAME, void()); \
    }

#define HOLDER2(TYPE, NAME, EXIT_CODE)                                              \
private:                                                                            \
    class TYPE* NAME##_ = nullptr;                                                  \
                                                                                    \
public:                                                                             \
    static auto* NAME() { return app_->NAME##_; }                                   \
    static void set##TYPE(TYPE* NAME) {                                             \
        (app_->NAME##_ && NAME) ? exit(EXIT_CODE) : (app_->NAME##_ = NAME, void()); \
    }

class App {
    inline static App* app_ = nullptr;

    HOLDER(DrillPlugin::Form, DrillForm, drillForm, -10)
    HOLDER(FileTree::Model, FileModel, fileModel, -11)
    HOLDER(FileTree::View, FileTreeView, fileTreeView, -12)
    HOLDER(GCodePropertiesForm, GCodePropertiesForm, gCodePropertiesForm, -13)
    HOLDER(QUndoStack, UndoStack, undoStack, -17)
    HOLDER2(GraphicsView, graphicsView, -14)
    HOLDER2(LayoutFrames, layoutFrames, -15)
    HOLDER2(MainWindow, mainWindow, -16)
    HOLDER2(Project, project, -18)
    HOLDER2(SplashScreen, splashScreen, -19)

    //    class GiPin* pins[2];
    class GiMarker* markers[2];

    FileInterfacesMap filePlugins_;
    GCodeInterfaceMap gCodePlugin_;
    ShapeInterfacesMap shapePlugin_;

    AppSettings appSettings_;
    Handlers handlers_;
    QSettings settings_;
    QString settingsPath_;
    ToolHolder toolHolder_;
    int dashOffset_ {};

    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;
    App(App&&) = delete;
    App(const App&) = delete;

    QSharedMemory sharedMemory {"AppSettings"};

public:
    explicit App() {
        if (sharedMemory.create(sizeof(nullptr), QSharedMemory::ReadWrite)) {
            app_ = *reinterpret_cast<App**>(sharedMemory.data()) = this;
        } else if (sharedMemory.attach(QSharedMemory::ReadOnly)) {
            app_ = *reinterpret_cast<App**>(sharedMemory.data());
        } else {
            qDebug() << "App" << app_ << sharedMemory.errorString();
        }
    }

    static auto& dashOffset() { return app_->dashOffset_; }

    static void setMarkers(int i, GiMarker* marker) { app_->markers[i] = marker; }
    static auto* zero() { return app_->markers[0]; }
    static auto* home() { return app_->markers[1]; }

    static auto& settingsPath() { return app_->settingsPath_; }

    static FilePlugin* filePlugin(int type) { return app_->filePlugins_.contains(type) ? app_->filePlugins_[type] : nullptr; }
    static auto& filePlugins() { return app_->filePlugins_; }

    static Shapes::Plugin* shapePlugin(int type) { return app_->shapePlugin_.contains(type) ? app_->shapePlugin_[type] : nullptr; }
    static auto& shapePlugins() { return app_->shapePlugin_; }

    static GCodePlugin* gCodePlugin(int type) { return app_->gCodePlugin_.contains(type) ? app_->gCodePlugin_[type] : nullptr; }
    static auto& gCodePlugins() { return app_->gCodePlugin_; }

    static auto& shapeHandlers() { return app_->handlers_; }

    static auto& settings() { return app_->appSettings_; }
    static auto& toolHolder() { return app_->toolHolder_; }
    static auto* qSettings() { return &app_->settings_; }
};
