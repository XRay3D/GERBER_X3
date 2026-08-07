/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "g2_plugin.h"

#include <QFile>
#include <QTextStream>

namespace Gerber2 {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} { }

QIcon Plugin::icon() const { return decoration(Qt::darkCyan, u'2'); }

AbstractFile* Plugin::loadFile(QDataStream& stream) const { return AbstractFile::load<File>(stream); }

bool Plugin::thisIsIt(const QString&) {
    // Плагин альтернативный: файл ему отдают только по явному выбору
    // пользователя, иначе гербер перехватил бы основной плагин.
    return false;
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;

    QFile source{fileName};
    if(!source.open(QFile::ReadOnly | QFile::Text)) {
        emit fileError(fileName, source.errorString());
        return nullptr;
    }
    QTextStream in{&source};
    in.setAutoDetectUnicode(true);

    auto file = std::make_unique<File>();
    file->setFileName(fileName);

    QString error;
    if(!file->setSource(in.readAll(), &error)) {
        emit fileError(fileName, error);
        return nullptr;
    }
    for(const QString& warning: file->parsed().warnings)
        emit fileWarning(fileName, warning);

    return file.release();
}

} // namespace Gerber2

#include "moc_g2_plugin.cpp"
