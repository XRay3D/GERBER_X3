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
#include "project.h"
#include "abstractfile.h"
#include "dxf_file.h"
#include "excellon.h"
#include "gbrfile.h"
#include "gcode.h"
#include "mainwindow.h"
#include "point.h"
#include "settings.h"
#include "shheaders.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <algorithm>
//#include <execution>
#include <filetree/filemodel.h>
#include <forms/gcodepropertiesform.h>
#include <gi/aperturepathitem.h>

#include "leakdetector.h"

Project::Project(QObject* parent)
    : QObject(parent)
{
    if (App::m_project) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class Project more than 2 times!!!");
        exit(1);
    }
    App::m_project = this;
}

Project::~Project() { App::m_project = nullptr; }

bool Project::save(QFile& file)
{
    try {
        QDataStream out(&file);
        out << (m_ver = ProVer_5);
        switch (m_ver) {
        case ProVer_5:
            out << m_isPinsPlaced;
            [[fallthrough]];
        case ProVer_4:
            [[fallthrough]];
        case ProVer_3:
            out << m_spacingX;
            out << m_spacingY;
            out << m_stepsX;
            out << m_stepsY;
            [[fallthrough]];
        case ProVer_2:
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
        case ProVer_1:;
        }
        out << m_files;
        out << m_shapes;
        m_isModified = false;
        return true;
    } catch (...) {
    }
    return false;
}

bool Project::open(QFile& file)
{
    try {
        qDebug() << __FUNCTION__ << file;
        QElapsedTimer t;
        t.start();
        QDataStream in(&file);
        in >> m_ver;

        QPointF tmpPt;

        switch (m_ver) {
        case ProVer_5:
            in >> m_isPinsPlaced;
            [[fallthrough]];
        case ProVer_4:
            [[fallthrough]];
        case ProVer_3:
            in >> m_spacingX;
            in >> m_spacingY;
            in >> m_stepsX;
            in >> m_stepsY;
            [[fallthrough]];
        case ProVer_2:
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
        case ProVer_1:;
        }
        in >> m_files;
        for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
            App::fileModel()->addFile(filePtr.data());
        }
        in >> m_shapes;
        for (const QSharedPointer<Shapes::Shape>& shPtr : m_shapes) {
            App::fileModel()->addShape(shPtr.data());
        }

        m_isModified = false;

        App::layoutFrames()->updateRect();
        return true;
    } catch (...) {
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
    m_isPinsPlaced = false;
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
        m_files.remove(id);
        setChanged();
    } else
        qWarning() << "Error id" << id << "File not found";
}

void Project::deleteShape(int id)
{
    QMutexLocker locker(&m_mutex);
    if (m_shapes.contains(id)) {
        m_shapes.take(id);
        setChanged();
    } else
        qWarning() << "Error id" << id << "Shape not found";
}

int Project::size() { return m_files.size() + m_shapes.size(); }

QRectF Project::getSelectedBoundingRect()
{
    QMutexLocker locker(&m_mutex);

    auto selectedItems(App::scene()->selectedItems());

    if (selectedItems.isEmpty())
        return {};

    auto getRect = [](QGraphicsItem* gi) -> QRectF {
        //        return static_cast<GiType>(gi->type()) == GiType::AperturePath
        //            ? static_cast<AperturePathItem*>(gi)->boundingRect2()
        //            : gi->boundingRect();
        switch (static_cast<GiType>(gi->type())) {
        case GiType::Gerber:
            return gi->boundingRect();
        case GiType::Drill:
            return gi->boundingRect();
        case GiType::AperturePath:
            return static_cast<AperturePathItem*>(gi)->boundingRect2();
        default:
            return {};
        }
    };

    QRectF rect(getRect(selectedItems.takeFirst()));
    for (auto gi : selectedItems)
        rect = rect.united(getRect(gi));

    if (!rect.isEmpty())
        setWorckRect(rect);

    return rect;
}

QRectF Project::getBoundingRect()
{
    QMutexLocker locker(&m_mutex);
    Point64 topLeft(std::numeric_limits<cInt>::max(), std::numeric_limits<cInt>::max());
    Point64 botRight(std::numeric_limits<cInt>::min(), std::numeric_limits<cInt>::min());
    for (const QSharedPointer<AbstractFile>& filePtr : m_files) {
        if (auto itemGroup = filePtr->itemGroup(); itemGroup->isVisible()) {
            for (const GraphicsItem* const item : *itemGroup) {
                for (const Path& path : item->paths()) {
                    for (const Point64& pt : path) {
                        topLeft.X = qMin(pt.X, topLeft.X);
                        topLeft.Y = qMin(pt.Y, topLeft.Y);
                        botRight.X = qMax(pt.X, botRight.X);
                        botRight.Y = qMax(pt.Y, botRight.Y);
                    }
                }
            }
        }
    }
    return QRectF(topLeft, botRight);
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
        if (item->type() == FileType::Gerber
            || item->type() == FileType::Excellon
            || item->type() == FileType::Dxf)
            if (QFileInfo(item->name()).fileName() == QFileInfo(name).fileName())
                return item->id();
    }
    return -1;
}

bool Project::reload(int id, AbstractFile* file)
{
    file->setId(id);
    if (m_files.contains(id)) {
        switch (file->type()) {
        case FileType::Gerber: {
            auto gbrFile = static_cast<Gerber::File*>(file);
            file->setColor(m_files[id]->color());
            // Normal
            gbrFile->itemGroup(Gerber::File::Normal)->setBrushColor(static_cast<Gerber::File*>(m_files[id].data())->itemGroup(Gerber::File::Normal)->brushColor());
            gbrFile->itemGroup(Gerber::File::Normal)->addToScene();
            gbrFile->itemGroup(Gerber::File::Normal)->setZValue(-id);
            // ApPaths
            gbrFile->itemGroup(Gerber::File::ApPaths)->setPen(static_cast<Gerber::File*>(m_files[id].data())->itemGroup(Gerber::File::ApPaths)->pen());
            gbrFile->itemGroup(Gerber::File::ApPaths)->addToScene();
            gbrFile->itemGroup(Gerber::File::ApPaths)->setZValue(-id);
            // Components
            gbrFile->itemGroup(Gerber::File::Components)->addToScene();
            gbrFile->itemGroup(Gerber::File::Components)->setZValue(-id);
            m_files[id] = QSharedPointer<AbstractFile>(file);
        } break;
        case FileType::Excellon: {
            auto excFile = static_cast<Excellon::File*>(file);
            excFile->setFormat(static_cast<Excellon::File*>(m_files[id].data())->format());
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            m_files[id] = QSharedPointer<AbstractFile>(file);

        } break;
        case FileType::GCode:
            file->itemGroup()->addToScene();
            file->itemGroup()->setZValue(-id);
            m_files[id] = QSharedPointer<AbstractFile>(file);
            break;
        case FileType::Dxf: {
            file->setFileIndex(m_files[id]->fileIndex());
            auto dxfFile = static_cast<Dxf::File*>(file);
            for (auto ig : dxfFile->itemGroups())
                ig->addToScene();
            m_files[id] = QSharedPointer<AbstractFile>(file);
            App::fileModel()->updateFile(file->fileIndex());
        } break;
        }
        return true;
    }
    return false;
}

AbstractFile* Project::aFile(int id)
{
    QMutexLocker locker(&m_mutex);
    return m_files.value(id).data();
}

Shapes::Shape* Project::aShape(int id)
{
    QMutexLocker locker(&m_mutex);
    return m_shapes.value(id).data();
}

int Project::addFile(AbstractFile* file)
{
    m_isPinsPlaced = false; //QMutexLocker locker(&m_mutex);
    file->createGi();
    file->setVisible(true);
    const int id = contains(file->name());
    if (id != -1) {
        reload(id, file);
    } else if (file->id() == -1) {
        file->setId(m_files.size() ? m_files.lastKey() + 1 : 0);
        m_files.insert(file->id(), QSharedPointer<AbstractFile>(file));
        App::fileModel()->addFile(file);
    }
    setChanged();
    return file->id();
}

int Project::addShape(Shapes::Shape* sh)
{
    m_isPinsPlaced = false;
    //QMutexLocker locker(&m_mutex);
    sh->m_id = m_shapes.size() ? m_shapes.lastKey() + 1 : 0;
    sh->setToolTip(QString::number(sh->m_id));
    m_shapes.insert(sh->m_id, QSharedPointer<Shapes::Shape>(sh));
    App::fileModel()->addShape(sh);
    setChanged();
    return sh->m_id;
}

void Project::showFiles(const QList<QPair<int, int>>&& fileIds)
{
    for (auto file : m_files)
        file->itemGroup()->setVisible(false);
    for (auto [fileId, giType] : fileIds) {
        if (giType > -1 && m_files[fileId]->type() == FileType::Gerber)
            file<Gerber::File>(fileId)->setItemType(static_cast<Gerber::File::ItemsType>(giType));
        m_files[fileId]->itemGroup()->setVisible(true);
    }
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

bool Project::pinsPlacedMessage()
{

    if (m_isPinsPlaced == false) {
        QMessageBox msgbx(QMessageBox::Information,
            "",
            tr("Board dimensions may have changed.\n"
               "It is advisable to perform automatic placement of the pins\n"
               "by selecting the necessary work items.\n\n"
               "Continue saving?"),
            QMessageBox::Yes | QMessageBox::No, nullptr);
        {
            auto label(msgbx.findChild<QLabel*>());
            label->setPixmap(QIcon::fromTheme("snap-nodes-cusp").pixmap(label->size()));
        }
        return msgbx.exec() == QMessageBox::No;
    }
    return false;
    /* Размеры платы могли измениться.
     * Выполните автоматическое размещение штифтов, выбрав необходимые рабочие элементы.
     * Продолжить сохранение?
     */
}

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2)
{
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

void Project::saveSelectedToolpaths()
{
    if (pinsPlacedMessage())
        return;

    QVector<GCode::File*> gcFiles(this->files<GCode::File>());
    for (int i = 0; i < gcFiles.size(); ++i) {
        if (!gcFiles[i]->itemGroup()->isVisible())
            gcFiles.remove(i--);
    }

    using Key = QPair<uint, Side>;

    QMap<Key, QList<GCode::File*>> mm;
    for (GCode::File* file : gcFiles)
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
    App::layoutFrames()->updateRect(true);
}

double Project::spaceY() const { return m_spacingY; }

void Project::setSpaceY(double value)
{
    m_spacingY = value;
    App::layoutFrames()->updateRect(true);
}

int Project::stepsX() const { return m_stepsX; }

void Project::setStepsX(int value)
{
    m_stepsX = value;
    App::layoutFrames()->updateRect(true);
}

int Project::stepsY() const { return m_stepsY; }

void Project::setStepsY(int value)
{
    m_stepsY = value;
    App::layoutFrames()->updateRect(true);
}

QRectF Project::worckRect() const { return m_worckRect; }

void Project::setWorckRect(const QRectF& worckRect)
{
    m_worckRect = worckRect;
    m_isPinsPlaced = true;
    App::layoutFrames()->updateRect();
}

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractFile>& file)
{
    stream << *file;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<AbstractFile>& file)
{
    int type;
    stream >> type;
    switch (static_cast<FileType>(type)) {
    case FileType::Gerber:
        file = QSharedPointer<AbstractFile>(new Gerber::File());
        break;
    case FileType::Excellon:
        file = QSharedPointer<AbstractFile>(new Excellon::File());
        break;
    case FileType::GCode:
        file = QSharedPointer<AbstractFile>(new GCode::File());
        break;
    case FileType::Dxf:
        file = QSharedPointer<AbstractFile>(new Dxf::File());
        break;
    }
    stream >> *file;
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const QSharedPointer<Shapes::Shape>& sh)
{
    stream << *sh;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, QSharedPointer<Shapes::Shape>& sh)
{
    int type;
    stream >> type;
    switch (static_cast<GiType>(type)) {
    case GiType::ShapeC:
        sh = QSharedPointer<Shapes::Shape>(new Shapes::Circle());
        break;
    case GiType::ShapeR:
        sh = QSharedPointer<Shapes::Shape>(new Shapes::Rectangle());
        break;
    case GiType::ShapeL:
        sh = QSharedPointer<Shapes::Shape>(new Shapes::PolyLine());
        break;
    case GiType::ShapeA:
        sh = QSharedPointer<Shapes::Shape>(new Shapes::Arc());
        break;
    case GiType::ShapeT:
        sh = QSharedPointer<Shapes::Shape>(new Shapes::Text());
        break;
    default:;
    }
    stream >> *sh;
    return stream;
}
