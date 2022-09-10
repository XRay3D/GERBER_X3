// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "ex_plugin.h"
#include "ex_file.h"

#include "ex_settingstab.h"
#include "ex_types.h"

#include "ctre.hpp"

#include "file.h"

#include "utils.h"

#include "drill/drill_form.h"

#include <QComboBox>
#include <QJsonObject>
#include <variant>

namespace Excellon {

Plugin::Plugin(QObject* parent)
    : FilePlugin(parent)
    , Parser(this) {
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_) {
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    QTextStream in(&file);
    Parser::parseFile(fileName);
    return Parser::file;
}

std::any Plugin::createPreviewGi(FileInterface* file, GCodePlugin* plugin, std::any param) {
    if (plugin->type() == ::GCode::Drill) {
        DrillPlugin::Preview retData;
        auto const exFile = static_cast<File*>(file);
        for (const Excellon::Hole& hole : *exFile) {
            auto name {QString("T%1").arg(hole.state.toolId)};
            if (bool slot = hole.state.path.size(); slot)
                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(exFile->transform().map(hole.state.path));
            else
                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(exFile->transform().map(hole.state.pos));
        }
        return retData;
    }
    return {};
}

bool Plugin::thisIsIt(const QString& fileName) {
    if (fileName.endsWith(".dxf", Qt::CaseInsensitive))
        return false;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    QString line;

    static constexpr ctll::fixed_string regex1(R"(^T(\d+)(?:([CFS])(\d*\.?\d+))?(?:([CFS])(\d*\.?\d+))?(?:([CFS])(\d*\.?\d+))?.*$)");
    static constexpr ctll::fixed_string regex2(R"(.*Holesize.*)"); // fixed_string(".*Holesize.*");

    while (in.readLineInto(&line)) {
        auto data {toU16StrView(line)};
        if (ctre::match<regex1>(data))
            return true;
        if (ctre::match<regex2>(data))
            return true;
    }

    return false;
}

int Plugin::type() const { return int(FileType::Excellon); }

QString Plugin::folderName() const { return tr("Excellon"); }

FileInterface* Plugin::createFile() { return new File(); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'E'); }

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent) {
    auto tab = new ExSettingsTab(parent);
    tab->setWindowTitle("Excellon");
    return tab;
}

void Plugin::addToGcForm(FileInterface* file, QComboBox* cbx) {
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

} // namespace Excellon
