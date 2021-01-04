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

#include "app.h"

#include "interfaces/parser.h"
#include "settings.h"

#include <QObject>
#include <QStack>

namespace Dxf {

class File;

class Parser : public QObject, public ParserInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "dxf.json")
    Q_INTERFACES(ParserInterface)

public:
    explicit Parser(QObject* parent = nullptr);

    // ParserInterface interface
    bool thisIsIt(const QString& fileName) override;
    QObject* getObject() override;
    int type() const override;
    NodeInterface* createNode(FileInterface* file) override;
    std::shared_ptr<FileInterface> createFile() override;
    void setupInterface(App*, AppSettings* s) override;
    void createMainMenu(QMenu& menu, FileTreeView* tv) override;
    void updateFileModel(FileInterface* file) override;

public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileReady(FileInterface* file) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileError(const QString& fileName, const QString& error) override;

private:
    File* dxfFile();
    App app;
    AppSettings appSettings;
};

}
