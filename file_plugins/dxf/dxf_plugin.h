/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "pluginfile.h"

#include <QObject>
#include <QStack>

namespace Dxf {

class File;

class Plugin : public QObject, public FilePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ParserInterface_iid FILE "dxf.json")
    Q_INTERFACES(FilePlugin)

public:
    explicit Plugin(QObject* parent = nullptr);

    // FilePlugin interface
    bool thisIsIt(const QString& fileName) override;
    QObject* getObject() override;
    int type() const override;
    QString folderName() const override;

    FileInterface* createFile() override;
    QJsonObject info() const override;
    QIcon icon() const override;
    SettingsTabInterface* createSettingsTab(QWidget* parent) override;
    void updateFileModel(FileInterface* file) override;
    DrillPreviewGiMap createDrillPreviewGi(FileInterface* file, mvector<Row>& data) override;
    void addToDrillForm(FileInterface* file, QComboBox* cbx);
public slots:
    FileInterface* parseFile(const QString& fileName, int type) override;

signals:
    void fileReady(FileInterface* file) override;
    void fileProgress(const QString& fileName, int max, int value) override;
    void fileError(const QString& fileName, const QString& error) override;

private:
    File* m_file = nullptr;
    QIcon drawDrillIcon(QColor color);
};

} // namespace Dxf
