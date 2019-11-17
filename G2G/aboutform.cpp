#include "aboutform.h"
#include "ui_aboutform.h"
#include <QDesktopServices>

AboutForm::AboutForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutForm)
{
    ui->setupUi(this);
#ifdef MINGW_DDK_H
    QString str(QString(__TIMESTAMP__).append("<br/>MINGW: ") + QString::number(__MINGW32_MAJOR_VERSION) + "." + QString::number(__MINGW32_MINOR_VERSION));
#elif __GNUG__ // specific variant for GCC
    QString str(QString(__TIMESTAMP__).append("<br/>GCC_VER: ") + QString(__VERSION__));
#else
    QString str(QString(__TIMESTAMP__).append("<br/>MSC_VER: ") + QString::number(_MSC_VER));
#endif
    str.append("<br/>Application Version: " + qApp->applicationVersion());
    ui->lblAbout->setText(ui->lblAbout->text().arg(/*qApp->applicationVersion()*/ str));
    connect(ui->cmdOk_2, &QPushButton::clicked, this, &AboutForm::accept);
    connect(ui->lblAbout, &QLabel::linkActivated, [](const QString& link) { QDesktopServices::openUrl(link); });
}
AboutForm::~AboutForm()
{
    delete ui;
}

