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
#ifndef TOOLNAME_H
#define TOOLNAME_H

#include <QWidget>

#include "toolpch.h"

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
