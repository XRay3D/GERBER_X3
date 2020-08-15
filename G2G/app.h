#pragma once
//#ifndef APP_H
//#define APP_H

#include <QtGlobal>
#include <exception>

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

class App {
    static App* mInstance;
    //    Q_DISABLE_COPY(App)

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

public:
    App()
    {
        if (!mInstance)
            mInstance = this;
    }
    ~App() = default;

    static DrillForm* drillForm() { return mInstance->m_drillForm; }
    static FileModel* fileModel() { return mInstance->m_fileModel; }
    static GCode::Creator* creator() { return mInstance->m_creator; }
    static GCodePropertiesForm* gCodePropertiesForm() { return mInstance->m_gCodePropertiesForm; }
    static GraphicsView* graphicsView() { return mInstance->m_graphicsView; }
    static LayoutFrames* layoutFrames() { return mInstance->m_layoutFrames; }
    static MainWindow* mainWindow() { return mInstance->m_mainWindow; }
    static Project* project() { return mInstance->m_project; }
    static Scene* scene() { return mInstance->m_scene; }
    static SplashScreen* splashScreen() { return mInstance->m_splashScreen; }
};

//#endif // APP_H
