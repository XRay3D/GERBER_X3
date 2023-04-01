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
#include <QToolBar>

#include "gc_formsutil.h"
#include "gc_plugin.h"
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

class Form : public GCode::FormBase {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
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

    void createTPI(AbstractFile* file);

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
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thermal.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("thermal-path"); }
    QKeySequence keySequence() const override { return {"Ctrl+Shift+T"}; }
    QWidget* createForm() override { return new Form(this); };
    bool canToShow() const override { return Form::canToShow(); }
    uint32_t type() const override { return GCode::Thermal; }
    AbstractFile* loadFile(QDataStream& stream) const override { return new GCode::ThermalFile; }
};

} // namespace Thermal
