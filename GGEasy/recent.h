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
#include <QObject>

class QAction;
class QMenu;
class MainWindow;
class QSettings;

class Recent : public QObject {
    Q_OBJECT
    MainWindow* mainWindow;

public:
    explicit Recent(MainWindow* mainWindow, QString&& recentFilesKey);

    enum { MaxRecentFiles = 20 };
    void createMenu(QMenu* fileMenu, const QString& menuName);

    QStringList readRecentFiles(QSettings& settings);
    bool hasRecentFiles();
    void openRecentFile();
    void prependToRecentFiles(const QString& fileName);
    void setRecentFilesVisible(bool visible);
    void updateRecentFileActions();
    void writeRecentFiles(const QStringList& files, QSettings& settings);

    QString fileKey();
    const QString recentFilesKey;

    QMenu* recentMenu = nullptr;
    QAction* recentFileSeparator = nullptr;
    QAction* recentFileSubMenuAct = nullptr;
    QAction* recentFileActs[MaxRecentFiles + 1];
signals:
};
