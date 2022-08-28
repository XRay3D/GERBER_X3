// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

#include "project.h"
#include "qprogressdialog.h"

const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");

GcFormBase::GcFormBase(GCodePlugin* plugin, GCode::Creator* tpc, QWidget* parent)
    : QWidget(parent)
    , plugin {plugin}
    , creator {tpc}
    , progressDialog(new QProgressDialog(this)) {

    content = new QWidget(this);

    dsbxDepth = new DepthForm(this);
    dsbxDepth->setObjectName("dsbxDepth");

    leName = new QLineEdit(this);
    leName->setObjectName("leName");

    pbClose = new QPushButton(tr("Close"), this);
    pbClose->setIcon(QIcon::fromTheme("window-close"));
    pbClose->setObjectName("pbClose");

    pbCreate = new QPushButton(tr("Create"), this);
    pbCreate->setIcon(QIcon::fromTheme("document-export"));
    pbCreate->setObjectName("pbCreate");
    connect(pbCreate, &QPushButton::clicked, this, &GcFormBase::createFile);

    auto line = [this] {
        auto line = new QFrame(this);
        line->setFrameShadow(QFrame::Plain);
        line->setFrameShape(QFrame::HLine);
        return line;
    };

    grid = new QGridLayout(this);
    grid->setContentsMargins(6, 6, 6, 6);
    grid->setSpacing(6);

    int row {};
    constexpr int rowSpan {1}, columnSpan {2};
    grid->addWidget(dsbxDepth, row, 0, rowSpan, columnSpan);           // row 0
    grid->addWidget(line(), ++row, 0, rowSpan, columnSpan);            // row 1
    grid->addWidget(content, ++row, 0, rowSpan, columnSpan);           // row 2
    grid->addWidget(line(), ++row, 0, rowSpan, columnSpan);            // row 3
    grid->addWidget(new QLabel(tr("Name:"), this), ++row, 0);          // row 4
    grid->addWidget(leName, row, 1);                                   // row 4
    grid->addWidget(pbCreate, ++row, 0, rowSpan, columnSpan);          // row 5
    grid->addWidget(pbClose, ++row, 0, rowSpan, columnSpan);           // row 6
    grid->addWidget(new QWidget(this), ++row, 0, rowSpan, columnSpan); // row 7
    grid->setRowStretch(row, 1);

    progressDialog->setMinimumDuration(100);
    progressDialog->setModal(true);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    progressDialog->reset();
    connect(progressDialog, &QProgressDialog::canceled, this, &GcFormBase::cancel);

    if (!tpc)
        return;

    connect(this, &GcFormBase::createToolpath, this, &GcFormBase::startProgress);
    connect(this, &GcFormBase::createToolpath, tpc, &GCode::Creator::createGc);

    connect(tpc, &GCode::Creator::canceled, this, &GcFormBase::stopProgress);
    connect(tpc, &GCode::Creator::errorOccurred, this, &GcFormBase::errorHandler);
    connect(tpc, &GCode::Creator::fileReady, this, &GcFormBase::fileHandler);

    tpc->moveToThread(&thread);
    connect(&thread, &QThread::finished, tpc, &QObject::deleteLater);
    thread.start(QThread::LowPriority /*HighestPriority*/);
}

GcFormBase::~GcFormBase() {
    thread.quit();
    thread.wait();
}

void GcFormBase::fileHandler(GCode::File* file) {
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

void GcFormBase::timerEvent(QTimerEvent* event) {
    if (event->timerId() == progressTimerId && progressDialog && creator) {
        const auto [max, val] = creator->getProgress();
        progressDialog->setMaximum(max);
        progressDialog->setValue(val);
        progressDialog->setLabelText(creator->msg);
    }
}

void GcFormBase::addUsedGi(GraphicsItem* gi) {
    if (gi->file()) {
        FileInterface const* file = gi->file();
        if (file->type() == FileType::Gerber) {
#ifdef GBR_
            usedItems_[{file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType()}].push_back(gi->id());
#endif
        } else {
            usedItems_[{file->id(), -1}].push_back(gi->id());
        }
    }
}

void GcFormBase::cancel() {
    creator->continueCalc({});
    stopProgress();
}

void GcFormBase::errorHandler(int) {
    stopProgress();
    if (bool fl = ErrorDialog(std::move(creator->items), this).exec(); fl) {
        startProgress();
        creator->continueCalc(fl);
    } else
        creator->continueCalc(fl);
}

void GcFormBase::startProgress() {
    if (!fileCount)
        fileCount = 1;
    creator->msg = fileName_;
    progressDialog->setLabelText(creator->msg);
    progressTimerId = startTimer(100);
}

void GcFormBase::stopProgress() {
    killTimer(progressTimerId);
    progressTimerId = 0;
    progressDialog->reset();
    progressDialog->hide();
}
