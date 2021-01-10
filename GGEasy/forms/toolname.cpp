// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#include "toolname.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>

ToolName::ToolName(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* l = new QHBoxLayout(this);
    lblPixmap = new QLabel(this);
    l->addWidget(lblPixmap);
    lblName = new QLabel(this);
    lblName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    l->addWidget(lblName);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(6);
    l->setStretch(1, 1);
}

void ToolName::setTool(const Tool& tool)
{
    lblPixmap->setPixmap(tool.icon().pixmap({ 22, 22 }));
    lblName->setText(tool.name());
    setToolTip(tool.note());
}
