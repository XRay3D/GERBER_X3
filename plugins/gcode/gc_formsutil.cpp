// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_formsutil.h"
#include "gc_errordialog.h"

#include "gcode.h"

#include "project.h"
#include "qprogressdialog.h"
#include "scene.h"

const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");

FormsUtil::FormsUtil(GCodePlugin* plugin, GCode::Creator* tpc, QWidget* parent)
    : QWidget(parent)
    , plugin { plugin }
    , creator(tpc)
    , fileCount(1)
    , progressDialog(new QProgressDialog(this)) {
    tpc->moveToThread(&thread);

    connect(&thread, &QThread::finished, tpc, &QObject::deleteLater);

    connect(tpc, &GCode::Creator::canceled, this, &FormsUtil::stopProgress);
    connect(tpc, &GCode::Creator::errorOccurred, this, &FormsUtil::errorHandler);
    connect(tpc, &GCode::Creator::fileReady, this, &FormsUtil::fileHandler);

    connect(progressDialog, &QProgressDialog::canceled, this, &FormsUtil::cancel);

    connect(this, &FormsUtil::createToolpath, tpc, &GCode::Creator::createGc);
    connect(this, &FormsUtil::createToolpath, this, &FormsUtil::startProgress);

    thread.start(QThread::LowPriority /*HighestPriority*/);

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
}

FormsUtil::~FormsUtil() {
    thread.quit();
    thread.wait();
}

void FormsUtil::fileHandler(GCode::File* file) {
    qDebug() << __FUNCTION__ << file;
    if (--fileCount == 0)
        cancel();

    if (file == nullptr) {
        QMessageBox::information(this, tr("Warning"), tr("The tool doesn`t fit in the Working items!"));
        return;
    }

    file->setFileName(fileName_ + "_" + file->name());
    file->setSide(boardSide);
    if (fileId > -1) {
        exit(-123456);
        //        App::project()->reload(fileId, file);
        //        m_editMode = false;
        //        fileId = -1;
    } else {
        App::project()->addFile(file);
    }
}

void FormsUtil::timerEvent(QTimerEvent* event) {
    if (event->timerId() == progressTimerId && progressDialog && creator) {
        const auto [max, val] = creator->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(creator->msg);
    }
}

void FormsUtil::addUsedGi(GraphicsItem* gi) {
    if (gi->file()) {
        FileInterface const* file = gi->file();
        if (file->type() == FileType::Gerber) {
#ifdef GBR_
            m_usedItems[{ file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType() }].push_back(gi->id());
#endif
        } else {
            usedItems_[{ file->id(), -1 }].push_back(gi->id());
        }
    }
}

void FormsUtil::cancel() {
    creator->continueCalc({});
    stopProgress();
}

void FormsUtil::errorHandler(int) {
    stopProgress();
    if (bool fl = ErrorDialog(std::move(creator->items), this).exec(); fl) {
        startProgress();
        creator->continueCalc(fl);
    } else
        creator->continueCalc(fl);
}

void FormsUtil::startProgress() {
    if (!fileCount)
        fileCount = 1;
    creator->msg = fileName_;
    progressDialog->setLabelText(creator->msg);
    progressTimerId = startTimer(100);
}

void FormsUtil::stopProgress() {
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}
