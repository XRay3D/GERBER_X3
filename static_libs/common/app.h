/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QSharedMemory>

#include "mvector.h"
#include "settings.h"
#include "tool.h"
#include "utils.h" //using namespace Qt::Literals;

#include <assert.h>
#include <map>

class AbstractFilePlugin;

namespace Drilling {
class Form;
} // namespace Drilling

namespace FileTree {
class View;
class Model;
} // namespace FileTree

namespace GCode {
class Plugin;
class PropertiesForm;
class Settings;
} // namespace GCode

namespace GCodeShapes {
class Plugin;
class Handle;
} // namespace GCodeShapes

namespace Gi {
class Marker;
class Pin;
} // namespace Gi

namespace Shapes {
class Plugin;
// class Handle;
} // namespace Shapes

// using handles = mvector<Shapes::Handle*>;

using FilePluginMap = std::map<uint32_t, AbstractFilePlugin*, std::less<>>;
using GCodePluginMap = std::map<uint32_t, GCode::Plugin*>;
using ShapePluginMap = std::map<int, Shapes::Plugin*>;

#define SINGLETON(TYPE, SET, NAME)                \
private:                                          \
    class TYPE* NAME##_ = nullptr;                \
                                                  \
public:                                           \
    static auto& NAME() {                         \
        assert(app->NAME##_);                     \
        return *app->NAME##_;                     \
    }                                             \
    static auto* NAME##Ptr() {                    \
        /*assert(app->NAME##_);*/                 \
        return app->NAME##_;                      \
    }                                             \
    static void SET(TYPE* NAME) {                 \
        if(app->NAME##_ && NAME)                  \
            throw std::logic_error(__FUNCTION__); \
        /*qInfo() << #NAME << NAME;*/             \
        app->NAME##_ = NAME;                      \
    }

class App {
    Q_DISABLE_COPY_MOVE(App)
    inline static App* app{};

    // clang-format off
    SINGLETON(GCode::PropertiesForm, setGCodePropertiesForm, gcPropertiesForm)
    SINGLETON(Drilling::Form,  setDrillForm,    drillForm    )
    SINGLETON(FileTree::Model, setFileModel,    fileModel    )
    SINGLETON(FileTree::View,  setFileTreeView, fileTreeView )
    SINGLETON(GCode::Settings, setGcSettings,   gcSettings   )
    SINGLETON(GraphicsView,    setGraphicsView, grView       )
    SINGLETON(LayoutFrames,    setLayoutFrames, layoutFrames )
    SINGLETON(MainWindow,      setMainWindow,   mainWindow   )
    SINGLETON(Project,         setProject,      project      )
    SINGLETON(QSplashScreen,   setSplashScreen, splashScreen )
    SINGLETON(QUndoStack,      setUndoStack,    undoStack    )

    SINGLETON(Gi::Marker, setHome, home)
    SINGLETON(Gi::Marker, setZero, zero)
    SINGLETON(Gi::Pin,    setPin0, pin0)
    SINGLETON(Gi::Pin,    setPin1, pin1)
    SINGLETON(Gi::Pin,    setPin2, pin2)
    SINGLETON(Gi::Pin,    setPin3, pin3)
    // clang-format on

    FilePluginMap filePlugins_;
    GCodePluginMap gCodePlugin_;
    ShapePluginMap shapePlugin_;

    AppSettings appSettings_;

    // handles handles_;
    //    QSettings settings_;
    QString settingsPath_;
    ToolHolder toolHolder_;
    int dashOffset_{};

    QSharedMemory sharedMemory{"AppSettings"};

    const bool isDebug_{QCoreApplication::applicationDirPath().contains("GERBER_X3/bin")};

    bool drawPdf_{};

public:
    explicit App() {
        if(sharedMemory.create(sizeof(void*), QSharedMemory::ReadWrite))
            app = *reinterpret_cast<App**>(sharedMemory.data()) = this;
        else if(sharedMemory.attach(QSharedMemory::ReadOnly))
            app = *reinterpret_cast<App**>(sharedMemory.data());
        else
            qDebug() << "App" << app << sharedMemory.errorString();
    }
    static auto& dashOffset() { return app->dashOffset_; }

    static auto pins() {
        static std::vector pins{&App::pin0(), &App::pin1(), &App::pin2(), &App::pin3()};
        return pins;
    }

    static bool isDebug() { return app->isDebug_; }

    static auto& settingsPath() { return app->settingsPath_; }

    static AbstractFilePlugin* filePlugin(uint32_t type) { return app->filePlugins_.contains(type) ? app->filePlugins_[type] : nullptr; }
    static auto& filePlugins() { return app->filePlugins_; }

    static GCode::Plugin* gCodePlugin(uint32_t type) { return app->gCodePlugin_.contains(type) ? app->gCodePlugin_[type] : nullptr; }
    static auto& gCodePlugins() { return app->gCodePlugin_; }

    static Shapes::Plugin* shapePlugin(int type) { return app->shapePlugin_.contains(type) ? app->shapePlugin_[type] : nullptr; }
    static auto& shapePlugins() { return app->shapePlugin_; }

    // static auto& shapehandles() { return app->handles_; }

    static auto& settings() { return app->appSettings_; }

    static auto& toolHolder() { return app->toolHolder_; }
    //    static auto* qSettings() { return &app->settings_; }

    static bool drawPdf() { return app->drawPdf_; }
    static void setDrawPdf(bool newDrawPdf) { app->drawPdf_ = newDrawPdf; }
};
