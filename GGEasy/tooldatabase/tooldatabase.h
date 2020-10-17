/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
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
        QVector<Tool::Type> types = QVector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser });

    ~ToolDatabase() override;

public:
    Tool tool() const;

private:
    Ui::ToolDatabase* ui;
    Tool m_tool;
    ToolItem* m_item;
    const QVector<Tool::Type> m_types;

protected:
    void keyPressEvent(QKeyEvent* evt) override;
};
