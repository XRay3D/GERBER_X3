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
    G2G_Ver_2,
    G2G_Ver_3,
    // G2G_Ver_4,
};

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

    int addFile(AbstractFile* file);

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
        int count;
        for (const QSharedPointer<AbstractFile>& sp : m_files) {
            if (dynamic_cast<T*>(sp.data()))
                ++count;
        }
        return count;
    }

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

    static Project* instance() { return m_instance; }

    double spasingX() const;
    void setSpasingX(double value);

    double spasingY() const;
    void setSpasingY(double value);

    int stepsX() const;
    void setStepsX(int value);

    int stepsY() const;
    void setStepsY(int value);

    QRectF worckRect() const;
    void setWorckRect(const QRectF& worckRect);

signals:
    void changed();

private:
    static Project* m_instance;
    QMap<int, QSharedPointer<AbstractFile>> m_files;
    QMutex m_mutex;
    QSemaphore sem;
    QString m_fileName;
    QString m_name;
    bool m_isModified = false;
    bool m_isUntitled = true;

    double m_spasingX = 0.0;
    double m_spasingY = 0.0;
    int m_stepsX = 1;
    int m_stepsY = 1;

    QRectF m_worckRect;
};

#endif // PROJECT_H
