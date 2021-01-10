/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "gcode/gcode.h"

#include <QThread>
#include <QWidget>

namespace GCode {
class File;
class Creator;
}

class GraphicsItem;
class QProgressDialog;

struct FormsUtilI {
    virtual void createFile() = 0;
    virtual void updateName() = 0;
};

class FormsUtil : public QWidget, protected FormsUtilI {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit FormsUtil(GCode::Creator* tps, QWidget* parent = nullptr);
    ~FormsUtil() override;
    virtual void editFile(GCode::File* file) = 0;

signals:
    void createToolpath();

protected:
    void fileHandler(GCode::File* file);

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;

    GCode::Creator* m_tpc = nullptr;
    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;
    UsedItems m_usedItems;
    Side boardSide = Top;
    void addUsedGi(GraphicsItem* gi);

    QString m_fileName;

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
