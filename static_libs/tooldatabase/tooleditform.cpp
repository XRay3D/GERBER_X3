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
#include "tooleditform.h"
#include "ui_tooleditform.h"

#include "toolitem.h"
#include <QSettings>

#include "leakdetector.h"

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ToolEditForm)
{
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
        connect(pPsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxFeedSpeeds, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
#else
    connect(ui->cbxFeedSpeeds, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), [this](int index) {
#endif
        double tmpFeed = m_feed;
        switch (index) {
        case mm_sec: // mm/sec
            m_feed = 1.0 / 60.0;
            break;
        case mm_min: // mm/min!!!
            m_feed = 1.0;
            break;
        case cm_min: // cm/min
            m_feed = 1.0 / 10.0;
            break;
        case m_min: //  m/min
            m_feed = 1.0 / 1000.0;
            break;
        default:
            break;
        }
        bool fl1 = parentWidget()->isWindowModified();
        bool fl2 = ui->pbApply->isEnabled();
        ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / tmpFeed) * m_feed);
        ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / tmpFeed) * m_feed);
        ui->dsbxFeedRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        ui->dsbxPlungeRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        parentWidget()->setWindowModified(fl1);
        ui->pbApply->setEnabled(fl2);
    });
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxToolType, qOverload<int>(&QComboBox::currentIndexChanged), this, &ToolEditForm::setupToolWidgets);
#else
    connect(ui->cbxToolType, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &ToolEditForm::setupToolWidgets);
#endif

    connect(ui->leName, &QLineEdit::textChanged, [this](const QString& arg1) { m_tool.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, [this](const QString& arg1) { m_tool.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, [this] { m_tool.setNote(ui->teNote->toPlainText()); setChanged(); });

    ui->cbxUnits->setVisible(false);

    ui->cbxToolType->setItemIcon(Tool::Drill, QIcon::fromTheme("drill"));
    ui->cbxToolType->setItemIcon(Tool::EndMill, QIcon::fromTheme("endmill"));
    ui->cbxToolType->setItemIcon(Tool::Engraver, QIcon::fromTheme("engraving"));
    ui->cbxToolType->setItemIcon(Tool::Laser, QIcon::fromTheme("laser"));

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value("cbxFeedSpeeds").toInt());
    //ui->cbxUnits->setCurrentIndex(settings.value("cbxUnits").toInt());
    setVisibleToolWidgets(false);
    setChanged(false);
}

ToolEditForm::~ToolEditForm()
{
    QSettings settings;
    settings.setValue("cbxFeedSpeeds", ui->cbxFeedSpeeds->currentIndex());
    //settings.setValue("cbxUnits", ui->cbxUnits->currentIndex());
    delete ui;
    ui = nullptr;
}

void ToolEditForm::setItem(ToolItem* item)
{
    if (item == nullptr)
        return;
    m_item = item;
    if (m_item->isTool()) {
        setTool(m_item->tool());
        setVisibleToolWidgets(true);
    } else {
        ui->leName->setText(m_item->name());
        ui->teNote->setText(m_item->note());
        setVisibleToolWidgets(false);
    }
    setChanged(false);
}

void ToolEditForm::setTool(const Tool& tool)
{
    m_tool = tool;
    setupToolWidgets(m_tool.type());
    dsbx[Tool::Angle]->setValue(tool.angle());
    dsbx[Tool::Diameter]->setValue(tool.diameter());
    dsbx[Tool::FeedRate]->setValue(tool.feedRate() * m_feed);
    dsbx[Tool::OneTurnCut]->setValue(tool.oneTurnCut());
    dsbx[Tool::PassDepth]->setValue(tool.passDepth());
    dsbx[Tool::PlungeRate]->setValue(tool.plungeRate() * m_feed);
    dsbx[Tool::SpindleSpeed]->setValue(tool.spindleSpeed());
    dsbx[Tool::Stepover]->setValue(tool.stepover());

    ui->cbxToolType->setCurrentIndex(tool.type());
    ui->chbxAutoName->setChecked(tool.autoName());
    ui->leName->setText(tool.name());
    ui->teNote->setText(tool.note());
}

void ToolEditForm::setChanged(bool fl)
{
    ui->pbApply->setEnabled(fl);
    parentWidget()->setWindowModified(fl);
}

void ToolEditForm::setVisibleToolWidgets(bool visible)
{
    ui->cbxFeedSpeeds->setVisible(visible);
    ui->cbxToolType->setVisible(visible);
    ui->cbxUnits->setVisible(visible);
    ui->chbxAutoName->setVisible(visible);
    ui->groupBox_2->setVisible(visible);
    ui->groupBox_3->setVisible(visible);
    ui->groupBox_4->setVisible(visible);
    ui->lblToolType->setVisible(visible);
    ui->lblUnits->setVisible(visible);
    setMinimumWidth(width());
}

void ToolEditForm::setupToolWidgets(int type)
{
    const int lastType = m_tool.type();
    m_tool.setType(type);

    auto setEnabled = [&, type](DoubleSpinBox* dsbx_, double min, double max) {
        if (!dsbx_->isEnabled()) {
            saveRestoreMap[type].restore(dsbx_);
            dsbx_->setEnabled(true);
        }
        if (min != 0.0 || max != 0.0)
            dsbx_->setRange(min, max);
    };
    auto setDisabled = [&, lastType](DoubleSpinBox* dsbx_) {
        if (dsbx_->isEnabled()) {
            saveRestoreMap[lastType].save(dsbx_);
            dsbx_->setRange(0.0, 0.0);
            dsbx_->setEnabled(false);
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
        //        if (ui->dsbxOneTurnCut->value() == 0.0) {
        //            ui->dsbxOneTurnCut->setValue(ui->dsbxDiameter->value() * 0.5);
        //            ui->dsbxPlungeRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        //        }
        ui->label_3->setText(tr("Pass"));
        break;
    case Tool::EndMill:
        setDisabled(ui->dsbxAngle);
        setEnabled(ui->dsbxFeedRate, 0.0, 100000.0);
        setEnabled(ui->dsbxOneTurnCut, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxOneTurnCutPercent, 0.0, 100.0);
        setEnabled(ui->dsbxPassDepth, 0.0, 10.0);
        setEnabled(ui->dsbxPlungeRate, 0.0, 100000.0);
        setEnabled(ui->dsbxStepover, 0.0, ui->dsbxDiameter->value() * 0.5);
        setEnabled(ui->dsbxStepoverPercent, 0.0, 50.0);
        //        if (ui->dsbxStepover->value() == 0.0) {
        //            ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
        //            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        //        }
        ui->label_3->setText(tr("Depth"));
        break;
    case Tool::Engraver:
        setEnabled(ui->dsbxAngle, 0.0, 180.0);
        setEnabled(ui->dsbxFeedRate, 0.0, 100000.0);
        setEnabled(ui->dsbxOneTurnCut, 0.0, ui->dsbxDiameter->value());
        setEnabled(ui->dsbxOneTurnCutPercent, 0.0, 100.0);
        setEnabled(ui->dsbxPassDepth, 0.0, 10.0);
        setEnabled(ui->dsbxPlungeRate, 0.0, 100000.0);
        setEnabled(ui->dsbxStepover, 0.0, ui->dsbxDiameter->value() * 0.5);
        setEnabled(ui->dsbxStepoverPercent, 0.0, 50.0);
        //        if (ui->dsbxStepover->value() == 0.0) {
        //            ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
        //            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        //        }
        ui->label_3->setText(tr("Depth"));
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
        //        if (ui->dsbxStepover->value() == 0.0) {
        //            ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
        //            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        //        }
        break;
    }
    setChanged();
    updateName();
}

void ToolEditForm::valueChangedSlot(double value)
{
    switch (dsbx.indexOf(dynamic_cast<DoubleSpinBox*>(sender()))) {
    case Tool::Angle:
        m_tool.setAngle(value);
        break;
    case Tool::Diameter:
        m_tool.setDiameter(value);
        ui->dsbxOneTurnCut->setMaximum(value);
        ui->dsbxStepover->setMaximum(value);
        if (ui->dsbxStepover->value() == 0.0)
            ui->dsbxStepover->setValue(value * 0.5);
        if (ui->dsbxOneTurnCut->value() == 0.0)
            ui->dsbxOneTurnCut->setValue(value * 0.1);
        ui->dsbxOneTurnCutPercent->valueChanged(ui->dsbxOneTurnCutPercent->value());
        ui->dsbxStepoverPercent->valueChanged(ui->dsbxStepoverPercent->value());
        break;
    case Tool::FeedRate:
        m_tool.setFeedRate(value / m_feed);
        break;
    case Tool::OneTurnCut:
        m_tool.setOneTurnCut(value);
        ui->dsbxOneTurnCutPercent->setValue(m_tool.diameter() > 0.0
                ? value / (m_tool.diameter() * 0.01)
                : 0);
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        break;
    case Tool::PassDepth:
        m_tool.setPassDepth(value);
        break;
    case Tool::PlungeRate:
        m_tool.setPlungeRate(value / m_feed);
        break;
    case Tool::SpindleSpeed:
        m_tool.setSpindleSpeed(value); //rpm
        if (ui->chbxFeedRate->isChecked())
            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        if (ui->chbxPlungeRate->isChecked())
            ui->dsbxPlungeRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        break;
    case Tool::Stepover:
        m_tool.setStepover(value);
        ui->dsbxStepoverPercent->setValue(m_tool.diameter() > 0.0
                ? value / (m_tool.diameter() * 0.01)
                : 0);
        break;
    case Tool::OneTurnCutPercent:
        ui->dsbxOneTurnCut->setValue(value * (m_tool.diameter() * 0.01));
        break;
    case Tool::StepoverPercent:
        ui->dsbxStepover->setValue(value * (m_tool.diameter() * 0.01));
        break;
    default:
        break;
    }

    updateName();
    setChanged();
}

void ToolEditForm::on_pbApply_clicked()
{
    if (m_item && m_tool.isValid()) {
        m_item->setName(m_tool.name());
        m_item->setNote(m_tool.note());
        m_tool.setAutoName(ui->chbxAutoName->isChecked());
        m_item->tool() = m_tool;
        emit itemChanged(m_item);
        setChanged(false);
        return;
    }
    switch (m_tool.type()) {
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

void ToolEditForm::setDialog()
{
    m_dialog = false;
    ui->pbApply->setVisible(false);
    ui->cbxToolType->setEnabled(m_dialog);
    setVisibleToolWidgets(true);
}

void ToolEditForm::updateName()
{
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
