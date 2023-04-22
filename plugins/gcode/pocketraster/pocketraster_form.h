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
#include "gc_plugin.h"
#include "pocketraster.h"
#include <QToolBar>

namespace Ui {
class PocketRasterForm;
}

namespace PocketRaster {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form();

private slots:
    void onNameTextChanged(const QString& arg1);

private:
    Ui::PocketRasterForm* ui;

    int direction = 0;
    void updatePixmap();
    void rb_clicked();
    const QStringList names;
    static inline const QString pixmaps[]{
        QStringLiteral("pock_rast_climb"),
        QStringLiteral("pock_rast_conv"),
    };
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    // FormsUtil interface
protected:
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "pocketraster.json")
    Q_INTERFACES(GCode::Plugin)
    Form form{this};
    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("raster-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+R"}; }
    QWidget* createForm() override { return &form; };
    uint32_t type() const override { return POCKET_RASTER; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace PocketRaster
