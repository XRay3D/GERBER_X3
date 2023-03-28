// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_plugin.h"
#include "topor_file.h"
#include "topor_types.h"

#include "ctre.hpp"
#include "drill/drill_form.h"
#include "file.h"
#include "topor_settingstab.h"
#include "utils.h"

#include <QComboBox>
#include <QJsonObject>
#include <variant>

namespace TopoR {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin(parent)
    , Parser(this) {
}

AbstractFile* Plugin::parseFile(const QString& fileName, int type_) {
    if (type_ != type())
        return nullptr;
    if (!QFile(fileName).exists())
        return nullptr;

    Parser::parseFile(fileName);
    emit fileReady(Parser::file);
    return Parser::file;
}

std::any Plugin::createPreviewGi(AbstractFile* file, GCodePlugin* plugin, std::any param) {
    if (plugin->type() == ::GCode::Drill) {
        DrillPlugin::Preview retData;
        //        auto const exFile = static_cast<File*>(file);
        //        QTransform t {exFile->transform()};
        //        for (const Excellon::Hole& hole : *exFile) {
        //            auto name {QString("T%1").arg(hole.state.toolId)};
        //            if (bool slot = hole.state.path.size(); slot)
        //                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.path));
        //            else
        //                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.pos));
        //        }
        return retData;
    }
    return {};
}

bool Plugin::thisIsIt(const QString& fileName) {
    if (!fileName.endsWith(".fst", Qt::CaseInsensitive))
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    QString line;

    while (in.readLineInto(&line)) {
        if (line.contains("<TopoR_PCB_File>")) {
            qDebug(__FUNCTION__);
            return true;
        }
    }

    return false;
}

int Plugin::type() const { return int(FileType::TopoR); }

QString Plugin::folderName() const { return tr("TopoR"); }

AbstractFile* Plugin::loadFile(QDataStream& stream) { return new / File(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'T'); }

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    auto tab = new ExSettingsTab(parent);
    tab->setWindowTitle("Excellon");
    return tab;
}

void Plugin::addToGcForm(AbstractFile* file, QComboBox* cbx) {
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

} // namespace TopoR

#include "moc_topor_plugin.cpp"
