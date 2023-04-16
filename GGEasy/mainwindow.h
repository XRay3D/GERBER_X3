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
// #include "a_pch.h"

#include "recent.h"

#include <QActionGroup>
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>
#include <QUndoStack>

namespace FileTree {
class View;
}

namespace GCode {
class File;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    //    friend void FileTree::View::on_doubleClicked(const QModelIndex&);
    friend class Recent;
    friend class Project;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void init();

    // QMainWindow interface
    QMenu* createPopupMenu() override;
    const QDockWidget* dockWidget() const;
    QDockWidget* dockWidget();

    static void translate(const QString& locale);
    void loadFile(const QString& fileName);
    static void updateTheme();

    QUndoStack& undoStack() { return undoStack_; }

signals:
    void parseFile(const QString& filename, int type);

private slots:
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void addFileToPro(class AbstractFile* file);
    void setDockWidget(QWidget* dwContent);

private:
    QDockWidget* dockWidget_ = nullptr;
    Recent recentFiles;
    Recent recentProjects;

    QAction* closeAllAct_ = nullptr;
    QAction* redoAct = nullptr;
    QAction* undoAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QMenu* serviceMenu = nullptr;

    QString lastPath;
    QThread parserThread;

    QToolBar* fileToolBar = nullptr;
    QToolBar* toolpathToolBar = nullptr;
    QToolBar* zoomToolBar = nullptr;
    QUndoStack undoStack_;

    class Project* project_;
    bool openFlag;

    std::map<uint32_t, QAction*> toolpathActions;
    QActionGroup actionGroup;

    QMap<QString, class QProgressDialog*> progressDialogs_;
    QMessageBox reloadQuestion;

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();

    void initWidgets();

    void printDialog();
    void renderPdf();

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

    void customContextMenuForToolBar(const QPoint& pos);

    // save GCode
    void saveGCodeFile(int32_t id);
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
    bool debug();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct Ui {
        class QWidget* centralwidget;
        class QHBoxLayout* horizontalLayout;
        class GraphicsView* graphicsView;
        class QMenuBar* menubar;
        class QStatusBar* statusbar;
        class QDockWidget* treeDockWidget;
        class QWidget* widget;
        class QVBoxLayout* verticalLayout;
        FileTree::View* treeView;
        void setupUi(QMainWindow* MainWindow);       // setupUi
        void retranslateUi(QMainWindow* MainWindow); // retranslateUi
    } ui;
};
