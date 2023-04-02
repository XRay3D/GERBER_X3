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
#include "voronoi.h"
#include <QToolBar>

namespace Ui {
class VoronoiForm;
}
namespace Voronoi {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

private slots:
    void onNameTextChanged(const QString& arg1);
    void on_cbxSolver_currentIndexChanged(int index);

private:
    Ui::VoronoiForm* ui;

    double size_ = 0.0;
    double lenght_ = 0.0;
    void setWidth(double w);

    // FormsUtil interface
protected:
    void сomputePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "voronoi.json")
    Q_INTERFACES(GCode::Plugin)

    // GCode::Plugin interface
public:
    uint32_t type() const override { return VORONOI; }
    QWidget* createForm() override { return new Form(this); };
    QKeySequence keySequence() const override { return {"Ctrl+Shift+V"}; }
    QIcon icon() const override { return QIcon::fromTheme("voronoi-path"); }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Voronoi
