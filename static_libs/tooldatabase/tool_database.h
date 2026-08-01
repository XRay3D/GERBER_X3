/********************************************************************************
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
#include "mvector.h"
#include "tool.h"
#include <QDialog>

namespace Ui {
class ToolDatabase;
}
class ToolItem;

class ToolDatabase : public QDialog {
    Q_OBJECT

    static constexpr std::array all{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser};

public:
    explicit ToolDatabase(
        QWidget* parent                   = nullptr,
        std::span<const Tool::Type> types = all);

    ~ToolDatabase() override;

public:
    Tool tool() const;

private:
    Ui::ToolDatabase* ui;
    Tool tool_;
    ToolItem* item_;
    std::span<const Tool::Type> types_;

protected:
    void keyPressEvent(QKeyEvent* evt) override;
    // QWidget interface
    void showEvent(QShowEvent* event) override;
};
