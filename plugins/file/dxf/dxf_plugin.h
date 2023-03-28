/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "abstract_fileplugin.h"

#include <QObject>
#include <QStack>

namespace Dxf {

class File;

class Plugin : public AbstractFilePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "dxf.json")
    Q_INTERFACES(AbstractFilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // AbstractFilePlugin interface
    bool thisIsIt(const QString& fileName) override;

    int type() const override;
    QString folderName() const override;

    AbstractFile* loadFile(QDataStream& stream) override;
    QIcon icon() const override;
    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    void updateFileModel(AbstractFile* file) override;
    // FIXME   DrillPreviewGiMap createDrillPreviewGi(AbstractFile* file, mvector<Row>& data) override;
    // FIXME   void addToGcForm(AbstractFile* file, QComboBox* cbx);

    // public slots:
    AbstractFile* parseFile(const QString& fileName, int type) override;
    // AbstractFilePlugin interface
    std::any createPreviewGi(AbstractFile* file, GCodePlugin* plugin, std::any param = {}) override;
    void addToGcForm(AbstractFile* file, QComboBox* cbx) override;

private:
    File* file_ = nullptr;
};

} // namespace Dxf
