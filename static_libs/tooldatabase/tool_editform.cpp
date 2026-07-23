/********************************************************************************
 * Author : Damir Bakiev *
 * Version : na *
 * Date : XXXXX XX, 2025 *
 * Website : na *
 * Copyright : Damir Bakiev 2016-2026 *
 * License : *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt *
 ********************************************************************************/
#include "tool_editform.h"
#include "ui_tooleditform.h"
#include "utils.h"

#include "tool_item.h"
#include <QDebug>
#include <QFormLayout>
#include <QSettings>

#define TR QCoreApplication::translate

ToolEditForm::ToolEditForm(QWidget* parent)
    : QWidget{parent}
    , ui{new Ui::ToolEditForm{}} {
    ui->setupUi(this);
    // clang-format AlignArrayOfStructures: Left
    update = {
        {ui->dsbxAngle,             &ToolEditForm::updateDsbxAngle            },
        {ui->dsbxDiameter,          &ToolEditForm::updateDsbxDiameter         },
        {ui->dsbxHoleDiam,          &ToolEditForm::updateDsbxHoleDiam         },
        {ui->dsbxFeedRate,          &ToolEditForm::updateDsbxFeedRate         },
        {ui->dsbxLenght,            &ToolEditForm::updateDsbxLenght           },
        {ui->dsbxOneTurnCut,        &ToolEditForm::updateDsbxOneTurnCut       },
        {ui->dsbxOneTurnCutPercent, &ToolEditForm::updateDsbxOneTurnCutPercent},
        {ui->dsbxPlungeRate,        &ToolEditForm::updateDsbxPlungeRate       },
        {ui->dsbxSpindleSpeed,      &ToolEditForm::updateDsbxSpindleSpeed     },
        {ui->dsbxStepover,          &ToolEditForm::updateDsbxStepover         },
        {ui->dsbxStepoverPercent,   &ToolEditForm::updateDsbxStepoverPercent  },
        {ui->dsbxDepth,             &ToolEditForm::updateDsbxPassDepth        },
        {ui->dsbxPass,              &ToolEditForm::updateDsbxPassDepth        },
        {ui->dsbxThreadPitch,       &ToolEditForm::updateDsbxPassDepth        },
    };
    for(auto [dsbx, _]: update)
        connect(dsbx, &QDoubleSpinBox::valueChanged, this, &ToolEditForm::valueChanged);

    get = {
        std::pair{ui->dsbxAngle,        &Tool::angle       },
        std::pair{ui->dsbxHoleDiam,     &Tool::angle       },
        std::pair{ui->dsbxDepth,        &Tool::passDepth   },
        std::pair{ui->dsbxDiameter,     &Tool::diameter    },
        std::pair{ui->dsbxFeedRate,     &Tool::feedRate    },
        std::pair{ui->dsbxLenght,       &Tool::lenght      },
        std::pair{ui->dsbxOneTurnCut,   &Tool::oneTurnCut  },
        std::pair{ui->dsbxPass,         &Tool::passDepth   },
        std::pair{ui->dsbxPlungeRate,   &Tool::plungeRate  },
        std::pair{ui->dsbxSpindleSpeed, &Tool::spindleSpeed},
        std::pair{ui->dsbxStepover,     &Tool::stepover    },
        std::pair{ui->dsbxThreadPitch,  &Tool::passDepth   },
    };

    set = {
        std::pair{ui->dsbxAngle,        &Tool::setAngle       },
        std::pair{ui->dsbxHoleDiam,     &Tool::setAngle       },
        std::pair{ui->dsbxDepth,        &Tool::setPassDepth   },
        std::pair{ui->dsbxDiameter,     &Tool::setDiameter    },
        std::pair{ui->dsbxFeedRate,     &Tool::setFeedRate    },
        std::pair{ui->dsbxLenght,       &Tool::setLenght      },
        std::pair{ui->dsbxOneTurnCut,   &Tool::setOneTurnCut  },
        std::pair{ui->dsbxPass,         &Tool::setPassDepth   },
        std::pair{ui->dsbxPlungeRate,   &Tool::setPlungeRate  },
        std::pair{ui->dsbxSpindleSpeed, &Tool::setSpindleSpeed},
        std::pair{ui->dsbxStepover,     &Tool::setStepover    },
        std::pair{ui->dsbxThreadPitch,  &Tool::setPassDepth   },
    };

    connect(ui->cbxFeedSpeeds, &QComboBox::currentIndexChanged, this, [this](int index) {
        double lastFeed = feed;
        switch(index) {
        case mmPerSec: feed = 1. / 60.; break;   // mm/sec
        case mmPerMin: feed = 1.; break;         // mm/min!!!
        case cmPerMin: feed = 1. / 10.; break;   // cm/min
        case mPerMin : feed = 1. / 1000.; break; // m/min
        default      : break;
        }

        bool fl1 = parentWidget()->isWindowModified();
        bool fl2 = ui->pbApply->isEnabled();

        ui->dsbxFeedRate->setSuffix(u" "_s + ui->cbxFeedSpeeds->currentText());
        ui->dsbxFeedRate->setValue((ui->dsbxFeedRate->value() / lastFeed) * feed);

        ui->dsbxPlungeRate->setSuffix(u" "_s + ui->cbxFeedSpeeds->currentText());
        ui->dsbxPlungeRate->setValue((ui->dsbxPlungeRate->value() / lastFeed) * feed);

        parentWidget()->setWindowModified(fl1);
        ui->pbApply->setEnabled(fl2);
    });

    connect(ui->leName, &QLineEdit::textChanged, this, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); });
    connect(ui->leName, &QLineEdit::textEdited, this, [this](const QString& arg1) { tool_.setName(arg1); setChanged(); ui->chbxAutoName->setChecked(false); });
    connect(ui->teNote, &QTextEdit::textChanged, this, [this] { tool_.setNote(ui->teNote->toPlainText()); setChanged(); });

    ui->cbxUnits->setVisible(false);
    ui->cbxToolType->addItem(QIcon::fromTheme(u"endmill"_s), TR("ToolEditForm", "End Mill", nullptr), Tool::EndMill);
    ui->cbxToolType->addItem(QIcon::fromTheme(u"drill"_s), TR("ToolEditForm", "Drill", nullptr), Tool::Drill);
    ui->cbxToolType->addItem(QIcon::fromTheme(u"engraving"_s), TR("ToolEditForm", "Engraver", nullptr), Tool::Engraver);
    ui->cbxToolType->addItem(QIcon::fromTheme(u"thread_mill"_s), TR("ToolEditForm", "Thread Mill", nullptr), Tool::ThreadMill);
    ui->cbxToolType->addItem(QIcon::fromTheme(u"laser"_s), TR("ToolEditForm", "Laser", nullptr), Tool::Laser);

    connect(ui->cbxToolType, &QComboBox::currentIndexChanged, this, &ToolEditForm::setupToolWidgets);

    ui->lblWarn->setPixmap(QIcon::fromTheme(u"window-close"_s).pixmap({16, 16}));
    ui->lblWarn->setToolTip(QApplication::translate("ToolEditForm",
        "If the offset value is more than 50%, unmilled areas are possible.\nThese errors do not appear in the visualization.",
        "При значении отступа более 50% возможны не отфрезерованные участки. Эти ошибки не отображаются в визуализации."));

    QSettings settings;
    ui->cbxFeedSpeeds->setCurrentIndex(settings.value(u"cbxFeedSpeeds"_s).toInt());
    // ui->cbxUnits->setCurrentIndex(settings.value(u"cbxUnits"_s).toInt());
    setVisibleToolWidgets(false);
    setChanged(false);
}

ToolEditForm::~ToolEditForm() {
    QSettings settings;
    settings.setValue(u"cbxFeedSpeeds"_s, ui->cbxFeedSpeeds->currentIndex());
    // settings.setValue(u"cbxUnits"_s, ui->cbxUnits->currentIndex());
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

    const std::array dsbxList{
        ui->dsbxAngle,
        ui->dsbxDepth,
        ui->dsbxFeedRate,
        ui->dsbxLenght,
        ui->dsbxOneTurnCut,
        ui->dsbxPass,
        ui->dsbxPlungeRate,
        ui->dsbxStepover,
        ui->dsbxThreadPitch,
    };

    for(DoubleSpinBox* dsbx: dsbxList)
        dsbx->setMaximum(std::numeric_limits<double>::max());

    for(auto [dsbx, get]: get)
        dsbx->setValue((tool.*get)());

    for(DoubleSpinBox* dsbx: dsbxList)
        dsbx->setProperty("lastVal", qFuzzyIsNull(dsbx->value()) ? QVariant{} : QVariant{dsbx->value()});

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

void ToolEditForm::setupToolWidgets(int) {
    auto currType = ui->cbxToolType->currentData().value<Tool::Type>();
    tool_.setType(currType);

    // строки, которые скрываются/показываются в зависимости от типа инструмента
    static const std::array trackedDsbx{
        ui->dsbxAngle,
        ui->dsbxFeedRate,
        ui->dsbxLenght,
        ui->dsbxOneTurnCut,
        ui->dsbxPass,
        ui->dsbxThreadPitch,
        ui->dsbxDepth,
        ui->dsbxPlungeRate,
        ui->dsbxStepover,
        ui->dsbxHoleDiam,
    };

    // перед скрытием запоминаем значения ещё видимых полей, чтобы восстановить их при возврате к этому типу
    for(auto* dsbx: trackedDsbx)
        if(dsbx->isVisible()) dsbx->setProperty("lastVal", dsbx->value());

    // скрываем все строки во всех формах — дальше покажем только нужные для currType
    for(QFormLayout* form: {ui->formGeometry, ui->formCutParam, ui->formFeedSpeeds})
        for(int row: v::iota(0, form->rowCount()))
            form->setRowVisible(row, false);

    // строки, общие для любого типа инструмента
    ui->formGeometry->setRowVisible(ui->lblUnits, true);
    ui->formGeometry->setRowVisible(ui->dsbxDiameter, true);
    ui->formFeedSpeeds->setRowVisible(ui->dsbxSpindleSpeed, true);
    ui->formCutParam->setRowVisible(ui->label_2, true); // Vc и число зубьев

    // одиночное поле: виджет сам является и меткой, и полем своей строки формы
    auto show = [](QFormLayout* form, DoubleSpinBox* dsbx, double max, std::optional<double> defVal = {}) {
        dsbx->setMaximum(max);
        if(auto lastVal = dsbx->property("lastVal"); lastVal.isValid()) // восстановить последнее значение
            dsbx->setValue(lastVal.toDouble());
        else if(defVal) // значение по умолчанию
            dsbx->setValue(defVal.value());
        form->setRowVisible(dsbx, true);
    };

    // парное поле: dsbx делит строку формы с процентным дублёром через вложенный QHBoxLayout
    auto showPair = [](QFormLayout* form, QLayout* row, DoubleSpinBox* dsbx, double max) {
        dsbx->setMaximum(max);
        if(auto lastVal = dsbx->property("lastVal"); lastVal.isValid()) // восстановить последнее значение
            dsbx->setValue(lastVal.toDouble());
        form->setRowVisible(row, true);
    };

    const double diameter = ui->dsbxDiameter->value();

    switch(currType) {
    case Tool::Drill:
        show(ui->formCutParam, ui->dsbxPass, 10.);
        show(ui->formGeometry, ui->dsbxAngle, 180., 120.);
        show(ui->formGeometry, ui->dsbxLenght, 100000.);
        showPair(ui->formCutParam, ui->hlayOneTurnCut, ui->dsbxOneTurnCut, diameter);
        show(ui->formFeedSpeeds, ui->dsbxPlungeRate, 100000.);
        break;
    case Tool::EndMill:
        show(ui->formCutParam, ui->dsbxDepth, 10.);
        show(ui->formFeedSpeeds, ui->dsbxFeedRate, 100000.);
        show(ui->formGeometry, ui->dsbxLenght, 100000.);
        showPair(ui->formCutParam, ui->hlayOneTurnCut, ui->dsbxOneTurnCut, diameter);
        show(ui->formFeedSpeeds, ui->dsbxPlungeRate, 100000.);
        showPair(ui->formCutParam, ui->hlayStepover, ui->dsbxStepover, diameter);
        break;
    case Tool::ThreadMill:
        show(ui->formCutParam, ui->dsbxHoleDiam, 10.);
        show(ui->formCutParam, ui->dsbxThreadPitch, 10.);
        show(ui->formFeedSpeeds, ui->dsbxFeedRate, 100000.);
        show(ui->formGeometry, ui->dsbxLenght, 100000.);
        showPair(ui->formCutParam, ui->hlayOneTurnCut, ui->dsbxOneTurnCut, diameter);
        show(ui->formFeedSpeeds, ui->dsbxPlungeRate, 100000.);
        showPair(ui->formCutParam, ui->hlayStepover, ui->dsbxStepover, diameter);
        break;
    case Tool::Engraver:
        show(ui->formCutParam, ui->dsbxDepth, 10.);
        show(ui->formGeometry, ui->dsbxAngle, 180., 90.);
        show(ui->formFeedSpeeds, ui->dsbxFeedRate, 100000.);
        show(ui->formGeometry, ui->dsbxLenght, 100000.);
        showPair(ui->formCutParam, ui->hlayOneTurnCut, ui->dsbxOneTurnCut, diameter);
        show(ui->formFeedSpeeds, ui->dsbxPlungeRate, 100000.);
        showPair(ui->formCutParam, ui->hlayStepover, ui->dsbxStepover, diameter);
        break;
    case Tool::Laser:
        show(ui->formFeedSpeeds, ui->dsbxFeedRate, 100000.);
        showPair(ui->formCutParam, ui->hlayStepover, ui->dsbxStepover, diameter);
        break;
    default: break;
    }

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
        if(dsbx->isVisible() && qFuzzyIsNull(dsbx->value()))
            dsbx->flicker(), fl = true;

    if(fl)
        return;

    for(auto [dsbx, set]: set)
        if(dsbx->isVisible())
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
        case Tool::EndMill   : return tr("End Mill (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Engraver  : return tr("Engrave (%2\302\260 %1 mm tip)").arg(ui->dsbxDiameter->value()).arg(ui->dsbxAngle->value());
        case Tool::Drill     : return tr("Drill (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Laser     : return tr("Laser (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::ThreadMill: return tr("Thread (Ø%1 mm)").arg(ui->dsbxDiameter->value());
        case Tool::Group     :
        default              : return QString{};
        }
    }(ui->cbxToolType->currentData().value<Tool::Type>()));
}

void ToolEditForm::updateDsbxAngle(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setAngle(val);
}

void ToolEditForm::updateDsbxHoleDiam(double val) {
    tool_.setAngle(val);
}

void ToolEditForm::updateDsbxDiameter(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setDiameter(val);
    ui->dsbxOneTurnCut->setMaximum(val);
    ui->dsbxStepover->setMaximum(val);
    if(ui->dsbxStepover->value() == 0.)
        ui->dsbxStepover->setValue(val * 0.5);
    if(ui->dsbxOneTurnCut->value() == 0.)
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
    ui->dsbxOneTurnCutPercent->setValue(tool_.diameter() > 0. ? val / (tool_.diameter() * 0.1) : 0.);
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
    ui->dsbxStepoverPercent->setValue(tool_.diameter() > 0. ? val / (tool_.diameter() * 0.1) : 0.);
}

void ToolEditForm::updateDsbxLenght(double val) {
    // qDebug() << __FUNCTION__ << val;
    tool_.setLenght(val);
}

void ToolEditForm::updateDsbxOneTurnCutPercent(double val) {
    // qDebug() << __FUNCTION__ << val;
    ui->dsbxOneTurnCut->setValue(val * (tool_.diameter() * 0.1));
}

void ToolEditForm::updateDsbxStepoverPercent(double val) {
    // qDebug() << __FUNCTION__ << val;
    ui->dsbxStepover->setValue(val * (tool_.diameter() * 0.1));
}

#include "moc_tool_editform.cpp"
