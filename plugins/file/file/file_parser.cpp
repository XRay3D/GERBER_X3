// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "file_parser.h"
#include "ctre.hpp"
#include "file_file.h"
#include <QFile>
#include <cmath>

namespace TmpFile {

Parser::Parser(AbstractFilePlugin* const interface)
    : interface(interface) {
}

AbstractFile* Parser::parseFile(const QString& fileName) {
    QFile file_(fileName);
    if(!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    file = new File;

    return file;
}

} // namespace TmpFile
