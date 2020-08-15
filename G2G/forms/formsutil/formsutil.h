#pragma once



#include "tooldatabase/tool.h"
#include <QThread>
#include <QVector>
#include <QWidget>
#include <abstractfile.h>
#include <gccreator.h>
#include <gctypes.h>

namespace GCode {
class File;
}

class GraphicsItem;
class QProgressDialog;

class FormsUtil : public QWidget {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit FormsUtil(GCode::Creator* tps, QWidget* parent = nullptr);
    ~FormsUtil() override;
    virtual void editFile(GCode::File* file) = 0;

signals:
    void createToolpath();

protected:
    virtual void createFile() = 0;
    virtual void updateName() = 0;

    void setFile(GCode::File* file);

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;

    GCode::Creator* m_tpc = nullptr;

    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;

    void addUsedGi(GraphicsItem* gi);

    UsedItems m_usedItems;

    QString m_fileName;
    Side boardSide = Top;

    bool m_editMode = false;
    int fileId = -1;

    int fileCount;

private:
    void cancel();

    QThread thread;
    GCode::File* m_file;
    QProgressDialog* pd;
    int m_timerId = 0;
};

//Q_DECLARE_METATYPE(QMap<int, QVector<int>>)


