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

#include "abstract_fileplugin.h"

#include <QObject>
#include <QStack>

namespace Excellon {

class Plugin : public AbstractFilePlugin, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "description.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    Plugin(QObject* parent = nullptr);

    [[nodiscard]] bool thisIsIt(const QString& fileName) override;

    [[nodiscard]] uint32_t type() const override;
    [[nodiscard]] QString folderName() const override;

    [[nodiscard]] AbstractFile* loadFile(std::string_view json) const override;
    [[nodiscard]] std::string_view typeName() const override;
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    [[nodiscard]] virtual QString extension() const override { return tr("Excellon (*.exc *.drl)"); }

    // public slots:
    AbstractFile* parseFile(const QString& fileName, uint32_t type) override;
};

} // namespace Excellon
