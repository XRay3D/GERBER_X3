// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "formsutil.h"

#include "project.h"
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QTimer>
#include <gcode.h>
#include <qprogressdialog.h>
#include <scene.h>
#include <gbrfile.h>

const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");

FormsUtil::FormsUtil(GCode::Creator* tps, QWidget* parent)
    : QWidget(parent)
    , m_tpc(tps)
    , fileCount(1)
    , pd(new QProgressDialog(this))
{
    m_tpc->moveToThread(&thread);
    connect(m_tpc, &GCode::Creator::fileReady, this, &FormsUtil::setFile, Qt::QueuedConnection);
    connect(&thread, &QThread::finished, m_tpc, &QObject::deleteLater);
    connect(this, &FormsUtil::createToolpath, m_tpc, qOverload<>(&GCode::Creator::createGc));
    connect(this, &FormsUtil::createToolpath, [this] {
        if (!fileCount)
            fileCount = 1;
        m_tpc->msg = m_fileName;
        pd->setLabelText(m_tpc->msg);
        m_timerId = startTimer(100);
    });
    thread.start(QThread::LowPriority /*HighestPriority*/);

    pd->setMinimumDuration(100);
    pd->setModal(true);
    pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
    pd->setAutoClose(false);
    pd->setAutoReset(false);
    pd->reset();
    connect(pd, &QProgressDialog::canceled, this, &FormsUtil::cancel, Qt::DirectConnection);
}

FormsUtil::~FormsUtil()
{
    qDebug(Q_FUNC_INFO);
    thread.quit();
    thread.wait();
}

void FormsUtil::cancel()
{
    m_tpc->cancel();
    killTimer(m_timerId);
    m_timerId = 0;
    pd->reset();
    pd->hide();
}

void FormsUtil::setFile(GCode::File* file)
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
    if (event->timerId() == m_timerId && pd && m_tpc) {
        const auto [max, val] = m_tpc->getProgress();
        pd->setMaximum(max);
        pd->setValue(val);
        //qDebug() << "timerEvent" << max << val;
        pd->setLabelText(m_tpc->msg);
    }
}

void FormsUtil::addUsedGi(GraphicsItem* gi)
{
    if (gi->file()) {
        AbstractFile const* file = gi->file();
        if (file->type() == FileType::Gerber) {
            m_usedItems[{ file->id(), reinterpret_cast<const Gerber::File*>(file)->itemsType() }].append(gi->id());
        } else {
            m_usedItems[{ file->id(), -1 }].append(gi->id());
        }
    }
}
