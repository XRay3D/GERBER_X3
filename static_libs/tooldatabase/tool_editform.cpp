// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author : Damir Bakiev *
 * Version : na *
 * Date : March 25, 2023 *
 * Website : na *
 * Copyright : Damir Bakiev 2016-2023 *
 * License : *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt *
 ********************************************************************************/
#include "tool_editform.h"
#include "ui_tooleditform.h"
#include "utils.h"

#include "tool_item.h"
#include <QDebug>
#include <QSettings>

#define TR QCoreApplication::translate

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget{parent}
    , ui(new Ui::ToolEditForm) {
    ui->setupUi(this);
    // clang-format AlignArrayOfStructures: Left
    update = {
        {            ui->dsbxAngle,             &ToolEditForm::updateDsbxAngle},
        {         ui->dsbxDiameter,          &ToolEditForm::updateDsbxDiameter},
        {         ui->dsbxFeedRate,          &ToolEditForm::updateDsbxFeedRate},
        {           ui->dsbxLenght,            &ToolEditForm::updateDsbxLenght},
        {       ui->dsbxOneTurnCut,        &ToolEditForm::updateDsbxOneTurnCut},
        {ui->dsbxOneTurnCutPercent, &ToolEditForm::updateDsbxOneTurnCutPercent},
        {        ui->dsbxPassDepth,         &ToolEditForm::updateDsbxPassDepth},
        {       ui->dsbxPlungeRate,        &ToolEditForm::updateDsbxPlungeRate},
        {     ui->dsbxSpindleSpeed,      &ToolEditForm::updateDsbxSpindleSpeed},
        {         ui->dsbxStepover,          &ToolEditForm::updateDsbxStepover},
        {  ui->dsbxStepoverPercent,   &ToolEditForm::updateDsbxStepoverPercent},
    };

    get = {
        std::pair{       ui->dsbxAngle,        &Tool::angle},
        std::pair{    ui->dsbxDiameter,     &Tool::diameter},
        std::pair{    ui->dsbxFeedRate,     &Tool::feedRate},
        std::pair{      ui->dsbxLenght,       &Tool::lenght},
        std::pair{  ui->dsbxOneTurnCut,   &Tool::oneTurnCut},
        std::pair{   ui->dsbxPassDepth,    &Tool::passDepth},
        std::pair{  ui->dsbxPlungeRate,   &Tool::plungeRate},
        std::pair{ui->dsbxSpindleSpeed, &Tool::spindleSpeed},
        std::pair{    ui->dsbxStepover,     &Tool::stepover},
    };

    set = {
        std::pair{       ui->dsbxAngle,        &Tool::setAngle},
        std::pair{    ui->dsbxDiameter,     &Tool::setDiameter},
        std::pair{    ui->dsbxFeedRate,     &Tool::setFeedRate},
        std::pair{      ui->dsbxLenght,       &Tool::setLenght},
        std::pair{  ui->dsbxOneTurnCut,   &Tool::setOneTurnCut},
        std::pair{   ui->dsbxPassDepth,    &Tool::setPassDepth},
        std::pair{  ui->dsbxPlungeRate,   &Tool::setPlungeRate},
        std::pair{ui->dsbxSpindleSpeed, &Tool::setSpindleSpeed},
        std::pair{    ui->dsbxStepover,     &Tool::setStepover},
    };

    dsbxMapdsbxMap = {
        // clang-format off
        Data {{ui->dsbxAngle},                                 {Tool::Drill, Tool::Engraver},                                  180.0, 120.0    },
        Data {{ui->dsbxFeedRate},                              {Tool::Laser, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, 100000.         },
        Data {{ui->dsbxLenght},                                {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, 100000.         },
        Data {{ui->dsbxOneTurnCut, ui->dsbxOneTurnCutPercent}, {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, ui->dsbxDiameter},
        Data {{ui->dsbxPassDepth},                             {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, 100.0           },
        Data {{ui->dsbxPlungeRate},                            {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, 100000.         }, // ThreadMill?
        Data {{ui->dsbxStepover, ui->dsbxStepoverPercent},     {Tool::Laser, Tool::EndMill, Tool::Engraver, Tool::ThreadMill}, ui->dsbxDiameter},
    }; // clang-format on

    for(auto [dsbx, _]: update)
        connect(dsbx, &QDoubleSpinBox::valueChanged, this, &ToolEditForm::valueChanged);

    connect(ui->cbxFeedSpeeds, &QComboBox::currentIndexChanged, this, [this](int index) {
        double lastFeed = feed;
        switch(index) {
        case mmPerSec: // mm/sec
            feed = 1.0 / 60.0;
            break;
        case mmPerMin: // mm/min!!!
            feed = 1.0;
            break;
        case cmPerMin: // cm/min
            feed = 1.0 / 10.0;
            break;
        case mPerMin: // m/min
            feed = 1.0 / 1000.0;
            break;
        default:
            break;
        }

        bool fl1 = parentWidget()->isWindowModified();
        bool fl2 = ui->pbApply->isEnabled();

        ui->dsbxFeedRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / lastFeed) * feed);

        ui->dsbxPlungeRate->setSuffix(" " + ui->cbxFeedSpeeds->currentText());
        ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / lastFeed) * feed);

        parentWidget()->setWindowModified(fl1);
        ui->pbApply->setEnabled(fl2);
    });

    connect(ui->leName, &QLineEdit::textChanged, this, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, this, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, this, [this] { tool_.setNote(ui->teNote->toPlainText()); setChanged(); });

    ui->cbxUnits->setVisible(false);
    ui->cbxToolType->addItem(QIcon::fromTheme("endmill"), TR("ToolEditForm", "End Mill", nullptr), Tool::EndMill);
    ui->cbxToolType->addItem(QIcon::fromTheme("drill"), TR("ToolEditForm", "Drill", nullptr), Tool::Drill);
    ui->cbxToolType->addItem(QIcon::fromTheme("engraving"), TR("ToolEditForm", "Engraver", nullptr), Tool::Engraver);
    ui->cbxToolType->addItem(QIcon::fromTheme("thread_mill"), TR("ToolEditForm", "Thread Mill", nullptr), Tool::ThreadMill);
    ui->cbxToolType->addItem(QIcon::fromTheme("laser"), TR("ToolEditForm", "Laser", nullptr), Tool::Laser);

    connect(ui->cbxToolType, &QComboBox::currentIndexChanged, this, &ToolEditForm::setupToolWidgets);

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
    if(item == nullptr)
        return;
    item_ = item;
    if(item_->isTool()) {
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
    // qDebug(__FUNCTION__);
    tool_ = tool;

    for(auto& data: dsbxMapdsbxMap) {
        data.dsbx[0]->setEnabled(true);
        data.dsbx[0]->setMaximum(std::numeric_limits<double>::max());
    }

    switch(tool_.type()) {
    case Tool::Drill:
        dsbxMapdsbxMap[0].defVal = 120.;
        break;
    case Tool::Engraver:
        dsbxMapdsbxMap[0].defVal = 90.;
        break;
    default:
        break;
    }

    for(auto [dsbx, get]: get)
        dsbx->setValue((tool.*get)());

    for(auto& data: dsbxMapdsbxMap)
        if(qFuzzyIsNull(data.dsbx[0]->value()))
            data.lastVal.reset();
        else
            data.lastVal = data.dsbx[0]->value();

    for(int i{}; i < ui->cbxToolType->count(); ++i) {
        if(ui->cbxToolType->itemData(i).value<Tool::Type>() == tool.type()) {
            if(ui->cbxToolType->currentIndex() == i)
                setupToolWidgets(tool_.type());
            ui->cbxToolType->setCurrentIndex(i);
            break;
        }
    }

    ui->chbxAutoName->setChecked(tool.autoName());
    ui->leName->setText(tool.name());
    ui->teNote->setText(tool.note());
}

void ToolEditForm::setChanged(bool fl) {
    ui->pbApply->setEnabled(fl);
    parentWidget()->setWindowModified(fl);
}

void ToolEditForm::setVisibleToolWidgets(bool visible) {
    for(auto* w: std::initializer_list<QWidget*>{
            ui->cbxFeedSpeeds,
            ui->cbxToolType,
            ui->cbxUnits,
            ui->chbxAutoName,
            ui->grbxCuttingParameters,
            ui->grbxFeedSpeeds,
            ui->grbxGeometry,
            ui->lblToolType,
            ui->lblUnits,
        })
        w->setVisible(visible);
    setMinimumWidth(width());
}

// bool operator<(std::set<Tool::Type> l, std::set<Tool::Type> r) {
//     return l < r;
// }

void ToolEditForm::setupToolWidgets(int) {
    const auto lastType = tool_.type();

    auto currType = ui->cbxToolType->currentData().value<Tool::Type>();
    tool_.setType(currType);

    // qDebug() << "\n\n";

    static Overload value{
        [](auto* val) { return val->value(); },
        [](auto val) { return val; },
    };

    for(auto& data: dsbxMapdsbxMap) {
        if(data.set.contains(currType)) {
            data.dsbx[0]->setMaximum(std::visit(value, data.max));

            if(data.lastVal) // restore last val
                data.dsbx[0]->setValue(data.lastVal.value());
            else if(data.defVal) // set default val
                data.dsbx[0]->setValue(data.defVal.value());

            data.dsbx[0]->setEnabled(true);
        } else {
            if(data.dsbx[0]->isEnabled()) // save val
                data.lastVal = data.dsbx[0]->value();

            data.dsbx[0]->setRange(.0, .0);
            data.dsbx[0]->setEnabled(false);
        }
        if(data.dsbx[1])
            data.dsbx[1]->setEnabled(data.dsbx[0]->isEnabled());
    }

    // //qDebug() << lastVal;

    static const std::unordered_map<Tool::Type, QString> lblText{
        {Tool::Drill, tr("Pass")},
        {Tool::EndMill, tr("Depth")},
        {Tool::Engraver, tr("Depth")},
        {Tool::Laser, ""},
        {Tool::ThreadMill, tr("Thread Pitch")},
    };

    ui->lblPassDepth->setText(lblText.at(currType));

    setChanged();
    updateName();
}

void ToolEditForm::valueChanged(double val) {
    if(auto dsbx = qobject_cast<QDoubleSpinBox*>(sender()); dsbx)
        (this->*update[dsbx])(val);
    ui->lblWarn->setVisible(ui->dsbxStepover->value() > (ui->dsbxDiameter->value() * 0.5)); // WARNING возможны 'непрорезы'
    updateName();
    setChanged();
}

void ToolEditForm::on_pbApply_clicked() {
    bool fl{};

    for(auto [dsbx, set]: set)
        if(dsbx->isEnabled() && qFuzzyIsNull(dsbx->value()))
            dsbx->flicker(), fl = true;

    if(fl)
        return;

    for(auto [dsbx, set]: set)
        (tool_.*set)(dsbx->value());

    if(item_ && tool_.isValid()) {
        item_->setName(tool_.name());
        item_->setNote(tool_.note());
        item_->tool() = tool_;
        tool_.setAutoName(ui->chbxAutoName->isChecked());
        emit itemChanged(item_);
        setChanged(false);
    }
}

void ToolEditForm::setDialog() {
    dialog_ = false;
    ui->pbApply->setVisible(false);
    ui->cbxToolType->setEnabled(dialog_);
    setVisibleToolWidgets(true);
}

void ToolEditForm::updateName() {
    if(!ui->chbxAutoName->isChecked())
        return;
    ui->leName->setText([this](auto type) {
        switch(type) {
        case Tool::EndMill:
            return tr("End Mill (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Engraver:
            return tr("Engrave (%2\302\260 %1 mm tip)").arg(ui->dsbxDiameter->value()).arg(ui->dsbxAngle->value());
        case Tool::Drill:
            return tr("Drill (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Laser:
            return tr("Laser (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::ThreadMill:
            return tr("Thread Mill (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Group:
        default: return QString{};
        }
    }(ui->cbxToolType->currentData().value<Tool::Type>()));
}

void ToolEditForm::updateDsbxAngle(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setAngle(val);
}

void ToolEditForm::updateDsbxDiameter(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setDiameter(val);
    ui->dsbxOneTurnCut->setMaximum(val);
    ui->dsbxStepover->setMaximum(val);
    if(ui->dsbxStepover->value() == 0.0)
        ui->dsbxStepover->setValue(val * 0.5);
    if(ui->dsbxOneTurnCut->value() == 0.0)
        ui->dsbxOneTurnCut->setValue(val * 0.1);
    emit ui->dsbxOneTurnCutPercent->valueChanged(ui->dsbxOneTurnCutPercent->value());
    emit ui->dsbxStepoverPercent->valueChanged(ui->dsbxStepoverPercent->value());
}

void ToolEditForm::updateDsbxFeedRate(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setFeedRate(val / feed);
}

void ToolEditForm::updateDsbxOneTurnCut(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setOneTurnCut(val);
    ui->dsbxOneTurnCutPercent->setValue(tool_.diameter() > 0.0 ? val / (tool_.diameter() * 0.01) : 0.0);
    if(ui->chbxFeedRate->isChecked())
        ui->dsbxFeedRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed);
    if(ui->chbxPlungeRate->isChecked())
        ui->dsbxPlungeRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed);
}

void ToolEditForm::updateDsbxPassDepth(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setPassDepth(val);
}

void ToolEditForm::updateDsbxPlungeRate(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setPlungeRate(val / feed);
}

void ToolEditForm::updateDsbxSpindleSpeed(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setSpindleSpeed(val); /*rpm*/
    if(ui->chbxFeedRate->isChecked())
        ui->dsbxFeedRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed);
    if(ui->chbxPlungeRate->isChecked())
        ui->dsbxPlungeRate->setValue(tool_.oneTurnCut() * tool_.spindleSpeed() * feed);
}

void ToolEditForm::updateDsbxStepover(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setStepover(val);
    ui->dsbxStepoverPercent->setValue(tool_.diameter() > 0.0 ? val / (tool_.diameter() * 0.01) : 0.0);
}

void ToolEditForm::updateDsbxLenght(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setLenght(val);
}

void ToolEditForm::updateDsbxOneTurnCutPercent(double val) {
    // qDebug() << __FUNCTION__ << val;
    ui->dsbxOneTurnCut->setValue(val * (tool_.diameter() * 0.01));
}

void ToolEditForm::updateDsbxStepoverPercent(double val) {
    // qDebug() << __FUNCTION__ << val;
    ui->dsbxStepover->setValue(val * (tool_.diameter() * 0.01));
}

#include "moc_tool_editform.cpp"
