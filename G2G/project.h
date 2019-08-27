#ifndef PROJECT_H
#define PROJECT_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSemaphore>
#include <exfile.h>
#include <gbrfile.h>
#include <gcfile.h>
#include <myclipper.h>

using namespace ClipperLib;

enum FileVersion {
    G2G_Ver_1 = 1,
    G2G_Ver_1_1,
};

class Project : public QObject {
    Q_OBJECT

public:
    explicit Project();
    ~Project() override;

    static bool save(QFile& file);
    static bool open(QFile& file);

    static AbstractFile* file(int id);
    static void deleteFile(int id);
    static bool isEmpty();
    static int size();

    //    static Paths getPaths()
    //    {
    //        QMutexLocker locker(&m_mutex);
    //        Paths paths;
    //        for (const QSharedPointer<AbstractFile>& sp : m_files) {
    //            AbstractFile* file = sp.data();
    //            if (file
    //                    && file->itemGroup()->isVisible()
    //                    && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
    //                for (Paths& p : file->groupedPaths())
    //                    paths.append(p);
    //        }
    //        return paths;
    //    }
    //    static Paths getSelectedPaths()
    //    {
    //        QMutexLocker locker(&m_mutex);
    //        Paths paths;
    //        for (const QSharedPointer<AbstractFile>& sp : m_files) {
    //            AbstractFile* file = sp.data();
    //            if (file
    //                    && file->itemGroup()->isVisible()
    //                    && (file->type() == FileType::Gerber || file->type() == FileType::Drill))
    //                for (GraphicsItem* item : *file->itemGroup())
    //                    if (item->isSelected())
    //                        paths.append(item->paths());
    //        }
    //        return paths;
    //    }

    static bool isModified() { return m_isModified; }
    static void setModified(bool fl) { m_isModified = fl; }

    static QRectF getSelectedBoundingRect();
    static QRectF getBoundingRect();
    static QString fileNames();
    static int contains(const QString& name);
    static bool reload(int id, AbstractFile* file);

    template <typename T>
    static T* file(int id)
    {
        QMutexLocker locker(&m_mutex);
        return static_cast<T*>(m_files.value(id).data());
    }

    static int addFile(AbstractFile* file);

    template <typename T>
    static bool replaceFile(int id, T* file)
    {
        QMutexLocker locker(&m_mutex);
        if (m_files.contains(id)) {
            m_files[id] = QSharedPointer<AbstractFile>(file);
            return true;
        }
        return false;
    }

    template <typename T>
    static QVector<T*> files()
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
    static QVector<T*> count()
    {
        QMutexLocker locker(&m_mutex);
        int count;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            if (dynamic_cast<T*>(sp.data()))
                ++count;
        }
        return count;
    }

    static bool contains(AbstractFile* file);
    static void setIsModified(bool isModified) { m_isModified = isModified; }
    static QString name() { return m_name; }
    void setName(const QString& name) { m_name = name; }
    static int ver();
    static void setChanged()
    {
        m_isModified = true;
        self->changed();
    }
signals:
    void changed();

private:
    static QString m_name;
    static int m_ver;

    static bool m_isModified;
    static QMutex m_mutex;
    static QMap<int, QSharedPointer<AbstractFile>> m_files;
    static QString m_fileName;
    static Project* self;
    static QSemaphore sem;
};

#endif // PROJECT_H
