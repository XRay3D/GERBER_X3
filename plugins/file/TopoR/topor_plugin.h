/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "abstract_fileplugin.h"
#include "topor_types.h"

#include <QObject>

namespace TopoR {

class Plugin : public AbstractFilePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "description.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    bool thisIsIt(const QString& fileName) override;
    uint32_t type() const override { return TOPOR; }
    QString folderName() const override { return tr("TopoR"); }
    AbstractFile* loadFile(std::string_view json) const override;
    std::string_view typeName() const override;
    QIcon icon() const override;
    void updateFileModel(AbstractFile* file) override;
    QString extension() const override { return tr("TopoR (*.fst)"); }

    // public slots:
    AbstractFile* parseFile(const QString& fileName, uint32_t type) override;
};

} // namespace TopoR
