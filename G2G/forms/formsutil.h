#ifndef TOOLPATHUTIL_H
#define TOOLPATHUTIL_H

#include "tooldatabase/tool.h"
#include <QThread>
#include <QVector>
#include <QWidget>
#include <abstractfile.h>
#include <gccreator.h>
#include <gcvars.h>

namespace GCode {
class File;
}

class QProgressDialog;

class FormsUtil : public QWidget {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit FormsUtil(const QString& name, QWidget* parent = nullptr);
    ~FormsUtil() override;
    virtual void editFile(GCode::File* file) = 0;

signals:
    void createToolpath(const GCode::GCodeParams&);

protected:
    void readTools(const QVector<Tool*>& tools) const;
    void writeTools(const QVector<Tool*>& tools) const;
    virtual void createFile() = 0;
    virtual void updateName() = 0;

    void setFile(GCode::File* file);

    int fileCount;
    Tool tool;
    Tool tool2;
    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;
    Side boardSide = Top;

    QString m_fileName;

    void showProgress();
    void toolPathCreator(GCode::Creator* tps); /*const Paths& value, const bool convent, GCode::SideOfMilling side*/

    QMap<int, QVector<int>> m_used;
    bool m_editMode = false;
    GCode::Creator* m_tps = nullptr;

private:
    const QString m_name;
    QThread thread;
    GCode::File* m_file;
    void cancel();
    QProgressDialog* pd = nullptr;
    int m_timerId = 0;

    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent* event) override;
};

#endif // TOOLPATHUTIL_H
