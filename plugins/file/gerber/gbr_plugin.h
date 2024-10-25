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
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "description.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    [[nodiscard]] AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    [[nodiscard]] QString folderName() const override { return tr("Gerber Files"); }
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] AbstractFile* loadFile(QDataStream& stream) const override;
    [[nodiscard]] bool thisIsIt(const QString& fileName) override;
    [[nodiscard]] uint32_t type() const override { return GERBER; }

    // public slots:
    [[nodiscard]] AbstractFile* parseFile(const QString& fileName, int type) override;
};

} // namespace Gerber
