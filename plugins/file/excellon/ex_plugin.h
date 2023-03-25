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
#pragma once

#include "ex_parser.h"

#include "file_plugin.h"

#include <QObject>
#include <QStack>

namespace Excellon {

class Plugin : public FilePlugin, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "excellon.json")
    Q_INTERFACES(FilePlugin)

public:
    Plugin(QObject* parent = nullptr);

    bool thisIsIt(const QString& fileName) override;

    int type() const override;
    QString folderName() const override;

    FileInterface* createFile() override;
    QIcon icon() const override;
    SettingsTabInterface* createSettingsTab(QWidget* parent) override;
    void addToGcForm(FileInterface* file, QComboBox* cbx) override;
    //    DrillPreviewGiMap createDrillPreviewGi(FileInterface* file, mvector<Row>& data) override;

    // public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

    // FilePlugin interface
    std::any createPreviewGi(FileInterface* file, GCodePlugin* plugin, std::any param = {}) override;
};

} // namespace Excellon
