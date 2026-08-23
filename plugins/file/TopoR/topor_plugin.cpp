/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_plugin.h"

#include "app.h"
#include "topor_file.h"
#include "topor_node.h"
#include "topor_parser.h"

#include <QFile>
#include <QTextStream>

namespace TopoR {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin{parent} {
}

AbstractFile* Plugin::parseFile(const QString& fileName, uint32_t type_) {
    if(type_ != type()) return nullptr;
    if(!QFile::exists(fileName)) return nullptr;

    Parser parser;
    File* file = parser.parseFile(fileName);
    if(!file) {
        emit fileError(QFileInfo(fileName).fileName(), tr("Failed to parse TopoR PCB file"));
        emit fileProgress(fileName, 1, 1);
        return nullptr;
    }

    file->setFileName(fileName);
    emit fileReady(file);
    emit fileProgress(fileName, 1, 1);
    return file;
}

bool Plugin::thisIsIt(const QString& fileName) {
    if(!fileName.endsWith(u".fst"_s, Qt::CaseInsensitive))
        return false;

    QFile file{fileName};
    if(!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in{&file};
    QString line;
    while(in.readLineInto(&line))
        if(line.contains(u"<TopoR_PCB_File>"_s))
            return true;

    return false;
}

AbstractFile* Plugin::loadFile(std::string_view json) const { return Serial::load<File>(json); }

std::string_view Plugin::typeName() const { return Serial::typeNameOf<File>(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, u'T'); }

void Plugin::updateFileModel(AbstractFile* file) {
    const auto fm = App::fileModelPtr();
    const QModelIndex fileIndex = file->node()->index();
    auto* item = fm->getItem(fileIndex);
    if(int count = item->childCount(); count) {
        fm->beginRemoveRows(fileIndex, 0, count - 1);
        do {
            item->remove(--count);
        } while(count);
        fm->endRemoveRows();
    }

    LayerList layers;
    for(Layer* layer: static_cast<File*>(file)->layers())
        if(!layer->isEmpty()) layers.push_back(layer);
    if(layers.empty()) return;

    fm->beginInsertRows(fileIndex, 0, int(layers.size()) - 1);
    for(Layer* layer: layers)
        item->addChild(new NodeLayer{layer->name(), layer});
    fm->endInsertRows();
}

} // namespace TopoR

#include "moc_topor_plugin.cpp"
