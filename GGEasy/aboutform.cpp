// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "aboutform.h"
#include "ui_aboutform.h"
#include <QDesktopServices>

#include "leakdetector.h"

AboutForm::AboutForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutForm)
{
    ui->setupUi(this);
#ifdef __MINGW32__
    //__MINGW_GCC_VERSION
    QString str(QString(BUILD_DATE).append("<br/>MINGW: GCC(%1.%2.%3) MINGW(%4)").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__).arg(__MINGW64_VERSION_STR));
#elif __GNUG__ // specific variant for GCC
    QString str(QString(BUILD_DATE).append("<br/>GCC_VER: ") + QString(__VERSION__));
#else
    QString str(QString(/*BUILD_DATE*/__DATE__ " " __TIME__).append("<br/>MSC_VER: ") + QString::number(_MSC_VER));
#endif
    str.push_back("<br/>Application Version: " + qApp->applicationVersion());
    ui->lblAbout->setText(ui->lblAbout->text().arg(/*qApp->applicationVersion()*/ str));
    connect(ui->cmdOk_2, &QPushButton::clicked, this, &AboutForm::accept);
    connect(ui->lblAbout, &QLabel::linkActivated, [](const QString& link) { QDesktopServices::openUrl(link); });
}

AboutForm::~AboutForm() { delete ui; }
