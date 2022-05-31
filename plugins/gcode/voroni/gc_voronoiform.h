/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once
#include "gc_formsutil.h"

namespace Ui {
class VoronoiForm;
}

class VoronoiForm : public FormsUtil {
    Q_OBJECT

public:
    explicit VoronoiForm(GCodePlugin* plugin, QWidget* parent = nullptr);
    ~VoronoiForm() override;

private slots:
    void on_leName_textChanged(const QString& arg1);
    void on_cbxSolver_currentIndexChanged(int index);

private:
    Ui::VoronoiForm* ui;

    double m_size = 0.0;
    double m_lenght = 0.0;
    void setWidth(double w);

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#include "gc_plugin.h"
#include <QToolBar>

class GCPluginImpl : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "voronoi.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    int type() const override { return GCode::Voronoi; }
    QWidget* createForm()  override { return new VoronoiForm(this); };
    QKeySequence keySequence() const override { return { "Ctrl+Shift+V" }; }
    QIcon icon() const override { return QIcon::fromTheme("voronoi-path"); }
};
