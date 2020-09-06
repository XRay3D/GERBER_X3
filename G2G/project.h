#pragma once

#include <QMap>
#include <QMutex>
#include <abstractfile.h>

enum FileVersion {
    G2G_Ver_1 = 1,
    G2G_Ver_2,
    G2G_Ver_3,
    G2G_Ver_4,
};

namespace Shapes {
class Shape;
}

class QFile;

class Project : public QObject {
    Q_OBJECT

public:
    explicit Project();
    ~Project() override;

    bool save(QFile& file);
    bool open(QFile& file);
    void close();

    AbstractFile* file(int id);
    void deleteFile(int id);
    void deleteShape(int id);
    bool isEmpty();
    int size();

    bool isModified() { return m_isModified; }
    void setModified(bool fl) { m_isModified = fl; }

    QRectF getSelectedBoundingRect();
    QRectF getBoundingRect();
    QString fileNames();
    int contains(const QString& name);
    bool reload(int id, AbstractFile* file);

    template <typename T>
    T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        return static_cast<T*>(m_files.value(id).data());
    }

    AbstractFile* aFile(int id);
    Shapes::Shape* aShape(int id);

    int addFile(AbstractFile* file);
    int addShape(Shapes::Shape* sh);

    template <typename T>
    bool replaceFile(int id, T* file)
    {
        QMutexLocker locker(&m_mutex);
        if (m_files.contains(id)) {
            m_files[id] = QSharedPointer<AbstractFile>(file);
            return true;
        }
        return false;
    }

    template <typename T>
    QVector<T*> files()
    {
        QMutexLocker locker(&m_mutex);
        QVector<T*> rfiles;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            T* file = dynamic_cast<T*>(sp.data());
            if (file)
                rfiles.append(file);
        }
        return rfiles;
    }

    template <typename T>
    QVector<T*> count()
    {
        QMutexLocker locker(&m_mutex);
        int count = 0;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            if (dynamic_cast<T*>(sp.data()))
                ++count;
        }
        return count;
    }

    void showFiles(const QList<QPair<int, int>>&& fileIds);

    bool contains(AbstractFile* file);
    void setIsModified(bool isModified) { m_isModified = isModified; }
    QString name() { return m_name; }
    void setName(const QString& name);
    void setChanged()
    {
        m_isModified = true;
        changed();
    }

    void saveSelectedToolpaths();
    bool isUntitled();
    void setUntitled(bool value);

    double spaceX() const;
    void setSpaceX(double value);

    double spaceY() const;
    void setSpaceY(double value);

    int stepsX() const;
    void setStepsX(int value);

    int stepsY() const;
    void setStepsY(int value);

    QRectF worckRect() const;
    void setWorckRect(const QRectF& worckRect);
    int ver() const { return m_ver; }

signals:
    void changed();

private:
    int m_ver;
    QMap<int, QSharedPointer<AbstractFile>> m_files;
    QMap<int, QSharedPointer<Shapes::Shape>> m_shapes;
    QMutex m_mutex;
    QString m_fileName;
    QString m_name;
    bool m_isModified = false;
    bool m_isUntitled = true;

    double m_spacingX = 0.0;
    double m_spacingY = 0.0;
    int m_stepsX = 1;
    int m_stepsY = 1;

    QRectF m_worckRect;
};
#include "app.h"
