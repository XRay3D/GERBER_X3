// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "profileform.h"
#include "ui_profileform.h"

#include "gi/bridgeitem.h"
#include "scene.h"
#include "settings.h"
#include <QMessageBox>

ProfileForm::ProfileForm(QWidget* parent)
    : FormsUtil(new GCode::ProfileCreator, parent)
    , ui(new Ui::ProfileForm)
    , names { tr("Profile On"), tr("Profile Outside"), tr("Profile Inside") }
    , pixmaps {
        QStringLiteral(":/toolpath/prof_on_climb.svg"),
        QStringLiteral(":/toolpath/prof_out_climb.svg"),
        QStringLiteral(":/toolpath/prof_in_climb.svg"),
        QStringLiteral(":/toolpath/prof_on_conv.svg"),
        QStringLiteral(":/toolpath/prof_out_conv.svg"),
        QStringLiteral(":/toolpath/prof_in_conv.svg"),
    }
{
    ui->setupUi(this);
    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOn);
    settings.getValue(ui->rbOutside);
    settings.getValue(ui->cbxStrip);
    settings.endGroup();

    // ui->gridLayout->addWidget(ui->labelPixmap, 0, 1, 2, 1, Qt::AlignHCenter);

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &ProfileForm::rb_clicked);

    connect(ui->dsbxBridgeLenght, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ProfileForm::updateBridge);
    connect(ui->dsbxDepth, &DepthForm::valueChanged, this, &ProfileForm::updateBridge);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &ProfileForm::updateName);

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &ProfileForm::createFile);
}

ProfileForm::~ProfileForm()
{
    qDebug(Q_FUNC_INFO);

    MySettings settings;
    settings.beginGroup("ProfileForm");
    settings.setValue(ui->dsbxBridgeLenght);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOn);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->cbxStrip);
    settings.endGroup();

    for (QGraphicsItem* item : App::scene()->items()) {
        if (item->type() == GiBridge)
            delete item;
    }
    delete ui;
}

void ProfileForm::createFile()
{
    m_usedItems.clear();
    const auto tool { ui->toolHolder->tool() };
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;
    bool skip { true };

    for (auto* item : App::scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiGerber:
        case GiAperturePath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            if (item->type() == GiGerber)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            break;
        case GiShapeC:
        case GiShapeR:
        case GiShapeL:
        case GiShapeA:
        case GiShapeT:
            wRawPaths.append(gi->paths());
            break;
        case GiDrill:
            wPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wRawPaths.isEmpty() && wPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gcp;
    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.tools.append(tool);
    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    if (ui->cbxStrip->isEnabled())
        gcp.params[GCode::GCodeParams::Strip] = ui->cbxStrip->isChecked();
    gcp.params[GCode::GCodeParams::GrItems].setValue(m_usedItems);

    {
        QVector<QPointF> brv;
        for (QGraphicsItem* item : App::scene()->items()) {
            if (item->type() == GiBridge)
                brv.append(item->pos());
        }
        if (!brv.isEmpty()) {
            gcp.params[GCode::GCodeParams::Bridges].setValue(brv);
            gcp.params[GCode::GCodeParams::BridgeLen].setValue(ui->dsbxBridgeLenght->value());
        }
    }

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    createToolpath();
}

void ProfileForm::updateName()
{
    ui->leName->setText(names[side]);
    updateBridge();
}

void ProfileForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void ProfileForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void ProfileForm::on_pbAddBridge_clicked()
{
    if (item) {
        if (!item->ok())
            delete item;
    }
    item = new BridgeItem(m_lenght, m_size, side, item);
    App::scene()->addItem(item);
}

void ProfileForm::updateBridge()
{
    m_lenght = ui->dsbxBridgeLenght->value();
    m_size = ui->toolHolder->tool().getDiameter(ui->dsbxDepth->value());
    for (QGraphicsItem* item : App::scene()->items()) {
        if (item->type() == GiBridge)
            dynamic_cast<BridgeItem*>(item)->update();
    }
}

void ProfileForm::updatePixmap()
{
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void ProfileForm::rb_clicked()
{
    if (ui->rbOn->isChecked()) {
        side = GCode::On;
        ui->cbxStrip->setEnabled(true);
    } else if (ui->rbOutside->isChecked()) {
        side = GCode::Outer;
        ui->cbxStrip->setEnabled(false);
    } else if (ui->rbInside->isChecked()) {
        side = GCode::Inner;
        ui->cbxStrip->setEnabled(false);
    }

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updatePixmap();
}

void ProfileForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ProfileForm::editFile(GCode::File* file)
{
    qDebug(Q_FUNC_INFO);
    GCode::GCodeParams gcp { file->gcp() };

    fileId = gcp.fileId;
    m_editMode = true;

    { // GUI
        side = gcp.side();
        direction = static_cast<GCode::Direction>(gcp.convent());
        ui->toolHolder->setTool(gcp.tools.first());
        ui->dsbxDepth->setValue(gcp.params[GCode::GCodeParams::Depth].toDouble());

        switch (side) {
        case GCode::On:
            ui->rbOn->setChecked(true);
            break;
        case GCode::Outer:
            ui->rbOutside->setChecked(true);
            break;
        case GCode::Inner:
            ui->rbInside->setChecked(true);
            break;
        }

        switch (direction) {
        case GCode::Climb:
            ui->rbClimb->setChecked(true);
            break;
        case GCode::Conventional:
            ui->rbConventional->setChecked(true);
            break;
        }
    }

    { // GrItems
        m_usedItems.clear();
        auto items { gcp.params[GCode::GCodeParams::GrItems].value<UsedItems>() };
        qDebug() << items;
        auto i = items.constBegin();
        while (i != items.constEnd()) {
            qDebug() << i.key() << i.value();
            auto [fileId, _] = i.key();
            Q_UNUSED(_)
            App::project()->aFile(fileId)->itemGroup()->setSelected(i.value());
            ++i;
        }
    }

    { // Bridges
        if (gcp.params.contains(GCode::GCodeParams::Bridges)) {
            ui->dsbxBridgeLenght->setValue(gcp.params[GCode::GCodeParams::BridgeLen].toDouble());
            for (auto& pos : gcp.params[GCode::GCodeParams::Bridges].value<QVector<QPointF>>()) {
                item = new BridgeItem(m_lenght, m_size, side, item);
                App::scene()->addItem(item);
                item->setPos(pos);
                item->m_lastPos = pos;
            }
            updateBridge();
            item = new BridgeItem(m_lenght, m_size, side, item);
            //        delete item;
        }
    }
}
