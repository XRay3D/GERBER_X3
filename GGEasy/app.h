/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
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
class TreeView;

class App {
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
    friend class TreeView;

    inline static DrillForm* m_drillForm = nullptr;
    inline static FileModel* m_fileModel = nullptr;
    inline static GCode::Creator* m_creator = nullptr;
    inline static GCodePropertiesForm* m_gCodePropertiesForm = nullptr;
    inline static GraphicsView* m_graphicsView = nullptr;
    inline static LayoutFrames* m_layoutFrames = nullptr;
    inline static MainWindow* m_mainWindow = nullptr;
    inline static Project* m_project = nullptr;
    inline static Scene* m_scene = nullptr;
    inline static SplashScreen* m_splashScreen = nullptr;
    inline static TreeView* m_treeView = nullptr;

public:
    static DrillForm* drillForm() { return m_drillForm; }
    static FileModel* fileModel() { return m_fileModel; }
    static GCode::Creator* creator() { return m_creator; }
    static GCodePropertiesForm* gCodePropertiesForm() { return m_gCodePropertiesForm; }
    static GraphicsView* graphicsView() { return m_graphicsView; }
    static LayoutFrames* layoutFrames() { return m_layoutFrames; }
    static MainWindow* mainWindow() { return m_mainWindow; }
    static Project* project() { return m_project; }
    static Scene* scene() { return m_scene; }
    static SplashScreen* splashScreen() { return m_splashScreen; }
    static TreeView* treeView() { return m_treeView; }
};
