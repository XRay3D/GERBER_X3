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

#include "gc_plugin.h"
#include "hatching.h"
#include <QToolBar>

namespace Ui {
class HatchingForm;
}

namespace CrossHatch {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form();

private slots:
    void onNameTextChanged(const QString& arg1);

private:
    Ui::HatchingForm* ui;

    int direction = 0;
    void updatePixmap();
    void rb_clicked();
    const QStringList names;
    const QStringList pixmaps;
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
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "hatching.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("crosshatch-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+C"}; }
    QWidget* createForm() override { return /*new Form(this);*/ };
    uint32_t type() const override { return CROSS_HATCH; }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace CrossHatch
