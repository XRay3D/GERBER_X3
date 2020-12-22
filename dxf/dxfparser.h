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

#include "gbrattributes.h"
#include "gbrfile.h"
#include "gbrtypes.h"
#include "parser.h"
#include <QObject>
#include <QStack>

namespace Dxf {

class File;

class Parser : public FileParser {
    Q_OBJECT
public:
    explicit Parser(QObject* parent = nullptr);
    // FileParser interface
    AbstractFile* parseFile(const QString& fileName) override;
    bool isDxfFile(const QString& fileName);

private:
    File* dxfFile() { return reinterpret_cast<File*>(m_file); };
};

}
