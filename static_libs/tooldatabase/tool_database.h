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
#include "mvector.h"
#include "tool.h"
#include <QDialog>

namespace Ui {
class ToolDatabase;
}
class ToolItem;

class ToolDatabase : public QDialog {
    Q_OBJECT

public:
    explicit ToolDatabase(
        QWidget* parent = nullptr,
        mvector<Tool::Type> types = mvector<Tool::Type>{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser});

    ~ToolDatabase() override;

public:
    Tool tool() const;

private:
    Ui::ToolDatabase* ui;
    Tool tool_;
    ToolItem* item_;
    const mvector<Tool::Type> types_;

protected:
    void keyPressEvent(QKeyEvent* evt) override;
};
