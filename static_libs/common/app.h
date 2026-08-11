/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QSharedMemory>

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

// using handles = std::vector<Shapes::Handle*>;

using FilePluginMap = std::map<uint32_t, AbstractFilePlugin*, std::less<>>;
using GCodePluginMap = std::map<uint32_t, GCode::Plugin*>;
using ShapePluginMap = std::map<int, Shapes::Plugin*>;

#define SINGLETON(TYPE, SET, NAME)                                                                               \
private:                                                                                                         \
    class TYPE* NAME##_ = nullptr;                                                                               \
                                                                                                                 \
public:                                                                                                          \
    static TYPE& NAME() {                                                                                        \
        assert(app->NAME##_);                                                                                    \
        return *app->NAME##_;                                                                                    \
    }                                                                                                            \
    /* Проверка app обязательна: указатели спрашивают и до создания App                                 \
     * (выбор режима OpenGL в main), и после её разрушения -- туда ходит  \
     * обработчик сообщений Qt, и на нулевом app он ронял процесс. */ \
    static TYPE* NAME##Ptr() {                                                                                   \
        return app ? app->NAME##_ : nullptr;                                                                     \
    }                                                                                                            \
    static void SET(TYPE* NAME) {                                                                                \
        if(app->NAME##_ && NAME)                                                                                 \
            throw std::logic_error(__FUNCTION__);                                                                \
        /*qInfo() << #NAME << NAME;*/                                                                            \
        app->NAME##_ = NAME;                                                                                     \
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
    // QSettings settings_;
    QString settingsPath_;
    ToolHolder toolHolder_;
    // Смещение штриха «бегущих муравьёв» на выделении. double, а не int:
    // GraphicsView держит его в пределах периода узора через fmod, иначе
    // счётчик за долгую сессию уходил в переполнение.
    double dashOffset_{};
    // 1.0 / |m11| вида. Публикуется GraphicsView при каждой смене
    // трансформации, чтобы item'ы не ходили за масштабом через
    // scene()->views().front()->transform() на каждый вызов.
    double viewScaleFactor_{1.0};

    QSharedMemory sharedMemory{sharedKey()};

    const bool isDebug_{QCoreApplication::applicationDirPath().contains(u"GERBER_X3/bin"_s)};

    bool drawPdf_{};

public:
    // Имя сегмента -- СВОЁ У КАЖДОГО ПРОЦЕССА. Сегмент нужен ровно затем, чтобы
    // указатель на App видели плагины: common -- статическая библиотека, и у
    // каждого .so своя копия inline static app. Делить его между РАЗНЫМИ копиями
    // программы не нужно и нельзя, а QSharedMemory общесистемна: со старым
    // именем "AppSettings" вторая запущенная копия не создавала сегмент, а
    // прицеплялась к чужому и забирала указатель на App первой -- адрес чужого
    // адресного пространства. Падало на первом же App::...(), то есть сразу на
    // старте, ещё до открытия файлов.
    //
    // Заодно перестаёт мешать сегмент, оставшийся от аварийно завершившейся
    // копии: у новой копии другой pid, а значит и другое имя.
    static QString sharedKey() {
        return u"AppSettings-"_s + QString::number(QCoreApplication::applicationPid());
    }

    explicit App() {
        if(sharedMemory.create(sizeof(void*), QSharedMemory::ReadWrite))
            app = *reinterpret_cast<App**>(sharedMemory.data()) = this;
        else if(sharedMemory.attach(QSharedMemory::ReadOnly))
            app = *reinterpret_cast<App**>(sharedMemory.data());
        else
            qDebug() << u"App"_s << app << sharedMemory.errorString();
    }
    static auto& dashOffset() { return app->dashOffset_; }
    static auto& viewScaleFactor() { return app->viewScaleFactor_; }

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
    // static auto* qSettings() { return &app->settings_; }

    static bool drawPdf() { return app->drawPdf_; }
    static void setDrawPdf(bool newDrawPdf) { app->drawPdf_ = newDrawPdf; }
};
