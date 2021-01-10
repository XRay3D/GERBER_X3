/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "recent.h"

#include "mainwindow.h"

#include <QMenu>
#include <QSettings>

Recent::Recent(MainWindow* mainWindow, QString&& recentFilesKey)
    : QObject(mainWindow)
    , mainWindow(mainWindow)
    , recentFilesKey(std::move(recentFilesKey))
{
}

void Recent::createMenu(QMenu* fileMenu, const QString& menuName)
{
    recentMenu = fileMenu->addMenu(menuName);
    connect(recentMenu, &QMenu::aboutToShow, this, &Recent::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < Recent::MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &Recent::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }
    recentMenu->addSeparator();
    recentFileActs[Recent::MaxRecentFiles] = recentMenu->addAction(tr("Clear Recent"), [this] {
        QSettings settings;
        writeRecentFiles({}, settings);
        updateRecentFileActions();
        setRecentFilesVisible(hasRecentFiles());
    });

    recentFileSeparator = fileMenu->addSeparator();
    setRecentFilesVisible(hasRecentFiles());
}

QStringList Recent::readRecentFiles(QSettings& settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.push_back(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

void Recent::writeRecentFiles(const QStringList& files, QSettings& settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey);
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

QString Recent::fileKey() { return QStringLiteral("file"); }

void Recent::setRecentFilesVisible(bool visible)
{
    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

bool Recent::hasRecentFiles()
{
    QSettings settings;
    const int count = settings.beginReadArray(recentFilesKey);
    settings.endArray();
    return count > 0;
}

void Recent::prependToRecentFiles(const QString& fileName)
{
    QSettings settings;
    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);
    setRecentFilesVisible(!recentFiles.isEmpty());
    mainWindow->documentWasModified();
}

void Recent::updateRecentFileActions()
{
    QSettings settings;

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = mainWindow->strippedName(recentFiles.at(i));
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for (; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);

    recentFileActs[MaxRecentFiles]->setVisible(count);
}

void Recent::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        mainWindow->loadFile(action->data().toString());
}
