/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "interfaces/file.h"
#include "interfaces/pluginfile.h"

#include "recent.h"

#include <QActionGroup>
#include <QDockWidget>
#include <QMainWindow>
#include <QStack>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <qevent.h>

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
    static void updateTheme();

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

    std::map<int, QAction*> toolpathActions;
    QActionGroup actionGroup;
    QMap<QString, QProgressDialog*> m_progressDialogs;
    QMessageBox reloadQuestion;

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();

    template <class T>
    void createDockWidget();

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

    void customContextMenuForToolBar(const QPoint& pos);

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

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

class DockWidget : public QDockWidget {
    Q_OBJECT
    QStack<QWidget*> widgets;
    //    void setWidget(QWidget*) { }

public:
    explicit DockWidget(QWidget* parent = nullptr)
        : QDockWidget(parent) {
        hide();
        setVisible(false);
    }
    ~DockWidget() override = default;

    void push(QWidget* w) {
        if (widget())
            widgets.push(widget());
        if (w)
            QDockWidget::setWidget(w);
    }
    void pop() {
        if (widget()) {
            if (widget()->objectName() == "ErrorDialog") {
                static_cast<QDialog*>(widget())->reject();
                QTimer::singleShot(1, [this] { widgets.pop(); });
            } else {
                delete widget();
            }
        }
        if (!widgets.isEmpty())
            QDockWidget::setWidget(widgets.pop());
    }

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override {
        pop();
        event->accept();
    }
    void showEvent(QShowEvent* event) override {
        event->ignore();
        if (widget() == nullptr)
            QTimer::singleShot(1, this, &QDockWidget::close);
    }
};
