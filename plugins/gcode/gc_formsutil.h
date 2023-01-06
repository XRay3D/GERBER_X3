/********************************************************************************4
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "depthform.h"
#include "gcode.h"

#include <QThread>
#include <QtWidgets>

namespace GCode {
class File;
class file_;

} // namespace GCode

class GraphicsItem;
class QProgressDialog;

class GcFormBase : public QWidget {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit GcFormBase(GCodePlugin* plugin, GCode::Creator* tpc, QWidget* parent = nullptr);
    ~GcFormBase() override;
    virtual void editFile(GCode::File* file) = 0;

signals:
    void createToolpath();

protected:
    void fileHandler(GCode::File* file);
    void updateButtonIconSize() {
        for (auto* button : findChildren<QPushButton*>())
            button->setIconSize({16, 16});
    }

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;
    virtual void createFile() = 0;
    virtual void updateName() = 0;

    GCode::Creator* const creator;
    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;
    UsedItems usedItems_;
    Side boardSide = Top;
    void addUsedGi(GraphicsItem* gi);

    QString fileName_;

    QString trOutside {tr("Outside")};
    QString trDepth {tr("Depth:")};
    QString trTool {tr("Tool:")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};
    //    QString trOutside {tr("Outside")};

    bool editMode_ = false;
    int fileId {-1};

    int fileCount {1};
    GCodePlugin* const plugin;

    DepthForm* dsbxDepth;
    //    QLabel* label;
    QLineEdit* leName;
    QPushButton* pbClose;
    QPushButton* pbCreate;
    QWidget* content;
    QGridLayout* grid;

private:
    void cancel();
    void errorHandler(int = 0);

    void startProgress();
    void stopProgress();

    QThread thread;
    GCode::File* file_;
    QProgressDialog* progressDialog;
    int progressTimerId = 0;
};
