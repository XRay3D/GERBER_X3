/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "svg_plugin.h"
#include "svg_file.h"
#include "svg_types.h"

#include "abstract_file.h"

namespace Svg {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} {
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;

    auto* file = new File;
    file->setFileName(fileName);

    if(!parser_.parseFile(fileName, file)) {
        delete file;
        emit fileError(fileName, tr("SVG parse error"));
        return nullptr;
    }

    if(file->isEmpty()) {
        delete file;
        emit fileError(fileName, tr("No graphical elements found in SVG"));
        return nullptr;
    }

    file->createGi();
    emit fileReady(file);
    return file;
}

bool Plugin::thisIsIt(const QString& fileName) {
    return fileName.endsWith(u".svg"_s, Qt::CaseInsensitive)
        || fileName.endsWith(u".svgz"_s, Qt::CaseInsensitive);
}

uint32_t Plugin::type() const { return int(SVG); }

QString Plugin::folderName() const { return tr("SVG"); }

AbstractFile* Plugin::loadFile(QDataStream& stream) const { return File::load<File>(stream); }

QIcon Plugin::icon() const { return decoration(QColor{0x4A, 0x90, 0xD9}, u'S'); }

} // namespace Svg
