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

#include <string_view>
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
        QTransform t {exFile->transform()};
        for (const Excellon::Hole& hole : *exFile) {
            auto name {QString("T%1").arg(hole.state.toolId)};
            if (bool slot = hole.state.path.size(); slot)
                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.path));
            else
                retData[{hole.state.toolId, exFile->tools()[hole.state.toolId], slot, name}].posOrPath.emplace_back(t.map(hole.state.pos));
        }
        return retData;
    }
    return {};
}

bool Plugin::thisIsIt(const QString& fileName) {
    if (!fileName.endsWith(".drl", Qt::CaseInsensitive))
        return false;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    auto data {file.readAll().trimmed()};

    // return data.startsWith("M48") && data.endsWith("M30");

    int toolCount {};
    for (auto [whole, var] : ctre::multiline_range<R"(^(T\d+).*)">(data))
        if (var)
            if (++toolCount >= 2) //++toolCount;
                break;

    return data.startsWith("M48") && data.endsWith("M30") && toolCount >= 2;
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

#include "moc_ex_plugin.cpp"
