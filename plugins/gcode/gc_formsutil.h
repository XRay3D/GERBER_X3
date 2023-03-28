/********************************************************************************4
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "depthform.h"
#include "gcode.h"

#include <QButtonGroup>
#include <QThread>
#include <QtWidgets>

class GCode::File;
// namespace GCode {
// class File;
// class file_;
// } // namespace GCode

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
    // GcFormBase interface
    virtual void сomputePaths() = 0;
    virtual void updateName() = 0;

    GCode::Creator* const gcCreator;
    GCode::Direction direction = GCode::Climb;
    GCode::SideOfMilling side = GCode::Outer;
    GCode::UsedItems usedItems_;
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
    QDialogButtonBox* errBtnBox;
    class TableView* errTable;

    QWidget* ctrWidget;
    QWidget* errWidget;

    void errContinue();
    void errBreak();

    void cancel();
    void errorHandler(int = 0);

    void startProgress();
    void stopProgress();

    QThread thread;
    GCode::File* file_;
    QProgressDialog* progressDialog;
    int progressTimerId = 0;
};
