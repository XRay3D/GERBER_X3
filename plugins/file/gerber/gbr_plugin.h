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

#include "gbr_parser.h"

#include "abstract_fileplugin.h"

#include <QObject>
#include <QStack>

namespace Gerber {

class Plugin : public AbstractFilePlugin, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "gerber.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    Plugin(QObject* parent = nullptr);

    bool thisIsIt(const QString& fileName) override;

    uint32_t type() const override { return GERBER; }
    QString folderName() const override { return tr("Gerber Files"); }

    AbstractFile* loadFile(QDataStream& stream) override;

    QIcon icon() const override;
    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    void addToGcForm(AbstractFile* file, QComboBox* cbx) override;

    // public slots:
    AbstractFile* parseFile(const QString& fileName, int type) override;

    // AbstractFilePlugin interface
    //    std::any createPreviewGi(AbstractFile* file, GCode::Plugin* plugin, std::any param = {}) override;
};

} // namespace Gerber
