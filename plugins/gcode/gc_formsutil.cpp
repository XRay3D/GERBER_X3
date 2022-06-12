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

FormsUtil::FormsUtil(GCodePlugin* plugin, GCode::Creator* tps, QWidget* parent)
    : QWidget(parent)
    , plugin { plugin }
    , tpc_(tps)
    , fileCount(1)
    , progressDialog(new QProgressDialog(this)) {
    tpc_->moveToThread(&thread);

    connect(&thread, &QThread::finished, tpc_, &QObject::deleteLater);

    connect(tpc_, &GCode::Creator::canceled, this, &FormsUtil::stopProgress, Qt::QueuedConnection);
    connect(tpc_, &GCode::Creator::errorOccurred, this, &FormsUtil::errorHandler, Qt::QueuedConnection);
    connect(tpc_, &GCode::Creator::fileReady, this, &FormsUtil::fileHandler, Qt::QueuedConnection);

    connect(progressDialog, &QProgressDialog::canceled, this, &FormsUtil::cancel, Qt::DirectConnection);

    connect(this, &FormsUtil::createToolpath, tpc_, qOverload<>(&GCode::Creator::createGc));
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
    if (event->timerId() == progressTimerId && progressDialog && tpc_) {
        const auto [max, val] = tpc_->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(tpc_->msg);
    }
    if (event->timerId() == flikerTimerId) {
        App::scene()->update();
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
    tpc_->cancel();
    stopProgress();
}

void FormsUtil::errorHandler(int) {
    stopProgress();
    flikerTimerId = startTimer(32);
    if (ErrorDialog(tpc_->items, this).exec()) {
        startProgress();
        tpc_->proceed();
    } else {
        tpc_->cancel();
    }
    killTimer(flikerTimerId);
    flikerTimerId = 0;
}

void FormsUtil::startProgress() {
    if (!fileCount)
        fileCount = 1;
    tpc_->msg = fileName_;
    progressDialog->setLabelText(tpc_->msg);
    progressTimerId = startTimer(100);
}

void FormsUtil::stopProgress() {
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}
