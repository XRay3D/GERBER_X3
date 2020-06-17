#include "project.h"
#include "mainwindow.h"
#include "settings.h"

#include <QElapsedTimer>
#include <QFileDialog>
#include <filetree/filemodel.h>
#include <forms/gcodepropertiesform.h>

Project::Project()
{
    if (App::mInstance->m_project) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class Project more than 2 times!!!");
        exit(1);
    }
    App::mInstance->m_project = this;
}

Project::~Project() { App::mInstance->m_project = nullptr; }

bool Project::save(QFile& file)
{
    try {
        QDataStream out(&file);
        out << (m_ver = G2G_Ver_4);
        switch (m_ver) {
        case G2G_Ver_4:
        case G2G_Ver_3:
            out << m_spacingX;
            out << m_spacingY;
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
        in >> m_ver;
        qDebug() << "Project ver:" << m_ver;

        QPointF tmpPt;

        switch (m_ver) {
        case G2G_Ver_4:
        case G2G_Ver_3:
            in >> m_spacingX;
            in >> m_spacingY;
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
                App::fileModel()->addFile(static_cast<Gerber::File*>(filePtr.data()));
                break;
            case FileType::Excellon:
                App::fileModel()->addFile(static_cast<Excellon::File*>(filePtr.data()));
                break;
            case FileType::GCode:
                App::fileModel()->addFile(static_cast<GCode::File*>(filePtr.data()));
                break;
            }
        }
        m_isModified = false;
        qDebug() << "Project::open" << t.elapsed();
        App::layoutFrames()->updateRect();
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
    setSpaceX(0.0);
    setSpaceY(0.0);
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
        if (sp.data() && (sp.data()->type() == FileType::Gerber || sp.data()->type() == FileType::Excellon))
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
    IntPoint topLeft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint botRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (auto itemGroup = filePtr->itemGroup(); itemGroup->isVisible()) {
            for (const GraphicsItem* const item : *itemGroup) {
                if (item->isSelected()) {
                    for (const Path& path : item->paths()) {
                        for (const IntPoint& pt : path) {
                            topLeft.X = qMin(pt.X, topLeft.X);
                            topLeft.Y = qMin(pt.Y, topLeft.Y);
                            botRight.X = qMax(pt.X, botRight.X);
                            botRight.Y = qMax(pt.Y, botRight.Y);
                        }
                    }
                }
            }
        }
    }

    const QRectF rect(toQPointF(topLeft), toQPointF(botRight));

    if (!rect.isEmpty())
        setWorckRect(rect);

    return rect;
}

QRectF Project::getBoundingRect()
{
    QMutexLocker locker(&m_mutex);
    IntPoint topLeft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    IntPoint botRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (auto itemGroup = filePtr->itemGroup(); itemGroup->isVisible()) {
            for (const GraphicsItem* const item : *itemGroup) {
                for (const Path& path : item->paths()) {
                    for (const IntPoint& pt : path) {
                        topLeft.X = qMin(pt.X, topLeft.X);
                        topLeft.Y = qMin(pt.Y, topLeft.Y);
                        botRight.X = qMax(pt.X, botRight.X);
                        botRight.Y = qMax(pt.Y, botRight.Y);
                    }
                }
            }
        }
    }
    return QRectF(toQPointF(topLeft), toQPointF(botRight));
}

QString Project::fileNames()
{
    QMutexLocker locker(&m_mutex);
    QString fileNames;
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item && (item->type() == FileType::Gerber || item->type() == FileType::Excellon))
            fileNames.append(item->name()).append('|');
    }
    return fileNames;
}

int Project::contains(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    for (const QSharedPointer<AbstractFile>& sp : m_files) {
        AbstractFile* item = sp.data();
        if (item->type() == FileType::Gerber || item->type() == FileType::Excellon)
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
        case FileType::Gerber: {
            Gerber::File* f = static_cast<Gerber::File*>(file);
            file->setColor(m_files[id]->color());
            // Normal
            f->itemGroup(Gerber::File::Normal)->setBrush(static_cast<Gerber::File*>(m_files[id].data())->itemGroup(Gerber::File::Normal)->brush());
            f->itemGroup(Gerber::File::Normal)->addToScene();
            f->itemGroup(Gerber::File::Normal)->setZValue(-id);
            // ApPaths
            f->itemGroup(Gerber::File::ApPaths)->setPen(static_cast<Gerber::File*>(m_files[id].data())->itemGroup(Gerber::File::ApPaths)->pen());
            f->itemGroup(Gerber::File::ApPaths)->addToScene();
            f->itemGroup(Gerber::File::ApPaths)->setZValue(-id);
            // Components
            f->itemGroup(Gerber::File::Components)->addToScene();
            f->itemGroup(Gerber::File::Components)->setZValue(-id);
        } break;
        case FileType::Excellon:
            static_cast<Excellon::File*>(file)->setFormat(static_cast<Excellon::File*>(m_files[id].data())->format());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            break;
        case FileType::GCode:
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
        App::fileModel()->addFile(file);
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

    using Key = QPair<uint, Side>;

    QMap<Key, QList<GCode::File*>> mm;
    for (GCode::File* file : files)
        mm[{ file->getTool().hash(), file->side() }].append(file);

    for (const Key& key : mm.keys()) {
        QList<GCode::File*> files(mm.value(key));
        if (files.size() < 2) {
            for (GCode::File* file : files) {
                QString name(GCode::GCUtils::getLastDir().append(file->shortName()));
                if (!name.endsWith("tap"))
                    name += QStringList({ "_TS", "_BS" })[file->side()];
                name = QFileDialog::getSaveFileName(nullptr, tr("Save GCode file"), name, tr("GCode (*.%1)").arg(GlobalSettings::gcFileExtension()));
                if (name.isEmpty())
                    return;
                file->save(name);
                file->itemGroup()->setVisible(false);
            }
        } else {
            QString name(GCode::GCUtils::getLastDir().append(files.first()->getTool().nameEnc()));
            if (!name.endsWith("tap"))
                name += QStringList({ "_TS", "_BS" })[files.first()->side()];
            name = QFileDialog::getSaveFileName(nullptr, tr("Save GCode file"), name, tr("GCode (*.%1)").arg(GlobalSettings::gcFileExtension()));
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
                file->genGcodeAndTile();
                if (i == (files.size() - 1))
                    file->endFile();
                sl.append(file->gCodeText());
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
    App::layoutFrames()->updateRect();
}

double Project::spaceX() const { return m_spacingX; }

void Project::setSpaceX(double value)
{
    m_spacingX = value;
    App::layoutFrames()->updateRect();
}

double Project::spaceY() const { return m_spacingY; }

void Project::setSpaceY(double value)
{
    m_spacingY = value;
    App::layoutFrames()->updateRect();
}

int Project::stepsX() const { return m_stepsX; }

void Project::setStepsX(int value)
{
    m_stepsX = value;
    App::layoutFrames()->updateRect();
}

int Project::stepsY() const { return m_stepsY; }

void Project::setStepsY(int value)
{
    m_stepsY = value;
    App::layoutFrames()->updateRect();
}

QRectF Project::worckRect() const { return m_worckRect; }

void Project::setWorckRect(const QRectF& worckRect)
{
    m_worckRect = worckRect;
    App::layoutFrames()->updateRect();
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
    case static_cast<int>(FileType::Excellon):
        file = QSharedPointer<AbstractFile>(new Excellon::File(stream));
        break;
    case static_cast<int>(FileType::GCode):
        file = QSharedPointer<AbstractFile>(new GCode::File(stream));
        break;
    }
    return stream;
}
