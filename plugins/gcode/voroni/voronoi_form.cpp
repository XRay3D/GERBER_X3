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
    const auto tool {ui->toolHolder->tool()};
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;
    bool skip {true};

    auto testFile = [&file, &skip, this](GraphicsItem* gi) -> bool {
        if (!file) {
            file = gi->file();
            boardSide = gi->file()->side();
        }
        if (file != gi->file()) {
            if (skip) {
                if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                    return true;
            }
        }
        return {};
    };

    for (auto* item : App::graphicsView()->selectedItems()) {
        auto gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiType::DataSolid:
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiType::DataPath:
            if (testFile(gi))
                return;
            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiType::Drill:
            if (testFile(gi))
                return;
            wPaths.append(static_cast<GraphicsItem*>(item)->paths(1));
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wPaths.empty() && wRawPaths.empty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    auto gpc = new GCode::Params;
    gpc->setConvent(true);
    gpc->setSide(GCode::Outer);
    gpc->tools.push_back(tool);
    gpc->params[GCode::Params::Depth] = dsbxDepth->value();
    gpc->params[FrameOffset] = ui->dsbxOffset->value();
    gpc->params[Tolerance] = ui->dsbxPrecision->value();
    gpc->params[VoronoiType] = ui->cbxSolver->currentIndex();
    gpc->params[Width] = ui->dsbxWidth->value() + 0.001;

    gpc->closedPaths = std::move(wPaths);
    gpc->openPaths = wRawPaths;
    createToolpath(gpc);
}

void Form::updateName() {
    leName->setText(tr("Voronoi"));
    setWidth(0.0);
}

void Form::onNameTextChanged(const QString& arg1) {
    fileName_ = arg1;
}

void Form::setWidth(double) {
    const auto tool {ui->toolHolder->tool()};
    const double d = tool.getDiameter(dsbxDepth->value());
    if (ui->dsbxWidth->value() > 0.0 && (qFuzzyCompare(ui->dsbxWidth->value(), d) || ui->dsbxWidth->value() < d)) {
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
