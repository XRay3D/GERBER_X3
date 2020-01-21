#ifndef TOOLEDITFORM_H
#define TOOLEDITFORM_H

#include "tool.h"

#include <QWidget>

namespace Ui {
class ToolEditForm;
}

class ToolItem;
class DoubleSpinBox;

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
        mm_sec,
        mm_min,
        cm_min,
        m_min
    };

public slots:
    void on_pbApply_clicked();

signals:
    void itemChanged(ToolItem* item);
    void toolChanged(const Tool& item);

private:
    Ui::ToolEditForm* ui;

    void valueChangedSlot(double value);

    ToolItem* m_item = nullptr;
    Tool m_tool;
    double m_feed = 1.0;
    bool m_dialog = true;
    double m_angle = 0.0;

    void updateName();
    void setChanged(bool fl = true);
    void setVisibleToolWidgets(bool visible);
    void setupToolWidgets(int type);
    QVector<DoubleSpinBox*> dsbx;
};

#endif // TOOLEDITFORM_H
