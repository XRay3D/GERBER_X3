/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "recent.h"
#include "ui_mainwindow.h"
#include <QStack>
#include <QThread>
#include <QTranslator>

namespace Gerber {
class Parser;
}
namespace Excellon {
class Parser;
}
namespace Dxf {
class Parser;
}
namespace GCode {
class File;
}

class DockWidget;
class Project;
class QProgressDialog;
class Scene;

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT
    friend void FileTreeView::on_doubleClicked(const QModelIndex&);
    friend class Recent;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // QMainWindow interface
    QMenu* createPopupMenu() override;
    const DockWidget* dockWidget() const { return m_dockWidget; }
    DockWidget* dockWidget() { return m_dockWidget; }

    static void translate(const QString& locale);

signals:
    void parseGerberFile(const QString& filename);
    void parseExcellonFile(const QString& filename);
    void parseDxfFile(const QString& filename);

private:
    DockWidget* m_dockWidget = nullptr;
    Recent recentFiles;
    Recent recentProjects;

    Gerber::Parser* gerberParser;
    Excellon::Parser* excellonParser;
    Dxf::Parser* dxfParser;

    QAction* m_closeAllAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QMenu* serviceMenu = nullptr;

    QString lastPath;
    QThread parserThread;

    QToolBar* fileToolBar = nullptr;
    QToolBar* toolpathToolBar = nullptr;
    QToolBar* zoomToolBar = nullptr;

    Project* m_project;
    bool openFlag;

    QMap<int, QAction*> toolpathActions;

    QMap<QString, QProgressDialog*> m_progressDialogs;

    //file

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();
    template <class T>
    void createDockWidget(/*QWidget* dwContent,*/ int type);
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void initWidgets();

    void printDialog();
    void readSettings();
    void resetToolPathsActions();
    void selectAll();
    void deSelectAll();

    void writeSettings();

    // create actions
    void createActions();
    void createActionsFile();
    void createActionsEdit();
    void createActionsService();
    void createActionsHelp();
    void createActionsZoom();
    void createActionsToolPath();
    void createActionsGraphics();

    QString strippedName(const QString& fullFileName);

    void newFile();
    void documentWasModified();
    bool maybeSave();

    void editGcFile(GCode::File* file);

public:
    void loadFile(const QString& fileName);

private:
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);
    void addFileToPro(AbstractFile* file);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;
};

class DockWidget : public QDockWidget {
    Q_OBJECT
    QStack<QWidget*> widgets;
    void setWidget(QWidget*) { }

public:
    explicit DockWidget(QWidget* parent = nullptr);
    ~DockWidget() override = default;

    void push(QWidget* w);
    void pop();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
};
