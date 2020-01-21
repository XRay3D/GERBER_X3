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

    connect(ui->cbxFeedSpeeds, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
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

    connect(ui->cbxToolType, qOverload<int>(&QComboBox::currentIndexChanged), this, &ToolEditForm::setupToolWidgets);

    connect(ui->leName, &QLineEdit::textChanged, [this](const QString& arg1) { m_tool.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, [this](const QString& arg1) { m_tool.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, [this] { m_tool.setNote(ui->teNote->toPlainText()); setChanged(); });

    ui->cbxUnits->setVisible(false);

    ui->cbxToolType->setItemIcon(Tool::Drill, QIcon::fromTheme("drill"));
    ui->cbxToolType->setItemIcon(Tool::EndMill, QIcon::fromTheme("endmill"));
    ui->cbxToolType->setItemIcon(Tool::Engraving, QIcon::fromTheme("engraving"));
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
    m_tool.setType(type);
    ui->dsbxAngle->setEnabled(true);
    ui->dsbxFeedRate->setEnabled(true);
    ui->dsbxStepover->setEnabled(true);
    ui->dsbxStepoverPercent->setEnabled(true);
    switch (type) {
    case Tool::Drill:
        ui->label_3->setText(tr("Pass"));
        ui->dsbxAngle->setRange(90.0, 180.0);
        ui->dsbxAngle->setValue(qFuzzyIsNull(m_angle) ? 120.0 : m_angle);
        ui->dsbxFeedRate->setEnabled(false);
        ui->dsbxFeedRate->setMaximum(0.0);
        ui->dsbxStepover->setEnabled(false);
        ui->dsbxStepover->setMaximum(0.0);
        ui->dsbxStepoverPercent->setEnabled(false);
        ui->dsbxStepoverPercent->setMaximum(0.0);
        break;
    case Tool::EndMill:
        ui->label_3->setText(tr("Depth"));
        m_angle = ui->dsbxAngle->value();
        ui->dsbxAngle->setEnabled(false);
        ui->dsbxAngle->setRange(0.0, 0.0);
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
        ui->dsbxAngle->setRange(0.0, 180.0);
        ui->dsbxAngle->setValue(m_angle);
        ui->dsbxFeedRate->setMaximum(100000.0);
        ui->dsbxStepover->setMaximum(ui->dsbxDiameter->value());
        ui->dsbxStepoverPercent->setMaximum(100.0);
        if (ui->dsbxStepover->value() == 0.0) {
            ui->dsbxStepover->setValue(ui->dsbxDiameter->value() * 0.5);
            ui->dsbxFeedRate->setValue(m_tool.oneTurnCut() * m_tool.spindleSpeed() * m_feed);
        }
        break;
    case Tool::Laser:
        ui->label_3->setText(tr("Depth"));
        m_angle = ui->dsbxAngle->value();

        ui->dsbxAngle->setEnabled(false);
        ui->dsbxAngle->setRange(0.0, 0.0);

        ui->dsbxPassDepth->setEnabled(false);
        ui->dsbxPassDepth->setRange(0.0, 0.0);

        ui->dsbxPlungeRate->setEnabled(false);
        ui->dsbxPlungeRate->setRange(0.0, 0.0);

        ui->dsbxOneTurnCut->setEnabled(false);
        ui->dsbxOneTurnCut->setRange(0.0, 0.0);
        ui->dsbxOneTurnCutPercent->setEnabled(false);

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
}

void ToolEditForm::valueChangedSlot(double value)
{
    switch (dsbx.indexOf(dynamic_cast<DoubleSpinBox*>(sender()))) {
    case Tool::Angle:
        m_tool.setAngle(value);
        m_angle = value;
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
    case Tool::Laser:
        ui->dsbxDiameter->flicker();
        ui->dsbxFeedRate->flicker();
        ui->dsbxOneTurnCut->flicker();
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
    case Tool::Engraving:
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
