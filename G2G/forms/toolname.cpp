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
