/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "interfaces/abstract_fileplugin.h"

#include "hpgl_parser.h"

#include <QObject>
#include <QStack>

namespace Hpgl {

class File;

class Plugin : public AbstractFilePlugin, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "hpgl.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    bool thisIsIt(const QString& fileName) override;

    uint32_t type() const override;
    QString folderName() const override;

    AbstractFile* loadFile(QDataStream& stream) constoverride;

    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    void updateFileModel(AbstractFile* file) override;

public slots:
    AbstractFile* parseFile(const QString& fileName, int type) override;

private:
    File* file = nullptr;
};

} // namespace Hpgl
