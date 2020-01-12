#include "project.h"
#include "mainwindow.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <filetree/filemodel.h>
#include <forms/gcodepropertiesform.h>

Project* Project::m_instance = nullptr;

Project::Project() { m_instance = this; }

Project::~Project() { m_instance = nullptr; }

bool Project::save(QFile& file)
{
    try {
        QDataStream out(&file);
        out << G2G_Ver_3;
        switch (G2G_Ver_3) {
        case G2G_Ver_3:
            out << m_spasingX;
            out << m_spasingY;
            out << m_stepsX;
            out << m_stepsY;
            [[fallthrough]];
        case G2G_Ver_2:
            out << Marker::get(Marker::Home)->pos();
            out << Marker::get(Marker::Zero)->pos();
            for (Pin* pin : Pin::pins())
                out << pin->pos();
            out << m_worckRect;
            out << GCodePropertiesForm::safeZ;
            out << GCodePropertiesForm::boardThickness;
            out << GCodePropertiesForm::copperThickness;
            out << GCodePropertiesForm::clearence;
            out << GCodePropertiesForm::plunge;
            out << GCodePropertiesForm::glue;
            [[fallthrough]];
        case G2G_Ver_1:;
        }
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
        int ver;
        in >> ver;
        qDebug() << "Project ver:" << ver;

        QPointF tmpPt;

        switch (ver) {
        case G2G_Ver_3:
            in >> m_spasingX;
            in >> m_spasingY;
            in >> m_stepsX;
            in >> m_stepsY;
            [[fallthrough]];
        case G2G_Ver_2:
            in >> tmpPt;
            Marker::get(Marker::Home)->setPos(tmpPt);
            in >> tmpPt;
            Marker::get(Marker::Zero)->setPos(tmpPt);
            for (Pin* pin : Pin::pins()) {
                in >> tmpPt;
                pin->setPos(tmpPt);
            }
            in >> m_worckRect;
            in >> GCodePropertiesForm::safeZ;
            in >> GCodePropertiesForm::boardThickness;
            in >> GCodePropertiesForm::copperThickness;
            in >> GCodePropertiesForm::clearence;
            in >> GCodePropertiesForm::plunge;
            in >> GCodePropertiesForm::glue;
            [[fallthrough]];
        case G2G_Ver_1:;
        }
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
        LayoutFrames::updateRect();
        return true;
    } catch (...) {
        qDebug() << file.errorString();
    }
    return false;
}

void Project::close()
{
    setWorckRect({});
    setStepsX(1);
    setStepsY(1);
    setSpasingX(0.0);
    setSpasingY(0.0);
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

    const QRectF rect(toQPointF(topleft), toQPointF(bottomRight));

    if (!rect.isEmpty())
        setWorckRect(rect);

    return rect;
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
            static_cast<Excellon::File*>(file)->setFormat(static_cast<Excellon::File*>(m_files[id].data())->format());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            break;
        default:
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

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2)
{
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

void Project::saveSelectedToolpaths()
{
    QVector<GCode::File*> files(this->files<GCode::File>());
    for (int i = 0; i < files.size(); ++i) {
        if (!files[i]->itemGroup()->isVisible())
            files.remove(i--);
    }

    QMap<QPair<Tool, Side>, QList<GCode::File*>> mm;
    for (GCode::File* file : files)
        mm[QPair { file->getTool(), file->side() }].append(file);

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

    if (mm.isEmpty())
        QMessageBox::information(nullptr, "", tr("No selected toolpath files."));
}

bool Project::isUntitled() { return m_isUntitled; }

void Project::setUntitled(bool value)
{
    m_isUntitled = value;
    LayoutFrames::updateRect();
}

double Project::spasingX() const { return m_spasingX; }

void Project::setSpasingX(double value)
{
    m_spasingX = value;
    LayoutFrames::updateRect();
}

double Project::spasingY() const { return m_spasingY; }

void Project::setSpasingY(double value)
{
    m_spasingY = value;
    LayoutFrames::updateRect();
}

int Project::stepsX() const { return m_stepsX; }

void Project::setStepsX(int value)
{
    m_stepsX = value;
    LayoutFrames::updateRect();
}

int Project::stepsY() const { return m_stepsY; }

void Project::setStepsY(int value)
{
    m_stepsY = value;
    LayoutFrames::updateRect();
}

QRectF Project::worckRect() const { return m_worckRect; }

void Project::setWorckRect(const QRectF& worckRect)
{
    m_worckRect = worckRect;
    LayoutFrames::updateRect();
}

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
