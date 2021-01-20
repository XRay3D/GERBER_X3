/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "mvector.h"

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRectF>
#include <memory>

#include "leakdetector.h"

enum FileVersion {
    ProVer_1 = 1,
    ProVer_2,
    ProVer_3,
    ProVer_4,
    ProVer_5,
};

namespace Shapes {
class Shape;
}

class FileInterface;
class QFile;
enum class FileType;

#if _MSVC_LANG >= 201705L
using FilesMap = std::map<int, std::shared_ptr<FileInterface>>;
using ShapesMap = std::map<int, std::shared_ptr<Shapes::Shape>>;
#else
struct FilesMap : std::map<int, std::shared_ptr<FileInterface>> {
    bool contains(int key) const { return find(key) != end(); }
};
struct ShapesMap : std::map<int, std::shared_ptr<Shapes::Shape>> {
    bool contains(int key) const { return find(key) != end(); }
};
#endif
class Project : public QObject {
    Q_OBJECT

public:
    explicit Project(QObject* parent = nullptr);
    ~Project();

    // FileInterface
    template <typename T = FileInterface>
    T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        if (m_files.contains(id))
            return static_cast<T*>(m_files[id].get());
        return nullptr;
    }

    template <typename T = FileInterface>
    mvector<T*> files()
    {
        QMutexLocker locker(&m_mutex);
        mvector<T*> rfiles;
        for (const auto& [id, sp] : m_files) {
            T* file = dynamic_cast<T*>(sp.get());
            if (file)
                rfiles.push_back(file);
        }
        return rfiles;
    }

    template <typename T>
    mvector<T*> count()
    {
        QMutexLocker locker(&m_mutex);
        int count = 0;
        for (const auto& [id, sp] : m_files) {
            if (dynamic_cast<T*>(sp.get()))
                ++count;
        }
        return count;
    }

    int addFile(FileInterface* const file);
    bool contains(FileInterface* file);
    mvector<FileInterface*> files(FileType type);
    mvector<FileInterface*> files(const mvector<FileType> types);
    void deleteFile(int id);
    QString fileNames();
    int contains(const QString& name);

    // Shape
    int addShape(Shapes::Shape* const shape);
    Shapes::Shape* shape(int id);
    void deleteShape(int id);

    // Project
    bool save(const QString& fileName);
    bool open(const QString& fileName);
    void close();

    int ver() const;

    int size();

    bool isModified();
    void setModified(bool fl);

    QString name();
    void setName(const QString& name);
    void setChanged();
    bool pinsPlacedMessage();

    bool isUntitled();
    bool isPinsPlaced() const;
    void setUntitled(bool value);

    // Bounding Rect
    QRectF getBoundingRect();

    double spaceX() const;
    void setSpaceX(double value);

    double spaceY() const;
    void setSpaceY(double value);

    uint stepsX() const;
    void setStepsX(uint value);

    uint stepsY() const;
    void setStepsY(uint value);

    double safeZ() const;
    void setSafeZ(double safeZ);

    double boardThickness() const;
    void setBoardThickness(double boardThickness);

    double copperThickness() const;
    void setCopperThickness(double copperThickness);

    double clearence() const;
    void setClearence(double clearence);

    double plunge() const;
    void setPlunge(double plunge);

    double glue() const;
    void setGlue(double glue);

    QRectF worckRect() const;
    void setWorckRect(const QRectF& worckRect);

    QPointF homePos() const;
    void setHomePos(const QPointF& pos);

    QPointF zeroPos() const;
    void setZeroPos(const QPointF& pos);

    const QPointF* pinsPos() const;
    void setPinsPos(const QPointF pos[4]);

signals:
    void changed();
    void homePosChanged(const QPointF&);
    void zeroPosChanged(const QPointF&);
    void pinsPosChanged(QPointF*);
    void worckRectChanged(const QRectF&);
    void layoutFrameUpdate(bool = false);
    // need for debuging
    void addFileDbg(FileInterface* file);

private:
    bool reload(int id, FileInterface* file);

    int m_ver;

    FilesMap m_files;
    ShapesMap m_shapes;
    QMutex m_mutex;
    QString m_fileName;
    QString m_name;
    bool m_isModified = false;
    bool m_isUntitled = true;
    bool m_isPinsPlaced = false;

    QPointF m_home;
    QPointF m_zero;
    QPointF m_pins[4];
    QRectF m_worckRect;

    double m_safeZ;
    double m_boardThickness;
    double m_copperThickness;
    double m_clearence;
    double m_plunge;
    double m_glue;

    double m_spacingX = 0.0;
    double m_spacingY = 0.0;
    uint m_stepsX = 1;
    uint m_stepsY = 1;
};
