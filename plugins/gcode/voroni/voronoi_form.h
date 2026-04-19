/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_form.h"
#include "voronoi.h"
#include <QToolBar>

namespace Ui {
class VoronoiForm;
}
namespace Voronoi {

class Form : public GCode::Form {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin);
    ~Form() override;

private slots:
    void onNameTextChanged(const QString& arg1);
    void on_cbxSolver_currentIndexChanged(int index);

private:
    Ui::VoronoiForm* ui;

    double size_{};
    double lenght_{};
    void setWidth(double w);

    // FormsUtil interface
protected:
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class Plugin final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "description.json")
    Q_INTERFACES(GCode::Plugin)
    Form form{this};
    // GCode::Plugin interface
public:
    uint32_t type() const override { return VORONOI; }
    QWidget* createForm() override { return &form; }
    QString gcName() const override { return u"Voronoi"_s; };
    QKeySequence keySequence() const override { return {u"Ctrl+Shift+V"_s}; }
    QIcon icon() const override { return QIcon::fromTheme(u"voronoi-path"_s); }
    AbstractFile* loadFile(QDataStream& stream) const override { return File::load<File>(stream); }
};

} // namespace Voronoi
