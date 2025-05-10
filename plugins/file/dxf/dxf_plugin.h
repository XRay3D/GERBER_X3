/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "abstract_fileplugin.h"
#include "dxf_types.h"

#include <QObject>
#include <QStack>

namespace Dxf {

class File;

class Plugin : public AbstractFilePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "description.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    bool thisIsIt(const QString& fileName) override;
    uint32_t type() const override;
    QString folderName() const override { return tr("Dxf Files"); }
    AbstractFile* loadFile(QDataStream& stream) const override;
    QIcon icon() const override;
    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    void updateFileModel(AbstractFile* file) override;
    // public slots:
    AbstractFile* parseFile(const QString& fileName, uint32_t type) override;

private:
    File* file_ = nullptr;
};

} // namespace Dxf
