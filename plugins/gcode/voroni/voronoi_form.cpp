// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

#include "voronoi_form.h"
#include "ui_voronoiform.h"
#include "voronoi.h"

#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>

namespace Voronoi {

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::VoronoiForm) {
    ui->setupUi(content);

    setWindowTitle(tr("Voronoi Toolpath"));

    ui->cbxSolver->setCurrentIndex(-1);

    MySettings settings;
    settings.beginGroup("VoronoiForm");
    settings.getValue(ui->dsbxPrecision, 0.1);
    settings.getValue(ui->dsbxWidth);
    settings.getValue(ui->dsbxOffset, 1.0);
    settings.getValue(ui->cbxSolver);
#ifdef _USE_CGAL_
#else
    //    ui->cbxSolver->setCurrentIndex(0);
    //    ui->cbxSolver->setEnabled(false);
#endif
    settings.endGroup();

    connect(dsbxDepth, &DepthForm::valueChanged, this, &Form::setWidth);
    connect(leName, &QLineEdit::textChanged, this, &Form::onNameTextChanged);
    //
    connect(ui->dsbxWidth, &QDoubleSpinBox::valueChanged, this, &Form::setWidth);
    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &Form::updateName);

    updateName();
    updateButtonIconSize();
}

Form::~Form() {
    MySettings settings;
    settings.beginGroup("VoronoiForm");
    settings.setValue(ui->dsbxPrecision);
    settings.setValue(ui->dsbxWidth);
    settings.setValue(ui->cbxSolver);
    settings.setValue(ui->dsbxOffset);
    settings.endGroup();
    delete ui;
}

void Form::computePaths() {
    const auto tool{ui->toolHolder->tool()};
    if(!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    auto gcp = getNewGcp();
    if(!gcp)
        return;

    gcp->setConvent(true);
    gcp->setSide(GCode::Outer);
    gcp->tools.push_back(tool);
    gcp->params[GCode::Params::Depth] = dsbxDepth->value();
    gcp->params[FrameOffset] = ui->dsbxOffset->value();
    gcp->params[Tolerance] = ui->dsbxPrecision->value();
    gcp->params[VoronoiType] = ui->cbxSolver->currentIndex();
    gcp->params[Width] = ui->dsbxWidth->value() + 0.001;

    createToolpath(gcp);
}

void Form::updateName() {
    leName->setText(tr("Voronoi"));
    setWidth(0.0);
}

void Form::onNameTextChanged(const QString& arg1) {
    fileName_ = arg1;
}

void Form::setWidth(double) {
    const auto tool{ui->toolHolder->tool()};
    const double d = tool.getDiameter(dsbxDepth->value());
    if(ui->dsbxWidth->value() > 0.0 && (qFuzzyCompare(ui->dsbxWidth->value(), d) || ui->dsbxWidth->value() < d)) {
        QMessageBox::warning(this, tr("Warning"), tr("The width must be larger than the tool diameter!"));
        ui->dsbxWidth->setValue(d + 0.05);
    }
}

void Form::editFile(GCode::File* /*file*/) {
}

void Form::on_cbxSolver_currentIndexChanged(int index) {
    ui->label_4->setVisible(index);
    ui->dsbxPrecision->setVisible(index);
}

} // namespace Voronoi

#include "moc_voronoi_form.cpp"
