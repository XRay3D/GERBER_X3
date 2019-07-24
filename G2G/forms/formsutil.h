#ifndef TOOLPATHUTIL_H
#define TOOLPATHUTIL_H

#include "gcode/gccreator.h"
#include "tooldatabase/tool.h"
#include <QThread>
#include <QVector>
#include <QWidget>

class QProgressDialog;

class FormsUtil : public QWidget {
    Q_OBJECT

public:
    FormsUtil(const QString& name, QWidget* parent = nullptr);
    ~FormsUtil();
    virtual void editFile(GCode::File* file) = 0;

protected:
    void readTools(const QVector<Tool*>& tools) const;
    void writeTools(const QVector<Tool*>& tools) const;
    virtual void create() = 0;
    virtual void updateName() = 0;

    void setFile(GCode::File* file);

    int fileCount;
    Tool tool;
    Tool tool2;
    Direction direction = Climb;
    SideOfMilling side = Outer;
    Side boardSide = Top;

    GCode::Creator* toolPathCreator(const Paths& value, const bool convent, SideOfMilling side);
    QString m_fileName;

    void showProgress();

    QMap<int, QVector<int>> m_used;
    bool m_editMode = false;

private:
    const QString m_name;
    QThread thread;
    GCode::File* m_file;
    void cancel();
    GCode::Creator* m_tps = nullptr;
    QProgressDialog* pd = nullptr;
    int m_timerId = 0;

    // QObject interface
protected:
    virtual void timerEvent(QTimerEvent* event) override;
};

#endif // TOOLPATHUTIL_H
