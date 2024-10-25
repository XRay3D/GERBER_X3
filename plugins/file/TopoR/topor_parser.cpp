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
#include "topor_parser.h"
#include "ctre.hpp"
#include "topor_file.h"
#include <QFile>
#include <cmath>
#include <qserializer.h>

#include "TopoR_PCB_File.h"

namespace TopoR {

Parser::Parser(AbstractFilePlugin* const interface)
    : interface(interface) {
}

AbstractFile* Parser::parseFile(const QString& fileName) {
    qDebug(__FUNCTION__);
    QFile file_(fileName);
    if(!file_.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    TopoR_PCB_File u;
    auto data{file_.readAll()};
    u.fromXml(data);

    data = u.toRawXml();

    file = new File;

    return file;
}

} // namespace TopoR
