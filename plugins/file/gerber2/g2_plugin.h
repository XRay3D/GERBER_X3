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
#pragma once

#include "abstract_fileplugin.h"
#include "g2_file.h"

namespace Gerber2 {

class Plugin : public AbstractFilePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "description.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    [[nodiscard]] AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    [[nodiscard]] QString folderName() const override { return tr("Gerber Files (editable)"); }
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] AbstractFile* loadFile(QDataStream& stream) const override;
    [[nodiscard]] bool thisIsIt(const QString& fileName) override;
    [[nodiscard]] uint32_t type() const override { return GERBER2; }
    [[nodiscard]] QString extension() const override { return tr("Gerber (*.gbr)"); }

    AbstractFile* parseFile(const QString& fileName, uint32_t type) override;
};

} // namespace Gerber2
