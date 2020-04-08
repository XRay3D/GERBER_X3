#pragma once
#ifndef TOOLNAME_H
#define TOOLNAME_H

#include <QWidget>

#include "tooldatabase/tool.h"

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
