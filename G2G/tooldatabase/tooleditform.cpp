#include "tooleditform.h"
#include "toolitem.h"
#include "toolmodel.h"
#include "ui_tooleditform.h"

#include <QDebug>
#include <QPicture>
#include <QSettings>
#include <QTimer>

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ToolEditForm)
{
    ui->setupUi(this);

    connect(ui->dsbxAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxDiameter, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxFeedRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxOneTurnCut, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxOneTurnCutPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxPassDepth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxPlungeRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxSpindleSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxStepover, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);
    connect(ui->dsbxStepoverPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolEditForm::valueChangedSlot);

    connect(ui->cbxFeedSpeeds, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        double tmpFeed = m_feed;
        switch (index) {
        case 0: //mm/sec
            m_feed = 1.0 / 60.0;
            break;
        case 1: //mm/min!!!
            m_feed = 1.0;
            break;
        case 2: //cm/min
            m_feed = 1.0 / 10.0;
            break;
        case 3: //m/min
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

    connect(ui->cbxToolType, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        m_tool.setType(index);
        ui->dsbxAngle->setEnabled(true);
        ui->dsbxFeedRate->setEnabled(true);
        ui->dsbxStepover->setEnabled(true);
        ui->dsbxStepoverPercent->setEnabled(true);
        switch (index) {
        case Tool::Drill:
            ui->label_3->setText(tr("Pass"));
            ui->dsbxAngle->setMaximum(180.0);
            if (!ui->dsbxAngle->value())
                ui->dsbxAngle->setValue(120.0);
            ui->dsbxFeedRate->setEnabled(false);
            ui->dsbxFeedRate->setMaximum(0.0);
            ui->dsbxStepover->setEnabled(false);
            ui->dsbxStepover->setMaximum(0.0);
            ui->dsbxStepoverPercent->setEnabled(false);
            ui->dsbxStepoverPercent->setMaximum(0.0);
            break;
        case Tool::EndMill:
            ui->label_3->setText(tr("Depth"));
            ui->dsbxAngle->setEnabled(false);
            ui->dsbxAngle->setMaximum(0.0);
            ui->dsbxFeedRate->setMaximum(100000.0);
            ui->dsbxStepover->setMaximum(ui->dsbxDiameter->value());
            ui->dsbxStepoverPercent->setMaximum(100.0);
            if (ui->dsbxStepover->value() == 0.0) {
                ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
                ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
            }
            break;
        case Tool::Engraving:
            ui->label_3->setText(tr("Depth"));
            ui->dsbxAngle->setValue(0.0);
            ui->dsbxAngle->setMaximum(180.0);
            ui->dsbxFeedRate->setMaximum(100000.0);
            ui->dsbxStepover->setMaximum(ui->dsbxDiameter->value());
            ui->dsbxStepoverPercent->setMaximum(100.0);
            if (ui->dsbxStepover->value() == 0.0) {
                ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
                ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
            }
            break;
        }
        setChanged();
        updateName();
    });

    connect(ui->leName, &QLineEdit::textChanged, [=](const QString& arg1) { m_tool.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, [=](const QString& arg1) { m_tool.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, [=] { m_tool.setNote(ui->teNote->toPlainText()); setChanged(); });

    set = {
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

    ui->cbxUnits->setVisible(false);

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value("cbxFeedSpeeds").toInt());
    //ui->cbxUnits->setCurrentIndex(settings.value("cbxUnits").toInt());
    setVisibleUnusedWidgets(false);
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
        setTool(item->tool());
    } else {
        ui->leName->setText(m_item->name());
        ui->teNote->setText(m_item->note());
        setVisibleUnusedWidgets(false);
    }
    setChanged(false);
}

void ToolEditForm::setChanged(bool fl)
{
    ui->pbApply->setEnabled(fl);
    parentWidget()->setWindowModified(fl);
}

void ToolEditForm::setVisibleUnusedWidgets(bool visible)
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

void ToolEditForm::valueChangedSlot(double value)
{
    switch (set.indexOf(static_cast<QDoubleSpinBox*>(sender()))) {
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
        ui->dsbxOneTurnCutPercent->setValue(value / (m_tool.diameter() * 0.01));
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
        ui->dsbxStepoverPercent->setValue(value / (m_tool.diameter() * 0.01));
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
    case Tool::Engraving:
        ui->dsbxDiameter->flicker();
        ui->dsbxFeedRate->flicker();
        ui->dsbxOneTurnCut->flicker();
        ui->dsbxPassDepth->flicker();
        ui->dsbxPlungeRate->flicker();
        ui->dsbxSpindleSpeed->flicker();
        ui->dsbxStepover->flicker();
        break;
    case Tool::Group:
    default:
        break;
    }
}

void ToolEditForm::setDialog()
{
    m_dialog = false;
    ui->pbApply->setVisible(false);
    ui->cbxToolType->setEnabled(m_dialog);
}

void ToolEditForm::setTool(const Tool& tool)
{
    m_tool = tool;

    set[Tool::Angle]->setValue(tool.angle());
    set[Tool::Diameter]->setValue(tool.diameter());
    set[Tool::FeedRate]->setValue(tool.feedRate() * m_feed);
    set[Tool::OneTurnCut]->setValue(tool.oneTurnCut());
    set[Tool::PassDepth]->setValue(tool.passDepth());
    set[Tool::PlungeRate]->setValue(tool.plungeRate() * m_feed);
    set[Tool::SpindleSpeed]->setValue(tool.spindleSpeed());
    set[Tool::Stepover]->setValue(tool.stepover());

    ui->cbxToolType->setCurrentIndex(tool.type());
    ui->chbxAutoName->setChecked(tool.autoName());
    ui->leName->setText(tool.name());
    ui->teNote->setText(tool.note());
    setVisibleUnusedWidgets(true);
}

void ToolEditForm::updateName()
{
    if (!ui->chbxAutoName->isChecked())
        return;
    switch (ui->cbxToolType->currentIndex()) {
    case Tool::EndMill:
        ui->leName->setText(QString(tr("End Mill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        return;
    case Tool::Engraving:
        ui->leName->setText(QString(tr("Engrave (%2\302\260 %1 mm tip)")).arg(ui->dsbxDiameter->value()).arg(ui->dsbxAngle->value()));
        return;
    case Tool::Drill:
        ui->leName->setText(QString(tr("Drill (Ø%1 mm)")).arg(ui->dsbxDiameter->value()));
        return;
    }
}
