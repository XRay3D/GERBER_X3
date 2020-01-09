#include "formsutil.h"

#include "project.h"
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPushButton>
#include <QTimer>
#include <gcvoronoi.h>
#include <qprogressdialog.h>
#include <scene.h>

const int gcpId = qRegisterMetaType<GCode::GCodeParams>("GCode::GCodeParams");

FormsUtil::FormsUtil(const QString& name, GCode::Creator* tps, QWidget* parent)
    : QWidget(parent)
    , m_tpc(tps)
    , m_name(qApp->applicationDirPath() + "/XrSoft/" + name + ".dat")
    , pd(new QProgressDialog(this))
{
    readTools({ &tool, &tool2 });

    fileCount = 1;
    m_tpc = tps;
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
    thread.start(QThread::HighestPriority);

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
    thread.quit();
    thread.wait();
    writeTools({ tool, tool2 });
}

void FormsUtil::readTools(const QVector<Tool*>& tools) const
{
    QFile file(m_name);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(file.readAll()));
    QJsonObject json = loadDoc.object();
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tools[treeIndex]->read(toolObject);
    }
}

void FormsUtil::writeTools(const QVector<Tool>& tools) const
{
    QFile file(m_name);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonArray toolArray;
    for (const Tool& tool : tools) {
        QJsonObject toolObject;
        tool.write(toolObject);
        toolArray.append(toolObject);
    }
    QJsonObject json;
    json["tools"] = toolArray;
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toBinaryData());
}

//void FormsUtil::showProgress()
//{
//    pd = new QProgressDialog(this);
//    pd->setMinimumDuration(0);
//    pd->setLabelText(m_fileName);
//    pd->setModal(true);
//    pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
//    connect(pd, &QProgressDialog::canceled, m_tpc, &GCode::Creator::cancel);
//    m_timerId = startTimer(100);
//    qDebug("FormsUtil::showProgress()");
//}

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

    file->setFileName(m_fileName + " (" + file->name() + ")");
    file->setSide(boardSide);
    file->m_usedItems = m_usedItems;
    Project::instance()->addFile(file);
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
