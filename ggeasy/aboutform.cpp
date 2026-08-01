/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
// #include "a_pch.h"
#include "aboutform.h"
#include "ui_aboutform.h"
#include <QDesktopServices>
using namespace Qt::Literals;

AboutForm::AboutForm(QWidget* parent)
    : QDialog{parent}
    , ui(new Ui::AboutForm) {
    ui->setupUi(this);
    QString str =
#ifdef __MINGW32__
        //__MINGW_GCC_VERSION
        QString::fromUtf8(BUILD_DATE).append(u"<br/>MINGW: GCC(%1.%2.%3) MINGW(%4)"_s.arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__).arg(__MINGW64_VERSION_STR));
#elif __GNUG__ // specific variant for GCC
        QString::fromUtf8(BUILD_DATE).append(u"<br/>GCC_VER: ") + QString::fromUtf8(__VERSION__);
#else
        QString(/*BUILD_DATE*/ __DATE__ u" "_s __TIME__).append(u"<br/>MSC_VER: "_s) + QString::number(_MSC_VER);
#endif
    str.append(u"<br/>Git: " GIT_REF_NAME u":" GIT_SHA);
    str.push_back(u"<br/>Application Version: " + qApp->applicationVersion());
    ui->lblAbout->setText(ui->lblAbout->text().arg(/*qApp->applicationVersion()*/ str));
    connect(ui->cmdOk_2, &QPushButton::clicked, this, &AboutForm::accept);
    connect(ui->lblAbout, &QLabel::linkActivated, [](const QString& link) { QDesktopServices::openUrl(QUrl{link}); });
}

AboutForm::~AboutForm() { delete ui; }

#include "moc_aboutform.cpp"
