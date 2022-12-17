/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "gc_formsutil.h"

namespace Ui {
class PocketRasterForm;
}

class PocketRasterForm : public GcFormBase {
    Q_OBJECT

public:
    explicit PocketRasterForm(GCodePlugin* plugin, QWidget* parent = nullptr);
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
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "gc_plugin.h"
#include <QToolBar>

class GCPluginImpl final : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "pocketraster.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("raster-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+R"}; }
    QWidget* createForm() override { return new PocketRasterForm(this); };
    int type() const override { return GCode::Raster; }
};
