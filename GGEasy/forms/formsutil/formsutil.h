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
#pragma once
#include "gcode.h"
#include <QThread>
#include <QWidget>

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

    void fileHandler(GCode::File* file);

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
    void errorHandler(int = 0);

    void startProgress();
    void stopProgress();

    QThread thread;
    GCode::File* m_file;
    QProgressDialog* progressDialog;
    int progressTimerId = 0;
    int flikerTimerId = 0;
};
