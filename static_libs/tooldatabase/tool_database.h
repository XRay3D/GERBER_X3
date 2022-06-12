/********************************************************************************
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
        mvector<Tool::Type> types = mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser });

    ~ToolDatabase() override;

public:
    Tool tool() const;

private:
    Ui::ToolDatabase* ui;
    Tool m_tool;
    ToolItem* m_item;
    const mvector<Tool::Type> m_types;

protected:
    void keyPressEvent(QKeyEvent* evt) override;
};
