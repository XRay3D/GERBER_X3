/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#pragma once

#include "abstractfile.h"
#include <QObject>

class File;

class FileParser : public QObject {
    Q_OBJECT
public:
    explicit FileParser(QObject* parent = nullptr);
    virtual ~FileParser();
    virtual AbstractFile* parseFile(const QString& fileName) = 0;

signals:
    void fileReady(AbstractFile* file);
    void fileProgress(const QString& fileName, int max, int value);
    void fileError(const QString& fileName, const QString& error);

protected:
    AbstractFile* m_file = nullptr;
};
