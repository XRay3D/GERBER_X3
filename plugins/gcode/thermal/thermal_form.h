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
#include "thermal_vars.h"

class Model;
class QCheckBox;
class QItemSelection;

namespace Ui {
class ThermalForm;
}

namespace Gerber {
class AbstractAperture;
}

namespace Thermal {

class AbstractThermPrGi;

class Form : public GcFormBase {
    Q_OBJECT

public:
    explicit Form(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

    void updateFiles();
    static bool canToShow();

private slots:
    void onNameTextChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);
    void on_dsbxDepth_valueChanged(double arg1);

    void on_dsbxAreaMax_editingFinished();
    void on_dsbxAreaMin_editingFinished();

private:
    Ui::ThermalForm* ui;

    void createTPI(FileInterface* file);

    mvector<std::shared_ptr<AbstractThermPrGi>> items_;
    PreviewGiMap thPaths;

    Model* model = nullptr;
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void setSelection(const QModelIndex& selected, const QModelIndex& deselected);

    ThParam par;
    double lastMax;
    double lastMin;

    QCheckBox* chbx;
    double depth_;
    inline void redraw();
    Tool tool;

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
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thermal.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("thermal-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+T"}; }
    QWidget* createForm() override { return new Form(this); };
    bool canToShow() const override { return Form::canToShow(); }
    int type() const override { return GCode::Thermal; }
};

} // namespace Thermal
