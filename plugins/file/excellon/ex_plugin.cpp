// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#include "ex_plugin.h"
#include "ex_file.h"
#include "ex_node.h"
#include "ex_settingstab.h"
#include "ex_types.h"

#include "app.h"
#include "ctre.hpp"
#include "doublespinbox.h"
#include "drillitem.h"
#include "drillpreviewgi.h"
#include "file.h"
#include "ft_view.h"
#include "utils.h"

#include "drill/gc_drillform.h"

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

std::any Plugin::createPreviewGi(FileInterface* file, GCodePlugin* plugin) {

    HoleMap retData;

    auto const exFile = reinterpret_cast<File*>(file);

    for (const Excellon::Hole& hole : *exFile) {
        if (bool slot = hole.state.path.size(); slot)
            retData[{ hole.state.tCode, exFile->tools()[hole.state.tCode], slot }].emplace_back(&hole.state.path);
        else
            retData[{ hole.state.tCode, exFile->tools()[hole.state.tCode], slot }].emplace_back(hole.state.pos);
    }

    //    if (0) {
    //        using DrillPreviewGiMap = std::map<int, mvector<std::shared_ptr<AbstractDrillPrGI>>>;

    //        DrillPreviewGiMap giPeview;

    //        auto const exFile = reinterpret_cast<File*>(file);

    //        std::map<int, mvector<const Excellon::Hole*>> cacheHoles;
    //        for (const Excellon::Hole& hole : *exFile)
    //            cacheHoles[hole.state.tCode] << &hole;

    //        data.reserve(cacheHoles.size()); // !!! reserve для отсутствия реалокаций, так как DrillPrGI хранит ссылки на него !!!

    //        for (auto [toolNum, diameter] : exFile->tools()) {
    //            QString name(tr("Tool Ø%1mm").arg(diameter));
    //            data.emplace_back(std::move(name), drawDrillIcon(), toolNum, diameter);
    //            for (const Excellon::Hole* hole : cacheHoles[toolNum]) {
    //                if (!hole->state.path.isEmpty())
    //                    data.back().isSlot = true;
    //                giPeview[toolNum].emplace_back(std::make_shared<DrillPrGI>(hole, data.back()));
    //            }
    //        }
    //    }

    return retData;
}

bool Plugin::thisIsIt(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    QString line;

    static constexpr ctll::fixed_string regex1(R"(^T(\d+))"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"(.*$)");
    static constexpr ctll::fixed_string regex2(R"(.*Holesize.*)"); // fixed_string(".*Holesize.*");

    while (in.readLineInto(&line)) {
        auto data { toU16StrView(line) };
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

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx) {
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

} // namespace Excellon
