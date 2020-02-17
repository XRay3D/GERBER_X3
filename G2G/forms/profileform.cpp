#include "profileform.h"
#include "ui_profileform.h"

#include "filetree/filemodel.h"
#include "gcodepropertiesform.h"
#include "gi/bridgeitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <gcfile.h>
#include <gcprofile.h>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

ProfileForm::ProfileForm(QWidget* parent)
    : FormsUtil("ProfileForm", new GCode::ProfileCreator, parent)
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

    ui->toolName->setTool(tool);

    auto rb_clicked = [&] {
        if (ui->rbOn->isChecked())
            side = GCode::On;
        else if (ui->rbOutside->isChecked())
            side = GCode::Outer;
        else if (ui->rbInside->isChecked())
            side = GCode::Inner;

        if (ui->rbClimb->isChecked())
            direction = GCode::Climb;
        else if (ui->rbConventional->isChecked())
            direction = GCode::Conventional;

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
    ui->dsbxBridgeLenght->setValue(settings.value("dsbxBridgeLenght", 1.0).toDouble());
    settings.endGroup();

    // ui->gridLayout->addWidget(ui->labelPixmap, 0, 1, 2, 1, Qt::AlignHCenter);

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("view-form"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    ui->pbAddBridge->setIcon(QIcon::fromTheme("edit-cut"));

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
    settings.endGroup();
    for (QGraphicsItem* item : Scene::items()) {
        if (item->type() == GiBridge)
            delete item;
    }
    delete ui;
}

void ProfileForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving, Tool::Laser });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->toolName->setTool(tool);
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
        ui->toolName->setTool(tool);
        updateName();
    }
}

void ProfileForm::on_pbCreate_clicked()
{
    createFile();
}

void ProfileForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void ProfileForm::createFile()
{
    m_usedItems.clear();

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;

    for (auto* item : Scene::selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiGerber:
        case GiAperturePath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                QMessageBox::warning(this, tr("Warning"), tr("Working items from different files!"));
                return;
            }
            if (item->type() == GiGerber)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            m_usedItems[gi->file()->id()].append(gi->id());
            break;
        case GiShapeC:
            wRawPaths.append(gi->paths());
            //m_used[gi->file()->id()].append(gi->id());
            break;
        case GiDrill:
            wPaths.append(gi->paths());
            m_usedItems[gi->file()->id()].append(gi->id());
            break;
        default:
            break;
        }
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
    static BridgeItem* item = nullptr;
    if (item) {
        if (!item->ok())
            delete item;
    }
    item = new BridgeItem(m_lenght, m_size, side, item);
    Scene::addItem(item);
}

void ProfileForm::on_dsbxBridgeLenght_valueChanged(double /*arg1*/) { updateBridge(); }

void ProfileForm::on_dsbxDepth_valueChanged(double /*arg1*/) { updateBridge(); }

void ProfileForm::updateBridge()
{
    m_lenght = ui->dsbxBridgeLenght->value();
    m_size = tool.getDiameter(ui->dsbxDepth->value());
    for (QGraphicsItem* item : Scene::items()) {
        if (item->type() == GiBridge)
            dynamic_cast<BridgeItem*>(item)->update();
    }
}

void ProfileForm::updatePixmap()
{
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void ProfileForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ProfileForm::editFile(GCode::File* /*file*/)
{
}
