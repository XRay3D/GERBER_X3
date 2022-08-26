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

    auto grid = new QGridLayout(this);
    grid->setContentsMargins(6, 6, 6, 6);
    grid->setSpacing(6);
    {
        auto frame = new QFrame(this);
        frame->setFrameShadow(QFrame::Plain);
        frame->setFrameShape(QFrame::Box);
        label = new QLabel("Label", frame);
        auto layout = new QVBoxLayout(frame);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->addWidget(label);

        grid->addWidget(frame, 0, 0, 1, 2); // 0
        dsbxDepth = new DepthForm(this);
        dsbxDepth->setObjectName("dsbxDepth");
        grid->addWidget(dsbxDepth, 1, 0, 1, 2); // 1
    }

    {
        auto line = new QFrame(this);
        line->setFrameShadow(QFrame::Plain);
        line->setFrameShape(QFrame::HLine);
        grid->addWidget(line, 2, 0, 1, 2); // 2
    }

    {
        content = new QWidget(this);
        grid->addWidget(content, 3, 0, 1, 2); // 3
    }

    {
        auto line = new QFrame(this);
        line->setFrameShadow(QFrame::Plain);
        line->setFrameShape(QFrame::HLine);
        grid->addWidget(line, 4, 0, 1, 2); // 4
    }
    {
        auto label = new QLabel(tr("Name:"), this);

        leName = new QLineEdit(this);
        leName->setObjectName("leName");

        pbClose = new QPushButton(tr("Close"), this);
        pbClose->setObjectName("pbClose");
        pbClose->setIcon(QIcon::fromTheme("window-close"));

        pbCreate = new QPushButton(tr("Create"), this);
        pbCreate->setObjectName("pbCreate");
        pbCreate->setIcon(QIcon::fromTheme("document-export"));

        grid->addWidget(label, 5, 0);                   // 5
        grid->addWidget(leName, 5, 1);                  // 5
        grid->addWidget(pbCreate, 6, 0, 1, 2);          // 6
        grid->addWidget(pbClose, 7, 0, 1, 2);           // 7
        grid->addWidget(new QWidget(this), 9, 0, 1, 2); // 9
    }
    grid->setRowStretch(8, 1);

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
        //        editMode_ = false;
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
            usedItems_[{ file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType() }].push_back(gi->id());
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
