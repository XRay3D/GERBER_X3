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

#include "interfaces/parser.h"

#include <QObject>

namespace GCode {

class Parser : public QObject, public ParserInterface {
    Q_OBJECT
public:
    explicit Parser(QObject* parent = nullptr);
    bool thisIsIt(const QString& fileName) override;
    QObject* getObject() override;
    int type() const override;
    NodeInterface* createNode(FileInterface* file) override;
    std::shared_ptr<FileInterface> createFile() override;
    void setupInterface(App* a, AppSettings* s) override;
    void createMainMenu(QMenu& menu, FileTreeView* tv) override;

public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileReady(FileInterface* file) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileError(const QString& fileName, const QString& error) override;
};
}
