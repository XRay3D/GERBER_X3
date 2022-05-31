/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once

#include "gbr_parser.h"

#include "file_plugin.h"

#include <QObject>
#include <QStack>

namespace Gerber {

class Plugin : public FilePlugin, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "gerber.json")
    Q_INTERFACES(FilePlugin)

public:
    Plugin(QObject* parent = nullptr);

    bool thisIsIt(const QString& fileName) override;

    int type() const override;
    QString folderName() const override;

    FileInterface* createFile() override;

    QIcon icon() const override;
    SettingsTabInterface* createSettingsTab(QWidget* parent) override;
    void addToDrillForm(FileInterface* file, QComboBox* cbx) override;
    // FIXME   DrillPreviewGiMap createDrillPreviewGi(FileInterface* file, mvector<Row>& data) override;
    // FIXME   ThermalPreviewGiMap createThermalPreviewGi(FileInterface* file, const ThParam2& tp2) override;

    // public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;
};
} // namespace Gerber
