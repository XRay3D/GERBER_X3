/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "gbrparser.h"

#include "interfaces/pluginfile.h"

#include <QObject>
#include <QStack>

namespace Gerber {

class Plugin : public QObject, public FilePluginInterface, Parser {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "gerber.json")
    Q_INTERFACES(FilePluginInterface)

public:
    Plugin(QObject* parent = nullptr);

    bool thisIsIt(const QString& fileName) override;
    QObject* getObject() override;
    int type() const override;

    std::shared_ptr<FileInterface> createFile() override;
    QJsonObject info() const override;
    std::pair<SettingsTabInterface*, QString> createSettingsTab(QWidget* parent) override;
    void addToDrillForm(FileInterface* file, QComboBox* cbx) override;
    DrillPreviewGiMap createDrillPreviewGi(FileInterface* file, mvector<Row>& data) override;
    ThermalPreviewGiVec createThermalPreviewGi(FileInterface* file, const ThParam2& param, Tool& tool) override;

public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileReady(FileInterface* file) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileError(const QString& fileName, const QString& error) override;

private:
    QIcon drawApertureIcon(AbstractAperture* aperture) const;
    QIcon drawRegionIcon(const GraphicObject& go) const;
};
}
