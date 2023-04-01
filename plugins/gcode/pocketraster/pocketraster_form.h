/*******************************************************************************
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
#include "gc_baseform.h"

namespace Ui {
class PocketRasterForm;
}

class PocketRasterForm : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit PocketRasterForm(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~PocketRasterForm();

private slots:
    void onNameTextChanged(const QString& arg1);

private:
    Ui::PocketRasterForm* ui;

    int direction = 0;
    void updatePixmap();
    void rb_clicked();
    const QStringList names;
    static inline const QString pixmaps[] {
        QStringLiteral("pock_rast_climb"),
        QStringLiteral("pock_rast_conv"),
    };
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    // FormsUtil interface
protected:
    void сomputePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "file.h"
#include "gc_plugin.h"
#include <QToolBar>

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "pocketraster.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("raster-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+R"}; }
    QWidget* createForm() override { return new PocketRasterForm(this); };
    uint32_t type() const override { return GCode::Raster; }
    AbstractFile* loadFile(QDataStream& stream) const override { return new GCode::PocketRasterFile; }
};
