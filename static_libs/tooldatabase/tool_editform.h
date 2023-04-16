/********************************************************************************
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

#include "doublespinbox.h"
#include "mvector.h"
#include "tool.h"
#include <QWidget>
#include <array>
#include <optional>
#include <set>
#include <variant>

namespace Ui {
class ToolEditForm;
}

class ToolItem;

class ToolEditForm : public QWidget {
    Q_OBJECT
    friend class ToolEditDialog;

public:
    explicit ToolEditForm(QWidget* parent = nullptr);
    ~ToolEditForm();

    void setItem(ToolItem* item);
    void setTool(const Tool& tool);
    void setDialog();
    enum Feeds {
        mmPerSec,
        mmPerMin,
        cmPerMin,
        mPerMin
    };

public slots:
    void on_pbApply_clicked();

signals:
    void itemChanged(ToolItem* item);
    void toolChanged(const Tool& item);

private:
    Ui::ToolEditForm* ui;

    void valueChanged(double value);

    struct Data {
        DoubleSpinBox* dsbx[2];
        std::set<Tool::Type> set;
        std::variant<double, DoubleSpinBox*> max;
        std::optional<double> defVal;
        std::optional<double> lastVal;
    };

    std::array<Data, 7> dsbxMapdsbxMap;

    ToolItem* item_ {nullptr};
    Tool tool_;
    double feed {1.0};
    bool dialog_ {true};

    void updateName();
    void setChanged(bool fl = true);
    void setVisibleToolWidgets(bool visible);
    void setupToolWidgets(int type);
    using Key = decltype(&Tool::angle);

    void updateDsbxAngle(double val);
    void updateDsbxDiameter(double val);
    void updateDsbxFeedRate(double val);
    void updateDsbxOneTurnCut(double val);
    void updateDsbxPassDepth(double val);
    void updateDsbxPlungeRate(double val);
    void updateDsbxSpindleSpeed(double val);
    void updateDsbxStepover(double val);
    void updateDsbxLenght(double val);
    void updateDsbxOneTurnCutPercent(double val);
    void updateDsbxStepoverPercent(double val);

    std::map<QDoubleSpinBox*, decltype(&ToolEditForm::updateDsbxAngle)> update;
    std::array<std::pair<DoubleSpinBox*, decltype(&Tool::angle)>, 9> get;
    std::array<std::pair<DoubleSpinBox*, decltype(&Tool::setAngle)>, 9> set;

    //    mvector<DoubleSpinBox*> update;
};
