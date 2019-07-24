#include "formsutil.h"

#include "project.h"
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <qprogressdialog.h>
#include <scene.h>

FormsUtil::FormsUtil(const QString& name, QWidget* parent)
    : QWidget(parent)
    , m_name(name + ".dat")
{
    readTools({ &tool, &tool2 });
}

FormsUtil::~FormsUtil()
{
    writeTools({ &tool, &tool2 });
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

void FormsUtil::writeTools(const QVector<Tool*>& tools) const
{
    QFile file(m_name);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonArray toolArray;
    for (Tool* tool : tools) {
        QJsonObject toolObject;
        tool->write(toolObject);
        toolArray.append(toolObject);
    }
    QJsonObject json;
    json["tools"] = toolArray;
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toBinaryData());
}

GCode::Creator* FormsUtil::toolPathCreator(const Paths& value, const bool convent, SideOfMilling side)
{
    //Scene::selectedItems();

    fileCount = 1;
    thread.wait();
    m_tps = new GCode::Creator(value, convent, side);
    m_tps->moveToThread(&thread);
    connect(m_tps, &GCode::Creator::fileReady, this, &FormsUtil::setFile, Qt::QueuedConnection);
    connect(&thread, &QThread::finished, m_tps, &QObject::deleteLater);
    thread.start(QThread::HighestPriority);
    return m_tps;
}

void FormsUtil::showProgress()
{
    if (pd)
        delete pd;
    pd = new QProgressDialog(this);
    pd->setLabelText(m_fileName);
    pd->setMaximum(/*max*/ 0);
    pd->setModal(true);
    pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
    pd->show();
    connect(pd, &QProgressDialog::canceled, this, &FormsUtil::cancel);
    m_timerId = startTimer(50);
}

void FormsUtil::cancel()
{
    thread.requestInterruption();
    thread.quit();
    thread.wait();
    if (pd) {
        pd->deleteLater();
        pd = nullptr;
    }
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    qDebug("canceled");
}

void FormsUtil::setFile(GCode::File* file)
{
    if (pd && !(--fileCount))
        cancel();

    if (file == nullptr) {
        QMessageBox::information(this, tr("Warning"), tr("The tool doesn`t fit in the Working items!"));
        return;
    }

    file->setFileName(m_fileName + " (" + file->name() + ")");
    file->setSide(boardSide);
    file->m_used = m_used;
    Project::addFile(file);
}

void FormsUtil::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId && pd && m_tps) {
        pd->setMaximum(m_tps->progressMax());
        pd->setValue(m_tps->progressValue());
        if (!m_tps->progressMax() && !m_tps->progressValue())
            pd->canceled();
    }
}
