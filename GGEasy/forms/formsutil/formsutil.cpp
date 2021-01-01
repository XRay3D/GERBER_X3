// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "formsutil.h"
#include "errordialog.h"

#ifdef GERBER
#include "gbrfile.h"
#endif
#include "gcode.h"
#include "gi/erroritem.h"
#include "project.h"
#include "qprogressdialog.h"
#include "scene.h"
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");

#include "leakdetector.h"

FormsUtil::FormsUtil(GCode::Creator* tps, QWidget* parent)
    : QWidget(parent)
    , m_tpc(tps)
    , fileCount(1)
    , progressDialog(new QProgressDialog(this))
{
    m_tpc->moveToThread(&thread);

    connect(&thread, &QThread::finished, m_tpc, &QObject::deleteLater);

    connect(m_tpc, &GCode::Creator::canceled, this, &FormsUtil::stopProgress, Qt::QueuedConnection);
    connect(m_tpc, &GCode::Creator::errorOccurred, this, &FormsUtil::errorHandler, Qt::QueuedConnection);
    connect(m_tpc, &GCode::Creator::fileReady, this, &FormsUtil::fileHandler, Qt::QueuedConnection);

    connect(progressDialog, &QProgressDialog::canceled, this, &FormsUtil::cancel, Qt::DirectConnection);

    connect(this, &FormsUtil::createToolpath, m_tpc, qOverload<>(&GCode::Creator::createGc));
    connect(this, &FormsUtil::createToolpath, this, &FormsUtil::startProgress);

    thread.start(QThread::LowPriority /*HighestPriority*/);

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
}

FormsUtil::~FormsUtil()
{
    thread.quit();
    thread.wait();
}

void FormsUtil::cancel()
{
    m_tpc->cancel();
    stopProgress();
}

void FormsUtil::errorHandler(int)
{
    stopProgress();
    flikerTimerId = startTimer(32);
    if (ErrorDialog(m_tpc->items, this).exec()) {
        startProgress();
        m_tpc->proceed();
    } else {
        m_tpc->cancel();
    }
    killTimer(flikerTimerId);
    flikerTimerId = 0;
}

void FormsUtil::startProgress()
{
    if (!fileCount)
        fileCount = 1;
    m_tpc->msg = m_fileName;
    progressDialog->setLabelText(m_tpc->msg);
    progressTimerId = startTimer(100);
}

void FormsUtil::stopProgress()
{
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}

void FormsUtil::fileHandler(GCode::File* file)
{
    if (--fileCount == 0)
        cancel();

    if (file == nullptr) {
        QMessageBox::information(this, tr("Warning"), tr("The tool doesn`t fit in the Working items!"));
        return;
    }

    file->setFileName(m_fileName + "_" + file->name());
    file->setSide(boardSide);
    if (fileId > -1) {
        App::project()->reload(fileId, file);
        m_editMode = false;
        fileId = -1;
    } else {
        App::project()->addFile(file);
    }
}

void FormsUtil::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == progressTimerId && progressDialog && m_tpc) {
        const auto [max, val] = m_tpc->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);

        progressDialog->setLabelText(m_tpc->msg);
    }
    if (event->timerId() == flikerTimerId) {
        App::scene()->update();
    }
}

void FormsUtil::addUsedGi(GraphicsItem* gi)
{
    if (gi->file()) {
        AbstractFile const* file = gi->file();
        if (file->type() == FileType::Gerber) {
#ifdef GERBER
            m_usedItems[{ file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType() }].append(gi->id());
#endif
        } else {
            m_usedItems[{ file->id(), -1 }].append(gi->id());
        }
    }
}
