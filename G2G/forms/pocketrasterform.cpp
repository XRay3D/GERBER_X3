// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pocketrasterform.h"
#include "ui_pocketrasterform.h"

#include "scene.h"
#include "settings.h"
#include <QMessageBox>

PocketRasterForm::PocketRasterForm(QWidget* parent)
    : FormsUtil(new GCode::RasterCreator, parent)
    , ui(new Ui::PocketRasterForm)
    , names { tr("Raster On"), tr("Raster Outside"), tr("Raster Inside") }
    , pixmaps {
        QStringLiteral(":/toolpath/pock_rast_climb.svg"),
        QStringLiteral(":/toolpath/pock_rast_conv.svg"),
    }
{
    ui->setupUi(this);
    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.getValue(ui->cbxPass);
    settings.getValue(ui->dsbxAcc);
    settings.getValue(ui->dsbxAngle);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbFast);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbNormal);
    settings.getValue(ui->rbOutside);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &PocketRasterForm::updateName);

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &PocketRasterForm::createFile);
}

PocketRasterForm::~PocketRasterForm()
{
    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.setValue(ui->cbxPass);
    settings.setValue(ui->dsbxAcc);
    settings.setValue(ui->dsbxAngle);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbFast);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbNormal);
    settings.setValue(ui->rbOutside);
    settings.endGroup();
    delete ui;
}

void PocketRasterForm::createFile()
{
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

    gcp.params[GCode::GCodeParams::UseAngle] = ui->dsbxAngle->value();
    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    gcp.params[GCode::GCodeParams::Pass] = ui->cbxPass->currentIndex();
    if (ui->rbFast->isChecked()) {
        gcp.params[GCode::GCodeParams::Fast] = true;
        gcp.params[GCode::GCodeParams::AccDistance] = (tool.feedRateMmS() * tool.feedRateMmS()) / (2 * ui->dsbxAcc->value());
    }

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    fileCount = 1;
    createToolpath();
}

void PocketRasterForm::updateName()
{
    const auto& tool { ui->toolHolder->tool() };
    if (tool.type() != Tool::Laser)
        ui->rbNormal->setChecked(true);
    ui->rbFast->setEnabled(tool.type() == Tool::Laser);

    ui->leName->setText(names[side]);
}

void PocketRasterForm::updatePixmap()
{
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmaps[direction]).pixmap(QSize(size, size)));
}

void PocketRasterForm::rb_clicked()
{

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updatePixmap();
}

void PocketRasterForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketRasterForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketRasterForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void PocketRasterForm::editFile(GCode::File* /*file*/)
{
}
