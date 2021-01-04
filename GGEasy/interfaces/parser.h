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

#include <QObject>
#include <QSemaphore>
#include <memory>

class FileInterface;
class NodeInterface;

class FileTreeView;

class QMenu;
class AppSettings;
class App;

class ParserInterface {
public:
    explicit ParserInterface() { }
    virtual ~ParserInterface() { }
    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;
    [[nodiscard]] virtual bool thisIsIt(const QString& fileName) = 0;
    [[nodiscard]] virtual QObject* getObject() = 0;
    [[nodiscard]] virtual int type() const = 0;
    [[nodiscard]] virtual NodeInterface* createNode(FileInterface* file) = 0;
    [[nodiscard]] virtual std::shared_ptr<FileInterface> createFile() = 0;
    virtual void setupInterface(App*, AppSettings*) = 0;
    virtual void createMainMenu(QMenu& menu, FileTreeView* tv)
    {
        Q_UNUSED(menu)
        Q_UNUSED(tv)
    };
    virtual void updateFileModel(FileInterface* file)
    {
        Q_UNUSED(file)
    };

    //signals:
    virtual void fileReady(FileInterface* file) = 0;
    virtual void fileProgress(const QString& fileName, int max, int value) = 0;
    virtual void fileError(const QString& fileName, const QString& error) = 0;

protected:
    FileInterface* m_file = nullptr;
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.ParserInterface"

Q_DECLARE_INTERFACE(ParserInterface, ParserInterface_iid)
