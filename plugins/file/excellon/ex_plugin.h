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

    bool thisIsIt(const QString& fileName) override;

    uint32_t type() const override;
    QString folderName() const override;

    AbstractFile* loadFile(QDataStream& stream) const override;
    QIcon icon() const override;
    AbstractFileSettings* createSettingsTab(QWidget* parent) override;

    // public slots:
    AbstractFile* parseFile(const QString& fileName, int type) override;
};

} // namespace Excellon
