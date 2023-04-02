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
#include "pocketoffset_form.h"
#include "pocketoffset.h"
#include "ui_pocketoffsetform.h"

#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>

namespace PocketOffset {

enum {
    Offset,
    Raster,
};

Form::
    Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new ::Ui::PocketOffsetForm)
    , names {tr("Pocket On"), tr("Pocket Outside"), tr("Pocket Inside")} {
    ui->setupUi(content);
    ui->toolHolder1->label()->setText("Tool 1:");
    ui->toolHolder2->label()->setText("Tool 2:");
    ui->toolHolder3->label()->setText("Tool 3:");
    ui->toolHolder4->label()->setText("Tool 4:");

    setWindowTitle(tr("Pocket Offset Toolpath"));

    MySettings settings;
    settings.beginGroup("Form");
    settings.getValue(ui->sbxToolQty);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->sbxSteps);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->sbxToolQty, qOverload<int>(&QSpinBox::valueChanged), this, &Form::rb_clicked);

    connect(ui->toolHolder1, &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder2, &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder3, &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->toolHolder4, &ToolSelectorForm::updateName, this, &Form::updateName);

    connect(leName, &QLineEdit::textChanged, this, &Form::onNameTextChanged);

    connect(ui->sbxSteps, &QSpinBox::valueChanged, this, &Form::onSbxStepsValueChanged);

    //
    if (ui->sbxSteps->value() == 0)
        ui->sbxSteps->setSuffix(tr(" - Infinity"));
}

Form::~Form() {

    MySettings settings;
    settings.beginGroup("Form");
    settings.setValue(ui->sbxToolQty);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->sbxSteps);
    settings.endGroup();
    delete ui;
}

void Form::сomputePaths() {
    const Tool tool[] {
        ui->toolHolder1->tool(),
        ui->toolHolder2->tool(),
        ui->toolHolder3->tool(),
        ui->toolHolder4->tool()};

    for (const Tool& t : tool) {
        if (!t.isValid()) {
            t.errorMessageBox(this);
            return;
        }
    }

    //    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
    //        tool2.errorMessageBox(this);
    //        return;
    //    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;
    bool skip {true};

    for (auto* gi : App::graphicsView()->selectedItems<GraphicsItem>()) {
        switch (gi->type()) {
        case GiType::DataSolid:
            wPaths.append(gi->paths());
            break;
        case GiType::DataPath: {
            auto paths = gi->paths();
            if (paths.front() == paths.back())
                wPaths.append(paths);
            else
                wRawPaths.append(paths);
        } break;
            //            if (!file) {
            //                file = gi->file();
            //                boardSide = file->side();
            //            } else if (file != gi->file()) {
            //                if (skip) {
            //                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
            //                        return;
            //                }
            //            }
            //            if (gi->type() == GiType::DataSolid)
            //                wPaths.append(gi->paths());
            //            else
            //                wRawPaths.append(gi->paths());
            //            break;
        case GiType::ShCircle:
        case GiType::ShRectangle:
        case GiType::ShText:
        case GiType::Drill:
            wPaths.append(gi->paths());
            break;
        case GiType::ShPolyLine:
        case GiType::ShCirArc:
            wRawPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    //    for (auto* item : App::graphicsView()->selectedItems()) {
    //        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
    //        switch (item->type()) {
    //        case GiType::DataSolid:
    //        case GiType::DataPath:
    //            if (!file) {
    //                file = gi->file();
    //                boardSide = file->side();
    //            } else if (file != gi->file()) {
    //                if (skip) {
    //                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
    //                        return;
    //                }
    //            }
    //            if (item->type() == GiType::DataSolid)
    //                wPaths.append(gi->paths());
    //            else
    //                wRawPaths.append(gi->paths());
    //            break;
    //        case GiType::ShCircle:
    //        case GiType::ShRectangle:
    //        case GiType::ShPolyLine:
    //        case GiType::ShCirArc:
    //        case GiType::ShText:
    //            wRawPaths.append(gi->paths());
    //            break;
    //        case GiType::Drill:
    //            wPaths.append(gi->paths());
    //            break;
    //        default:
    //            break;
    //        }
    //        addUsedGi(gi);
    //    }

    if (wRawPaths.empty() && wPaths.empty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::Params gcp_;
    for (const Tool& t : tool) {
        gcp_.tools.push_back(t);
        if (gcp_.tools.size() == static_cast<size_t>(ui->sbxToolQty->value()))
            break;
    }

    { // sort and unique
        auto cmp = [this](const Tool& t1, const Tool& t2) -> bool {
            return t1.getDiameter(dsbxDepth->value()) > t2.getDiameter(dsbxDepth->value());
        };
        auto cmp2 = [this](const Tool& t1, const Tool& t2) -> bool {
            return qFuzzyCompare(t1.getDiameter(dsbxDepth->value()), t2.getDiameter(dsbxDepth->value()));
        };
        std::sort(gcp_.tools.begin(), gcp_.tools.end(), cmp);
        auto last = std::unique(gcp_.tools.begin(), gcp_.tools.end(), cmp2);
        gcp_.tools.erase(last, gcp_.tools.end());
    }

    gcp_.setConvent(ui->rbConventional->isChecked());
    gcp_.setSide(side);
    gcp_.params[GCode::Params::Depth] = dsbxDepth->value();
    if (ui->sbxSteps->isVisible())
        gcp_.params[Creator::OffsetSteps] = ui->sbxSteps->value();

    creator->setGcp(gcp_);
    creator->addPaths(std::move(wPaths));
    creator->addRawPaths(wRawPaths);
    fileCount = static_cast<int>(gcp_.tools.size());
    createToolpath();
}

void Form::onSbxStepsValueChanged(int arg1) {
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : "");
}

void Form::updateName() {
    leName->setText(names[side]);
}

void Form::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void Form::rb_clicked() {
    const auto tool {ui->toolHolder1->tool()};

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    //    if (tool.type() == Tool::Laser)
    //        ui->chbxUseTwoTools->setChecked(false);

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    {
        int fl = ui->sbxToolQty->value();

        //        ui->labelSteps->setVisible(fl < 2);
        //        ui->sbxSteps->setVisible(fl < 2);

        ui->toolHolder2->setVisible(--fl > 0);
        ui->toolHolder3->setVisible(--fl > 0);
        ui->toolHolder4->setVisible(--fl > 0);
    }

    updateName();
    updateButtonIconSize();

    updatePixmap();
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::editFile(GCode::File* /*file*/) { }

} // namespace PocketOffset

#include "moc_pocketoffset_form.cpp"
