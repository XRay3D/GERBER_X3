/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gcode.h"

#include <QThread>
#include <QWidget>

namespace GCode {
class File;
class Creator;
} // namespace GCode

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
    explicit FormsUtil(GCodePlugin* plugin, GCode::Creator* tps, QWidget* parent = nullptr);
    ~FormsUtil() override;
    virtual void editFile(GCode::File* file) = 0;

signals:
    void createToolpath();

protected:
    void fileHandler(GCode::File* file);

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;

    GCode::Creator* const tpc_;
    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;
    UsedItems usedItems_;
    Side boardSide = Top;
    void addUsedGi(GraphicsItem* gi);

    QString fileName_;

    bool m_editMode = false;
    int fileId = -1;

    int fileCount;
    GCodePlugin* const plugin;

private:
    void cancel();
    void errorHandler(int = 0);

    void startProgress();
    void stopProgress();

    QThread thread;
    GCode::File* file_;
    QProgressDialog* progressDialog;
    int progressTimerId = 0;
    int flikerTimerId = 0;
};
