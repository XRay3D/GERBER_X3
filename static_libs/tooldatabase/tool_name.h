/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#ifndef TOOLNAME_H
#define TOOLNAME_H

#include <QWidget>

#include "tool_pch.h"

class QLabel;

class ToolName : public QWidget {
    Q_OBJECT
    QLabel* lblPixmap;
    QLabel* lblName;

public:
    explicit ToolName(QWidget* parent = nullptr);
    void setTool(const Tool& tool);
signals:
};

#endif // TOOLNAME_H
