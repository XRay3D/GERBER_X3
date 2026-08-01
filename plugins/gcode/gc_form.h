/********************************************************************************4
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "cancelation.h"
#include "depthform.h"
#include "gcode.h"

#include <QButtonGroup>
#include <QThread>
#include <QtWidgets>

namespace GCode {

class File;

class Form : public QWidget {
    Q_OBJECT

public:
    explicit Form(Plugin* plugin, Creator* tpc);
    ~Form() override;
    virtual void editFile(File* file) = 0;

    Creator* creator() const { return creator_; }

    void setCreator(Creator* newCreator);

signals:
    void createToolpath(Params* gcp);

protected:
    void fileHandler(File* file);
    void updateButtonIconSize() {
        for(auto* button: findChildren<QPushButton*>())
            button->setIconSize({16, 16});
    }
    void addUsedGi(class ::Gi::Item* gi);
    QWidget* widget() const { return ctrWidget; }

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;
    // Form interface
    virtual void computePaths() = 0;
    virtual void updateName() = 0;

    const Params& getNewGcpWithGi();

    Direction direction = Climb;
    SideOfMilling side = Outer;
    UsedItems usedItems_;
    Side boardSide = Top;

    QString fileName_;

    QString trOutside{tr("Outside")};
    QString trDepth{tr("Depth:")};
    QString trTool{tr("Tool:")};

    bool editMode_{};
    int fileId{-1};

    int fileCount{1};
    Plugin* const plugin;

    DepthForm* dsbxDepth;
    // QLabel* label;
    QLineEdit* leName;
    QPushButton* pbClose;
    QPushButton* pbCreate;
    QWidget* content;
    QGridLayout* grid;
    Params gcp;

private:
    Creator* creator_{nullptr};

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

    // QThread thread;
    class Runer : public QThread {
        // Q_OBJECT
        Form* const form;
        Params gcp{};

    public:
        Runer(Form* form)
            : form{form} { }
        virtual ~Runer() { }
        void createGc(Params* newGcp) {
            if(0) { // FIXME
                gcp = std::move(*newGcp);
                start();
            } else {
                form->creator_->createGc(std::move(*newGcp));
            }
        }

    protected:
        // QThread interface
        void run() override {
            // setTerminationEnabled(true);
            form->creator_->createGc(std::move(gcp));
            // setTerminationEnabled(false);
        }
    } runer{this};

    File* file_;
    class QProgressDialog* progressDialog;
    int progressTimerId{};
};

} // namespace GCode
