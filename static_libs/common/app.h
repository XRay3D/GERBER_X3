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

#include "reflection.h" // fixed_string
#include "settings.h"
#include "tool.h"
#include "utils.h" //using namespace Qt::Literals;

#include <assert.h>
#include <map>
#include <meta>
#include <stdexcept>
#include <vector>

class AbstractFilePlugin;
class GraphicsView;
class LayoutFrames;
class MainWindow;
class Project;
class QSplashScreen;
class QUndoStack;

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

// Синглтоны приложения. Единственный список «тип + имя»: из него рефлексией
// (P2996, define_aggregate) собирается хранилище AppBase -- по одному T* на
// слот, все nullptr. Порядок слотов = порядок полей AppBase.
namespace detail {
struct AppSlot {
    std::meta::info type;
    std::string_view name;
};
consteval std::vector<AppSlot> appSlots() {
    // clang-format off
    return {
        {^^GCode::PropertiesForm, "gcPropertiesForm"},
        {^^Drilling::Form,        "drillForm"       },
        {^^FileTree::Model,       "fileModel"       },
        {^^FileTree::View,        "fileTreeView"    },
        {^^GCode::Settings,       "gcSettings"      },
        {^^GraphicsView,          "grView"          },
        {^^LayoutFrames,          "layoutFrames"    },
        {^^MainWindow,            "mainWindow"      },
        {^^Project,               "project"         },
        {^^QSplashScreen,         "splashScreen"    },
        {^^QUndoStack,            "undoStack"       },
        {^^Gi::Marker,            "home"            },
        {^^Gi::Marker,            "zero"            },
        {^^Gi::Pin,               "pin0"            },
        {^^Gi::Pin,               "pin1"            },
        {^^Gi::Pin,               "pin2"            },
        {^^Gi::Pin,               "pin3"            },
    };
    // clang-format on
}
} // namespace detail

struct AppBase;
consteval {
    std::vector<std::meta::info> members;
    for(auto slot: detail::appSlots())
        members.push_back(std::meta::data_member_spec(std::meta::add_pointer(slot.type), {.name = slot.name}));
    std::meta::define_aggregate(^^AppBase, members);
}

// Поле AppBase по имени. Свободная функция, а не член App: внутри тела App
// она ещё не определена в момент использования в static constexpr членах.
template <fixed_string Name>
consteval std::meta::info appMember() {
    for(auto m: std::meta::nonstatic_data_members_of(^^AppBase, std::meta::access_context::unchecked()))
        if(std::meta::identifier_of(m) == Name.sv()) return m;
    throw "App: no such singleton";
}

class App : AppBase {
    Q_DISABLE_COPY_MOVE(App)
    // Определение -- в app.cpp: единственная копия живёт в libggcore, и все
    // модули (exe и плагины) биндятся на неё через динамический линкер.
    // Раньше common влинковывался в каждый .so статически, у каждого была
    // своя копия inline static, и указатель разносился через QSharedMemory.
    static App* app;

    // Доступ к слоту. data_member_spec порождает только нестатические поля
    // данных (ни статических членов, ни функций), поэтому именованные
    // аксессоры -- static constexpr объекты: App::project() через operator(),
    // App::project.ptr(), App::project.set(p).
    template <std::meta::info M>
    struct Accessor {
        using T = std::remove_pointer_t<typename[:std::meta::type_of(M):]>;
        T& operator()() const {
            assert(app->[:M:]);
            return *app->[:M:];
        }
        // Проверка app обязательна: указатели спрашивают и до создания App
        // (выбор режима OpenGL в main), и после её разрушения -- туда ходит
        // обработчик сообщений Qt, и на нулевом app он ронял процесс.
        T* ptr() const { return app ? app->[:M:] : nullptr; }
        void set(T* p) const {
            if(app->[:M:] && p)
                throw std::logic_error(std::string{std::meta::identifier_of(M)});
            app->[:M:] = p;
        }
    };

public:
    // clang-format off
    static constexpr Accessor<appMember<"gcPropertiesForm">()> gcPropertiesForm{};
    static constexpr Accessor<appMember<"drillForm">()>        drillForm{};
    static constexpr Accessor<appMember<"fileModel">()>        fileModel{};
    static constexpr Accessor<appMember<"fileTreeView">()>     fileTreeView{};
    static constexpr Accessor<appMember<"gcSettings">()>       gcSettings{};
    static constexpr Accessor<appMember<"grView">()>           grView{};
    static constexpr Accessor<appMember<"layoutFrames">()>     layoutFrames{};
    static constexpr Accessor<appMember<"mainWindow">()>       mainWindow{};
    static constexpr Accessor<appMember<"project">()>          project{};
    static constexpr Accessor<appMember<"splashScreen">()>     splashScreen{};
    static constexpr Accessor<appMember<"undoStack">()>        undoStack{};
    static constexpr Accessor<appMember<"home">()>             home{};
    static constexpr Accessor<appMember<"zero">()>             zero{};
    static constexpr Accessor<appMember<"pin0">()>             pin0{};
    static constexpr Accessor<appMember<"pin1">()>             pin1{};
    static constexpr Accessor<appMember<"pin2">()>             pin2{};
    static constexpr Accessor<appMember<"pin3">()>             pin3{};
    // clang-format on

    // То же по строковому имени: App::get<"project">(), ptr<>, set<>.
    template <fixed_string N>
    static auto& get() { return Accessor<appMember<N>()>{}(); }
    template <fixed_string N>
    static auto* ptr() { return Accessor<appMember<N>()>{}.ptr(); }
    template <fixed_string N>
    static void set(auto* p) { Accessor<appMember<N>()>{}.set(p); }

private:
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

    const bool isDebug_{QCoreApplication::applicationDirPath().contains(u"GERBER_X3/bin"_s)};

    bool drawPdf_{};

public:
    explicit App() {
        if(!app) app = this;
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
