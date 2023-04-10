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

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QSharedMemory>

#include "mvector.h"
#include "settings.h"
#include "tool.h"

#include <assert.h>
#include <map>

namespace Shapes {
class Plugin;
class Handle;
} // namespace Shapes

namespace GCode {
class Plugin;
class PropertiesForm;
class Settings;
} // namespace GCode

class AbstractFilePlugin;
namespace GCodeShapes {
class Plugin;
class Handle;
} // namespace GCodeShapes

namespace FileTree {
class View;
class Model;
} // namespace FileTree

namespace Drilling {
class Form;
} // namespace Drilling

using Handlers = mvector<Shapes::Handle*>;

using FilePluginMap = std::map<uint32_t, AbstractFilePlugin*, std::less<>>;
using GCodePluginMap = std::map<uint32_t, GCode::Plugin*>;
using ShapePluginMap = std::map<int, Shapes::Plugin*>;

#define HOLDER(TYPE, SET, NAME)                   \
private:                                          \
    class TYPE* NAME##_ = nullptr;                \
                                                  \
public:                                           \
    static auto& NAME() {                         \
        /* assert(app_->NAME##_); */              \
        return *app_->NAME##_;                    \
    }                                             \
    static auto NAME##Ptr() {                     \
        /* assert(app_->NAME##_); */              \
        return app_->NAME##_;                     \
    }                                             \
    static void SET(TYPE* NAME) {                 \
        if (app_->NAME##_ && NAME)                \
            throw std::logic_error(__FUNCTION__); \
        else                                      \
            app_->NAME##_ = NAME;                 \
    }

class App {
    inline static App* app_ = nullptr;

    // clang-format off
    HOLDER(Drilling::Form,        setDrillForm,           drillForm          )
    HOLDER(FileTree::Model,       setFileModel,           fileModel          )
    HOLDER(FileTree::View,        setFileTreeView,        fileTreeView       )
    HOLDER(GCode::PropertiesForm, setGCodePropertiesForm, gCodePropertiesForm)
    HOLDER(QUndoStack,            setUndoStack,           undoStack          )
    HOLDER(GraphicsView,          setGraphicsView,        graphicsView       )
    HOLDER(LayoutFrames,          setLayoutFrames,        layoutFrames       )
    HOLDER(MainWindow,            setMainWindow,          mainWindow         )
    HOLDER(Project,               setProject,             project            )
    HOLDER(QSplashScreen,         setSplashScreen,        splashScreen       )
    HOLDER(GCode::Settings,       setGcSettings,          gcSettings         )

    HOLDER(GiMarker,              setHome,                home               )
    HOLDER(GiMarker,              setZero,                zero               )
    // clang-format on

    FilePluginMap filePlugins_;
    GCodePluginMap gCodePlugin_;
    ShapePluginMap shapePlugin_;

    AppSettings appSettings_;

    Handlers handlers_;
    //    QSettings settings_;
    QString settingsPath_;
    ToolHolder toolHolder_;
    int dashOffset_ {};

    App& operator=(App&& a) = delete;
    App& operator=(const App& app) = delete;
    App(App&&) = delete;
    App(const App&) = delete;

    QSharedMemory sharedMemory {"AppSettings"};

    const bool isDebug_ {QCoreApplication::applicationDirPath().contains("GERBER_X3/bin")};

    bool drawPdf_ {};

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

    static bool isDebug() { return app_->isDebug_; }

    static auto& settingsPath() { return app_->settingsPath_; }

    static AbstractFilePlugin* filePlugin(uint32_t type) { return app_->filePlugins_.contains(type) ? app_->filePlugins_[type] : nullptr; }
    static auto& filePlugins() { return app_->filePlugins_; }

    static GCode::Plugin* gCodePlugin(uint32_t type) { return app_->gCodePlugin_.contains(type) ? app_->gCodePlugin_[type] : nullptr; }
    static auto& gCodePlugins() { return app_->gCodePlugin_; }

    static Shapes::Plugin* shapePlugin(int type) { return app_->shapePlugin_.contains(type) ? app_->shapePlugin_[type] : nullptr; }
    static auto& shapePlugins() { return app_->shapePlugin_; }

    static auto& shapeHandlers() { return app_->handlers_; }

    static auto& settings() { return app_->appSettings_; }

    static auto& toolHolder() { return app_->toolHolder_; }
    //    static auto* qSettings() { return &app_->settings_; }

    static bool drawPdf() { return app_->drawPdf_; }
    static void setDrawPdf(bool newDrawPdf) { app_->drawPdf_ = newDrawPdf; }
};
