// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "voronoiform.h"
#include "ui_voronoiform.h"

#include "scene.h"
#include "settings.h"
#include <QMessageBox>

#include "leakdetector.h"

VoronoiForm::VoronoiForm(QWidget* parent)
    : FormsUtil(new GCode::VoronoiCreator, parent)
    , ui(new Ui::VoronoiForm)
{
    ui->setupUi(this);

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    parent->setWindowTitle(ui->label->text());

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("VoronoiForm");
    settings.getValue(ui->dsbxPrecision, 0.1);
    settings.getValue(ui->dsbxWidth);
    settings.getValue(ui->dsbxOffset, 1.0);
#ifdef _USE_CGAL_
    settings.getValue(ui->cbxSolver);
#else
    ui->cbxSolver->setCurrentIndex(0);
    ui->cbxSolver->setEnabled(false);
#endif
    settings.endGroup();

    connect(ui->pbCreate, &QPushButton::clicked, this, &VoronoiForm::createFile);
    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &VoronoiForm::updateName);

    connect(ui->dsbxDepth, &DepthForm::valueChanged, this, &VoronoiForm::setWidth);
    connect(ui->dsbxWidth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &VoronoiForm::setWidth);

    updateName();
}

VoronoiForm::~VoronoiForm()
{
    MySettings settings;
    settings.beginGroup("VoronoiForm");
    settings.setValue(ui->dsbxPrecision);
    settings.setValue(ui->dsbxWidth);
    settings.setValue(ui->cbxSolver);
    settings.setValue(ui->dsbxOffset);
    settings.endGroup();
    delete ui;
}

void VoronoiForm::createFile()
{
    const auto tool { ui->toolHolder->tool() };
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    FileInterface const* file = nullptr;
    bool skip { true };

    for (auto* item : App::scene()->selectedItems()) {
        auto* gi = dynamic_cast<GraphicsItem*>(item);
        switch (static_cast<GiType>(item->type())) {
        case GiType::DataSolid:
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            wPaths.push_back(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiType::DataPath:
            //RawItem* gi = static_cast<RawItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            wRawPaths.push_back(static_cast<GraphicsItem*>(item)->paths());
            break;
            //        case DrillItemType:
            //            //            if (static_cast<DrillItem*>(item)->isSlot())
            //            //                wrPaths.push_back(static_cast<GraphicsItem*>(item)->paths());
            //            //            else
            //            wPaths.push_back(static_cast<GraphicsItem*>(item)->paths());
            //            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gpc;
    gpc.setConvent(true);
    gpc.setSide(GCode::Outer);
    gpc.tools.push_back(tool);
    gpc.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    gpc.params[GCode::GCodeParams::Tolerance] = ui->dsbxPrecision->value();
    gpc.params[GCode::GCodeParams::Width] = ui->dsbxWidth->value() + 0.001;
    gpc.params[GCode::GCodeParams::VorT] = ui->cbxSolver->currentIndex();
    gpc.params[GCode::GCodeParams::FrameOffset] = ui->dsbxOffset->value();

    m_tpc->setGcp(gpc);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    createToolpath();
}

void VoronoiForm::updateName()
{
    ui->leName->setText(tr("Voronoi"));
    setWidth(0.0);
}

void VoronoiForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}

void VoronoiForm::setWidth(double)
{
    const auto tool { ui->toolHolder->tool() };
    const double d = tool.getDiameter(ui->dsbxDepth->value());
    if (ui->dsbxWidth->value() > 0.0 && (qFuzzyCompare(ui->dsbxWidth->value(), d) || ui->dsbxWidth->value() < d)) {
        QMessageBox::warning(this, tr("Warning"), tr("The width must be larger than the tool diameter!"));
        ui->dsbxWidth->setValue(d + 0.05);
    }
}

void VoronoiForm::editFile(GCode::File* /*file*/)
{
}

void VoronoiForm::on_cbxSolver_currentIndexChanged(int index)
{
    ui->dsbxPrecision->setEnabled(!index);
}
