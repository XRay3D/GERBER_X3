// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "tool_editform.h"
#include "ui_tooleditform.h"

#include "tool_item.h"
#include <QDebug>
#include <QSettings>

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ToolEditForm) {
    ui->setupUi(this);

    dsbx = {
        ui->dsbxAngle,
        ui->dsbxDiameter,
        ui->dsbxFeedRate,
        ui->dsbxOneTurnCut,
        ui->dsbxPassDepth,
        ui->dsbxPlungeRate,
        ui->dsbxSpindleSpeed,
        ui->dsbxStepover,
        ui->dsbxOneTurnCutPercent,
        ui->dsbxStepoverPercent,
    };
    for (DoubleSpinBox* pPsbx : dsbx) {
        connect(pPsbx, &QDoubleSpinBox::valueChanged, this, &ToolEditForm::valueChangedSlot);
    }
    connect(ui->cbxFeedSpeeds, &QComboBox::currentIndexChanged, [this](int index) {
        double tmpFeed = feed_;
        switch (index) {
        case msec_: // mm/sec
            feed_ = 1.0 / 60.0;
            break;
        case mmin_: // mm/min!!!
            feed_ = 1.0;
            break;
        case cmin_: // cm/min
            feed_ = 1.0 / 10.0;
            break;
        case min_: //  m/min
            feed_ = 1.0 / 1000.0;
            break;
        default:
            break;
        }
        bool fl1 = parentWidget()->isWindowModified();
        bool fl2 = ui->pbApply->isEnabled();
        ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / tmpFeed) * feed_);
        ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / tmpFeed) * feed_);
        ui->dsbxFeedRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        ui->dsbxPlungeRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        parentWidget()->setWindowModified(fl1);
        ui->pbApply->setEnabled(fl2);
    });
    connect(ui->cbxToolType, &QComboBox::currentIndexChanged, this, &ToolEditForm::setupToolWidgets);

    connect(ui->leName, &QLineEdit::textChanged, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, [this] { tool_.setNote(ui->teNote->toPlainText()); setChanged(); });

    ui->cbxUnits->setVisible(false);

    ui->cbxToolType->setItemIcon(Tool::Drill, QIcon::fromTheme("drill"));
    ui->cbxToolType->setItemIcon(Tool::EndMill, QIcon::fromTheme("endmill"));
    ui->cbxToolType->setItemIcon(Tool::Engraver, QIcon::fromTheme("engraving"));
    ui->cbxToolType->setItemIcon(Tool::Laser, QIcon::fromTheme("laser"));

    ui->lblWarn->setPixmap(QIcon::fromTheme("window-close").pixmap({16, 16}));
    ui->lblWarn->setToolTip(QApplication::translate("ToolEditForm", "If the offset value is more than 50%, unmilled areas are possible.\nThese errors do not appear in the visualization.", "При значении отступа более 50% возможны не отфрезерованные участки. Эти ошибки не отображаются в визуализации."));

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value("cbxFeedSpeeds").toInt());
    // ui->cbxUnits->setCurrentIndex(settings.value("cbxUnits").toInt());
    setVisibleToolWidgets(false);
    setChanged(false);
}

ToolEditForm::~ToolEditForm() {
    QSettings settings;
    settings.setValue("cbxFeedSpeeds", ui->cbxFeedSpeeds->currentIndex());
    // settings.setValue("cbxUnits", ui->cbxUnits->currentIndex());
    delete ui;
    ui = nullptr;
}

void ToolEditForm::setItem(ToolItem* item) {
    if (item == nullptr)
        return;
    item_ = item;
    if (item_->isTool()) {
        setTool(item_->tool());
        setVisibleToolWidgets(true);
    } else {
        ui->leName->setText(item_->name());
        ui->teNote->setText(item_->note());
        setVisibleToolWidgets(false);
    }
    setChanged(false);
}

void ToolEditForm::setTool(const Tool& tool) {
    tool_ = tool;
    setupToolWidgets(tool_.type());
    dsbx[Tool::Angle]->setValue(tool.angle());
    dsbx[Tool::Diameter]->setValue(tool.diameter());
    dsbx[Tool::FeedRate]->setValue(tool.feedRate() * feed_);
    dsbx[Tool::OneTurnCut]->setValue(tool.oneTurnCut());
    dsbx[Tool::PassDepth]->setValue(tool.passDepth());
    dsbx[Tool::PlungeRate]->setValue(tool.plungeRate() * feed_);
    dsbx[Tool::SpindleSpeed]->setValue(tool.spindleSpeed());
    dsbx[Tool::Stepover]->setValue(tool.stepover());

    ui->cbxToolType->setCurrentIndex(tool.type());
    ui->chbxAutoName->setChecked(tool.autoName());
    ui->leName->setText(tool.name());
    ui->teNote->setText(tool.note());
}

void ToolEditForm::setChanged(bool fl) {
    ui->pbApply->setEnabled(fl);
    parentWidget()->setWindowModified(fl);
}

void ToolEditForm::setVisibleToolWidgets(bool visible) {
    ui->cbxFeedSpeeds->setVisible(visible);
    ui->cbxToolType->setVisible(visible);
    ui->cbxUnits->setVisible(visible);
    ui->chbxAutoName->setVisible(visible);
    ui->grbxGeometry->setVisible(visible);
    ui->grbxFeedSpeeds->setVisible(visible);
    ui->grbxCuttingParameters->setVisible(visible);
    ui->lblToolType->setVisible(visible);
    ui->lblUnits->setVisible(visible);
    setMinimumWidth(width());
}

void ToolEditForm::setupToolWidgets(int type) {
    const int lastType = tool_.type();
    tool_.setType(type);

    auto setEnabled = [this, type](DoubleSpinBox* dsbx, double min, double max) {
        if (!dsbx->isEnabled()) {
            saveRestoreMap[type].restore(dsbx);
            dsbx->setEnabled(true);
        }
        if (min != 0.0 || max != 0.0)
            dsbx->setRange(min, max);
    };
    auto setDisabled = [this, lastType](DoubleSpinBox* dsbx) {
        if (dsbx->isEnabled()) {
            saveRestoreMap[lastType].save(dsbx);
            dsbx->setRange(0.0, 0.0);
            dsbx->setEnabled(false);
        }
    };

    switch (type) {
    case Tool::Drill:
        setDisabled(ui->dsbxFeedRate);
        setDisabled(ui->dsbxStepover);
        setDisabled(ui->dsbxStepoverPercent);
        setEnabled(ui->dsbxAngle, 90.0, 120.0);
        setEnabled(ui->dsbxOneTurnCut, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxOneTurnCutPercent, 0.0, 100.0);
        setEnabled(ui->dsbxPassDepth, 0.0, 10.0);
        setEnabled(ui->dsbxPlungeRate, 0.0, 100000.0);
        if (qFuzzyCompare(ui->dsbxAngle->value(), 90.0))
            ui->dsbxAngle->setValue(120.0); ////////////////////
        ui->lblPassDepth->setText(tr("Pass"));
        break;
    case Tool::EndMill:
        setDisabled(ui->dsbxAngle);
        setEnabled(ui->dsbxFeedRate, 0.0, 100000.0);
        setEnabled(ui->dsbxOneTurnCut, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxOneTurnCutPercent, 0.0, 100.0);
        setEnabled(ui->dsbxPassDepth, 0.0, 10.0);
        setEnabled(ui->dsbxPlungeRate, 0.0, 100000.0);
        setEnabled(ui->dsbxStepover, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxStepoverPercent, 0.0, 100.0);
        ui->lblPassDepth->setText(tr("Depth"));
        break;
    case Tool::Engraver:
        setEnabled(ui->dsbxAngle, 0.0, 180.0);
        setEnabled(ui->dsbxFeedRate, 0.0, 100000.0);
        setEnabled(ui->dsbxOneTurnCut, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxOneTurnCutPercent, 0.0, 100.0);
        setEnabled(ui->dsbxPassDepth, 0.0, 10.0);
        setEnabled(ui->dsbxPlungeRate, 0.0, 100000.0);
        setEnabled(ui->dsbxStepover, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxStepoverPercent, 0.0, 100.0);
        ui->lblPassDepth->setText(tr("Depth"));
        break;
    case Tool::Laser:
        setDisabled(ui->dsbxAngle);
        setDisabled(ui->dsbxOneTurnCut);
        setDisabled(ui->dsbxOneTurnCutPercent);
        setDisabled(ui->dsbxPassDepth);
        setDisabled(ui->dsbxPlungeRate);
        setEnabled(ui->dsbxFeedRate, 0.0, 100000.0);
        setEnabled(ui->dsbxStepover, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxStepoverPercent, 0.0, 100.0);
        break;
    }
    setChanged();
    updateName();
}

void ToolEditForm::valueChangedSlot(double value) {
    switch (dsbx.indexOf(dynamic_cast<DoubleSpinBox*>(sender()))) {
    case Tool::Angle:
        tool_.setAngle(value);
        break;
    case Tool::Diameter:
        tool_.setDiameter(value);
        ui->dsbxOneTurnCut->setMaximum(value);
        ui->dsbxStepover->setMaximum(value);
        if (ui->dsbxStepover->value() == 0.0) {
            ui->dsbxStepover->setValue(value * 0.5);
        }
        if (ui->dsbxOneTurnCut->value() == 0.0)
            ui->dsbxOneTurnCut->setValue(value * 0.1);
        ui->dsbxOneTurnCutPercent->valueChanged(ui->dsbxOneTurnCutPercent->value());
        ui->dsbxStepoverPercent->valueChanged(ui->dsbxStepoverPercent->value());
        break;
    case Tool::FeedRate:
        tool_.setFeedRate(value / feed_);
        break;
    case Tool::OneTurnCut:
        tool_.setOneTurnCut(value);
        ui->dsbxOneTurnCutPercent->setValue(tool_.diameter() > 0.0 ? value / (tool_.diameter() * 0.01) : 0.0);
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed_);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed_);
        break;
    case Tool::PassDepth:
        tool_.setPassDepth(value);
        break;
    case Tool::PlungeRate:
        tool_.setPlungeRate(value / feed_);
        break;
    case Tool::SpindleSpeed:
        tool_.setSpindleSpeed(value); // rpm
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed_);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed_);
        break;
    case Tool::Stepover:
        tool_.setStepover(value);
        ui->dsbxStepoverPercent->setValue(tool_.diameter() > 0.0 ? value / (tool_.diameter() * 0.01) : 0.0);
        break;
    case Tool::OneTurnCutPercent:
        ui->dsbxOneTurnCut->setValue(value * (tool_.diameter() * 0.01));
        break;
    case Tool::StepoverPercent:
        ui->dsbxStepover->setValue(value * (tool_.diameter() * 0.01));
        break;
    default:
        break;
    }

    const bool overflow = (ui->dsbxStepover->value() > (tool_.diameter() * 0.5));
    ui->lblWarn->setVisible(overflow);

    updateName();
    setChanged();
}

void ToolEditForm::on_pbApply_clicked() {
    if (item_ && tool_.isValid()) {
        item_->setName(tool_.name());
        item_->setNote(tool_.note());
        tool_.setAutoName(ui->chbxAutoName->isChecked());
        item_->tool() = tool_;
        emit itemChanged(item_);
        setChanged(false);
        return;
    }
    switch (tool_.type()) {
    case Tool::Drill:
        ui->dsbxDiameter->flicker();
        ui->dsbxOneTurnCut->flicker();
        ui->dsbxPassDepth->flicker();
        ui->dsbxPlungeRate->flicker();
        ui->dsbxSpindleSpeed->flicker();
        break;
    case Tool::EndMill:
    case Tool::Engraver:
        ui->dsbxDiameter->flicker();
        ui->dsbxFeedRate->flicker();
        ui->dsbxOneTurnCut->flicker();
        ui->dsbxPassDepth->flicker();
        ui->dsbxPlungeRate->flicker();
        ui->dsbxSpindleSpeed->flicker();
        ui->dsbxStepover->flicker();
        break;
    case Tool::Laser:
        ui->dsbxDiameter->flicker();
        ui->dsbxFeedRate->flicker();
        ui->dsbxSpindleSpeed->flicker();
        ui->dsbxStepover->flicker();
        break;
    case Tool::Group:
        break;
    }
}

void ToolEditForm::setDialog() {
    dialog_ = false;
    ui->pbApply->setVisible(false);
    ui->cbxToolType->setEnabled(dialog_);
    setVisibleToolWidgets(true);
}

void ToolEditForm::updateName() {
    if (!ui->chbxAutoName->isChecked())
        return;
    switch (ui->cbxToolType->currentIndex()) {
    case Tool::EndMill:
        ui->leName->setText(QString(tr("End Mill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        return;
    case Tool::Engraver:
        ui->leName->setText(QString(tr("Engrave (%2\302\260 %1 mm tip)")).arg(ui->dsbxDiameter->value()).arg(ui->dsbxAngle->value()));
        return;
    case Tool::Drill:
        ui->leName->setText(QString(tr("Drill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        return;
    case Tool::Laser:
        ui->leName->setText(QString(tr("Laser (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        return;
    }
}

#include "moc_tool_editform.cpp"
