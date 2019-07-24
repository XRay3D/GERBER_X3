#include "profileform.h"
#include "ui_profileform.h"

#include "filetree/filemodel.h"
#include "gcode/gcfile.h"
#include "gcodepropertiesform.h"
#include "gi/bridgeitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

ProfileForm::ProfileForm(QWidget* parent)
    : FormsUtil("ProfileForm", parent)
    , ui(new Ui::ProfileForm)

{
    ui->setupUi(this);

    ui->lblToolName->setText(tool.name());
    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    auto rb_clicked = [&] {
        if (ui->rbOn->isChecked())
            side = On;
        else if (ui->rbOutside->isChecked())
            side = Outer;
        else if (ui->rbInside->isChecked())
            side = Inner;

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

        updateName();
        updatePixmap();
    };

    QSettings settings;
    settings.beginGroup("ProfileForm");
    if (settings.value("rbClimb").toBool())
        ui->rbClimb->setChecked(true);
    if (settings.value("rbConventional").toBool())
        ui->rbConventional->setChecked(true);
    if (settings.value("rbInside").toBool())
        ui->rbInside->setChecked(true);
    if (settings.value("rbOn").toBool())
        ui->rbOn->setChecked(true);
    if (settings.value("rbOutside").toBool())
        ui->rbOutside->setChecked(true);

    ui->dsbxDepth->setValue(settings.value("dsbxDepth").toDouble());
    if (settings.value("rbBoard").toBool())
        ui->dsbxDepth->rbBoard->setChecked(true);
    if (settings.value("rbCopper").toBool())
        ui->dsbxDepth->rbCopper->setChecked(true);

    ui->dsbxBridgeLenght->setValue(settings.value("dsbxBridgeLenght").toDouble());
    settings.endGroup();

    // ui->gridLayout->addWidget(ui->labelPixmap, 0, 1, 2, 1, Qt::AlignHCenter);

    ui->pbEdit->setIcon(Icon(ButtonEditIcon));
    ui->pbSelect->setIcon(Icon(ButtonSelectIcon));
    ui->pbClose->setIcon(Icon(ButtonCloseIcon));
    ui->pbCreate->setIcon(Icon(ButtonCreateIcon));
    ui->pbAddBridge->setIcon(Icon(ButtonAddBridgeIcon));

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    rb_clicked();
    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOn, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);
    parent->setWindowTitle(ui->label->text());
}

ProfileForm::~ProfileForm()
{

    QSettings settings;
    settings.beginGroup("ProfileForm");
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rbInside", ui->rbInside->isChecked());
    settings.setValue("rbOn", ui->rbOn->isChecked());
    settings.setValue("rbOutside", ui->rbOutside->isChecked());
    settings.setValue("dsbxBridgeLenght", ui->dsbxBridgeLenght->value());

    settings.setValue("dsbxDepth", ui->dsbxDepth->value(true));
    settings.setValue("rbBoard", ui->dsbxDepth->rbBoard->isChecked());
    settings.setValue("rbCopper", ui->dsbxDepth->rbCopper->isChecked());
    settings.endGroup();

    for (QGraphicsItem* item : Scene::items()) {
        if (item->type() == BridgeType)
            delete item;
    }
    delete ui;
}

void ProfileForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name());
        updateName();
    }
}

void ProfileForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.setTool(tool);
    if (d.exec()) {
        tool = d.tool();
        tool.setId(-1);
        ui->lblToolName->setText(tool.name());
        updateName();
    }
}

void ProfileForm::on_pbCreate_clicked()
{
    create();
}

void ProfileForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void ProfileForm::create()
{
    m_used.clear();

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wrPaths;
    AbstractFile const* file = nullptr;

    for (QGraphicsItem* item : Scene::selectedItems()) {
        GraphicsItem* gi = static_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GerberItemType:
        case RawItemType:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
            if (item->type() == GerberItemType)
                wPaths.append(gi->paths());
            else
                wrPaths.append(gi->paths());
            m_used[gi->file()->id()].append(gi->id());
            break;
        case DrillItemType:
            wPaths.append(gi->paths());
            m_used[gi->file()->id()].append(gi->id());
            break;
        default:
            break;
        }
    }

    if (wrPaths.isEmpty() && wPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::Creator* tpc = toolPathCreator(wPaths, ui->rbConventional->isChecked(), side);
    tpc->addRawPaths(wrPaths);
    connect(this, &ProfileForm::createProfile, tpc, &GCode::Creator::createProfile);
    emit createProfile(tool, ui->dsbxDepth->value());
    showProgress();
}

void ProfileForm::updateName()
{
    static const QStringList name = { tr("Profile On"), tr("Profile Outside"), tr("Profile Inside") };
    ui->leName->setText(name[side]);
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
    static BridgeItem* item = nullptr;
    if (item) {
        if (!item->ok())
            delete item;
    }
    item = new BridgeItem(m_lenght, m_size, item);
    GraphicsView::self->scene()->addItem(item);
}

void ProfileForm::on_dsbxBridgeLenght_valueChanged(double /*arg1*/) { updateBridge(); }

void ProfileForm::on_dsbxDepth_valueChanged(double /*arg1*/) { updateBridge(); }

void ProfileForm::updateBridge()
{
    m_lenght = ui->dsbxBridgeLenght->value();
    m_size = tool.getDiameter(ui->dsbxDepth->value());
    for (QGraphicsItem* item : Scene::items()) {
        if (item->type() == BridgeType)
            static_cast<BridgeItem*>(item)->update();
    }
}

void ProfileForm::updatePixmap()
{
    static const QStringList pixmapList{
        QStringLiteral(":/toolpath/prof_on_climb.svg"),
        QStringLiteral(":/toolpath/prof_out_climb.svg"),
        QStringLiteral(":/toolpath/prof_in_climb.svg"),
        QStringLiteral(":/toolpath/prof_on_conv.svg"),
        QStringLiteral(":/toolpath/prof_out_conv.svg"),
        QStringLiteral(":/toolpath/prof_in_conv.svg"),
    };
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmapList[side + direction * 3]).pixmap(QSize(size, size)));
}

void ProfileForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ProfileForm::editFile(GCode::File* /*file*/)
{
}
