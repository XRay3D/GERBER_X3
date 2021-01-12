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
#pragma once

#include "interfaces/pluginfile.h"

#include <QObject>

namespace GCode {

class Plugin : public QObject, public FilePluginInterface {
    Q_OBJECT
public:
    explicit Plugin(QObject* parent = nullptr);
    QObject* getObject() override;
    bool thisIsIt(const QString& fileName) override;
    int type() const override;

    SettingsTab createSettingsTab(QWidget* parent) override;
    std::shared_ptr<FileInterface> createFile() override;
    QJsonObject info() const override;
    void createMainMenu(QMenu& menu, FileTreeView* tv) override;

public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileError(const QString& fileName, const QString& error) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileReady(FileInterface* file) override;
};
}
