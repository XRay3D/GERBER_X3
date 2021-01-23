/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "interfaces/file.h"
#include "interfaces/pluginfile.h"

#include "recent.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QStack>
#include <QThread>
#include <QTranslator>

namespace GCode {
class File;
}

class DockWidget;
class Project;
class QProgressDialog;
class QToolBar;
class Scene;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    //    friend void FileTree::View::on_doubleClicked(const QModelIndex&);
    friend class Recent;
    friend class Project;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // QMainWindow interface
    QMenu* createPopupMenu() override;
    const DockWidget* dockWidget() const { return m_dockWidget; }
    DockWidget* dockWidget() { return m_dockWidget; }

    static void translate(const QString& locale);
    void loadFile(const QString& fileName);

signals:
    void parseFile(const QString& filename, int type);

private slots:
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void addFileToPro(FileInterface* file);

private:
    Ui::MainWindow* ui;
    DockWidget* m_dockWidget = nullptr;
    Recent recentFiles;
    Recent recentProjects;

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

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();
    template <class T>
    void createDockWidget(/*QWidget* dwContent,*/ int type);

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
    void createActionsShape();

    void customContextMenuForToolBar(const QPoint &pos);

    // save GCode
    void saveGCodeFile(int id);
    void saveGCodeFiles();
    void saveSelectedGCodeFiles();

    QString strippedName(const QString& fullFileName);

    void newFile();
    void documentWasModified();
    bool maybeSave();

    void editGcFile(GCode::File* file);

private:
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
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
