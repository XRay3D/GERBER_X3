#include "project.h"
#include "mainwindow.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <filetree/filemodel.h>
#include <forms/gcodepropertiesform.h>

bool Project::m_isUntitled = true;
QMap<int, QSharedPointer<AbstractFile>> Project::m_files;
bool Project::m_isModified = false;
QMutex Project::m_mutex;
QString Project::m_fileName;
Project* Project::self = nullptr;
QSemaphore Project::sem;
QString Project::m_name;
int Project::m_ver = G2G_Ver_1_2;

Project::Project() { self = this; }

Project::~Project() { self = nullptr; }

bool Project::save(QFile& file)
{
    try {
        QDataStream out(&file);
        out << G2G_Ver_1_1;
        out << GCodePropertiesForm::homePoint->pos();
        out << GCodePropertiesForm::zeroPoint->pos();
        out << Pin::pins()[0]->pos();
        out << Pin::pins()[1]->pos();
        out << Pin::pins()[2]->pos();
        out << Pin::pins()[3]->pos();
        out << Pin::worckRect;
        out << GCodePropertiesForm::safeZ;
        out << GCodePropertiesForm::thickness;
        out << GCodePropertiesForm::copperThickness;
        out << GCodePropertiesForm::clearence;
        out << GCodePropertiesForm::plunge;
        out << GCodePropertiesForm::glue;
        out << m_files;
        m_isModified = false;
        return true;
    } catch (...) {
        qDebug() << file.errorString();
    }
    return false;
}

bool Project::open(QFile& file)
{
    try {
        QElapsedTimer t;
        t.start();
        QDataStream in(&file);
        in >> m_ver;
        QPointF pt;
        in >> pt;
        GCodePropertiesForm::homePoint->setPos(pt);
        in >> pt;
        GCodePropertiesForm::zeroPoint->setPos(pt);
        in >> pt;
        Pin::pins()[0]->setPos(pt);
        in >> pt;
        Pin::pins()[1]->setPos(pt);
        in >> pt;
        Pin::pins()[2]->setPos(pt);
        in >> pt;
        Pin::pins()[3]->setPos(pt);
        in >> Pin::worckRect;
        in >> GCodePropertiesForm::safeZ;
        in >> GCodePropertiesForm::thickness;
        in >> GCodePropertiesForm::copperThickness;
        in >> GCodePropertiesForm::clearence;
        in >> GCodePropertiesForm::plunge;
        in >> GCodePropertiesForm::glue;
        in >> m_files;
        for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
            switch (filePtr->type()) {
            case FileType::Gerber:
                FileModel::addFile(static_cast<Gerber::File*>(filePtr.data()));
                break;
            case FileType::Drill:
                FileModel::addFile(static_cast<Excellon::File*>(filePtr.data()));
                break;
            case FileType::GCode:
                FileModel::addFile(static_cast<GCode::File*>(filePtr.data()));
                break;
            }
        }
        m_isModified = false;
        qDebug() << "Project::open" << t.elapsed();
        m_ver = G2G_Ver_1_1;
        return true;
    } catch (...) {
        qDebug() << file.errorString();
    }
    return false;
}

AbstractFile* Project::file(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id))
        return m_files.value(id).data();
    return nullptr;
}

void Project::deleteFile(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_files.contains(id)) {
        m_files.take(id);
        setChanged();
    } else
        qWarning() << "Error id" << id;
}

bool Project::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        if (sp.data() && (sp.data()->type() == FileType::Gerber || sp.data()->type() == FileType::Drill))
            return true;
    }
    return false;
}

int Project::size()
{
    return m_files.size();
}

QRectF Project::getSelectedBoundingRect()
{
    QMutexLocker locker(&m_mutex);
    IntPoint topleft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint bottomRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (filePtr->itemGroup()->isVisible()) {
            for (const GraphicsItem* const item : *filePtr->itemGroup()) {
                if (item->isSelected()) {
                    for (const Path& path : item->paths()) {
                        for (const IntPoint& pt : path) {
                            topleft.X = qMin(pt.X, topleft.X);
                            topleft.Y = qMin(pt.Y, topleft.Y);
                            bottomRight.X = qMax(pt.X, bottomRight.X);
                            bottomRight.Y = qMax(pt.Y, bottomRight.Y);
                        }
                    }
                }
            }
        }
    }
    return QRectF(toQPointF(topleft), toQPointF(bottomRight));
}

QRectF Project::getBoundingRect()
{
    QMutexLocker locker(&m_mutex);
    IntPoint topleft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint bottomRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (filePtr->itemGroup()->isVisible()) {
            for (const GraphicsItem* const item : *filePtr->itemGroup()) {
                for (const Path& path : item->paths()) {
                    for (const IntPoint& pt : path) {
                        topleft.X = qMin(pt.X, topleft.X);
                        topleft.Y = qMin(pt.Y, topleft.Y);
                        bottomRight.X = qMax(pt.X, bottomRight.X);
                        bottomRight.Y = qMax(pt.Y, bottomRight.Y);
                    }
                }
            }
        }
    }
    return QRectF(toQPointF(topleft), toQPointF(bottomRight));
}

QString Project::fileNames()
{
    QMutexLocker locker(&m_mutex);
    QString fileNames;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item && (item->type() == FileType::Gerber || item->type() == FileType::Drill))
            fileNames.append(item->name()).append('|');
    }
    return fileNames;
}

int Project::contains(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item->type() == FileType::Gerber || item->type() == FileType::Drill)
            if (QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
                return item->id();
    }
    return -1;
}

bool Project::reload(int id, AbstractFile* file)
{
    file->m_id = id;
    if (m_files.contains(id)) {
        switch (file->type()) {
        case FileType::Gerber:
            file->setColor(m_files[id]->color());
            file->itemGroup()->setBrush(m_files[id]->itemGroup()->brush());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            static_cast<Gerber::File*>(file)->rawItemGroup()->setPen(static_cast<Gerber::File*>(m_files[id].data())->rawItemGroup()->pen());
            static_cast<Gerber::File*>(file)->rawItemGroup()->addToScene();
            static_cast<Gerber::File*>(file)->rawItemGroup()->setZValue(-id);
            break;
        case FileType::Drill:
            //            file->setColor(m_files[id]->color());
            static_cast<Excellon::File*>(file)->setFormat(static_cast<Excellon::File*>(m_files[id].data())->format());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            break;
        default:
            //            file->setColor(m_files[id]->color());
            //            file->itemGroup()->setBrush(m_files[id]->itemGroup()->brush());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            break;
        }
        m_files[id] = QSharedPointer<AbstractFile>(file);
        return true;
    }
    return false;
}

int Project::addFile(AbstractFile* file)
{
    //QMutexLocker locker(&m_mutex);
    const int id = contains(file->name());
    if (id != -1) {
        reload(id, file);
    } else if (file->m_id == -1) {
        file->m_id = m_files.size() ? m_files.lastKey() + 1 : 0;
        m_files.insert(file->m_id, QSharedPointer<AbstractFile>(file));
        FileModel::addFile(file);
    }
    setChanged();
    return file->m_id;
}

bool Project::contains(AbstractFile* file) { return m_files.values().contains(QSharedPointer<AbstractFile>(file)); }

void Project::setName(const QString& name)
{
    setUntitled(name.isEmpty());
    if (m_isUntitled)
        m_name = tr("Untitled.g2g");
    else
        m_name = name;
}

int Project::ver() { return m_ver; }

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2)
{
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

void Project::saveSelectedToolpaths()
{
    QVector<GCode::File*> files(files<GCode::File>());
    for (int i = 0; i < files.size(); ++i) {
        if (!files[i]->itemGroup()->isVisible())
            files.remove(i--);
    }

    QMap<QPair<Tool, Side>, QList<GCode::File*>> mm;
    for (GCode::File* file : files)
        mm[QPair{ file->getTool(), file->side() }].append(file);

    for (const QPair<Tool, Side>& key : mm.keys()) {
        QList<GCode::File*> files(mm.value(key));
        if (files.size() < 2 /*|| key.first == -1*/) {
            for (GCode::File* file : files) {
                QString name(GCode::File::getLastDir().append(file->shortName()));
                if (!name.endsWith("tap"))
                    name += QStringList({ "(Top)", "(Bot)" })[file->side()];
                name = QFileDialog::getSaveFileName(nullptr, tr("Save GCode file"), name, tr("GCode (*.tap)"));
                if (name.isEmpty())
                    return;
                file->save(name);
                file->itemGroup()->setVisible(false);
            }
        } else {
            QString name(GCode::File::getLastDir().append(files.first()->getTool().name()));
            if (!name.endsWith("tap"))
                name += QStringList({ "(Top)", "(Bot)" })[files.first()->side()];
            name = QFileDialog::getSaveFileName(nullptr, tr("Save GCode file"), name, tr("GCode (*.tap)"));
            if (name.isEmpty())
                return;
            QList<QString> sl;
            for (int i = 0; i < files.size(); ++i) {
                GCode::File* file = files[i];
                file->itemGroup()->setVisible(false);
                file->initSave();
                if (i == 0)
                    file->statFile();
                file->addInfo(true);
                file->genGcode();
                if (i == files.size() - 1)
                    file->endFile();
                sl.append(file->getSl());
            }
            QFile file(name);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                QString str;
                for (QString& s : sl) {
                    if (!s.isEmpty())
                        str.append(s);
                    if (!str.endsWith('\n'))
                        str.append("\n");
                }
                out << str;
            }
            file.close();
        }
    }
    //    for (GCode::File* file : files) {
    //        if (!file->itemGroup()->isVisible())
    //            continue;
    //        isEmpty = false;
    //        QString name(GCode::File::getLastDir().append(file->shortName()));
    //        if (!name.endsWith("tap"))
    //            name += QStringList({ "(Top)", "(Bot)" })[file->side()];
    //        name = QFileDialog::getSaveFileName(nullptr, tr("Save GCode file"), name, tr("GCode (*.tap)"));
    //        if (name.isEmpty())
    //            return;
    //        file->save(name);
    //        file->itemGroup()->setVisible(false);
    //    }
    if (mm.isEmpty())
        QMessageBox::information(nullptr, "", tr("No selected toolpath files."));
}

bool Project::isUntitled() { return m_isUntitled; }

void Project::setUntitled(bool value) { m_isUntitled = value; }

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractFile>& file)
{
    stream << static_cast<int>(file->type());
    file->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractFile>& file)
{
    int type;
    stream >> type;
    switch (type) {
    case static_cast<int>(FileType::Gerber):
        file = QSharedPointer<AbstractFile>(new Gerber::File(stream));
        break;
    case static_cast<int>(FileType::Drill):
        file = QSharedPointer<AbstractFile>(new Excellon::File(stream));
        break;
    case static_cast<int>(FileType::GCode):
        file = QSharedPointer<AbstractFile>(new GCode::File(stream));
        break;
    }
    return stream;
}
