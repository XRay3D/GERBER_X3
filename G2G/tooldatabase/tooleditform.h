#ifndef TOOLEDITFORM_H
#define TOOLEDITFORM_H

#include "tool.h"

#include <QDoubleSpinBox>
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

    struct State {
        QMap<QDoubleSpinBox*, double> min;
        QMap<QDoubleSpinBox*, double> max;
        QMap<QDoubleSpinBox*, double> val;
        void save(QDoubleSpinBox* dsbx)
        {
            min[dsbx] = dsbx->minimum();
            max[dsbx] = dsbx->maximum();
            val[dsbx] = dsbx->value();
        }
        void restore(QDoubleSpinBox* dsbx)
        {
            dsbx->setRange(min[dsbx], max[dsbx]);
            dsbx->setValue(val[dsbx]);
        }
    };

    QMap<int, State> saveRestoreMap;

    void valueChangedSlot(double value);

    ToolItem* m_item = nullptr;
    Tool m_tool;
    double m_feed = 1.0;
    bool m_dialog = true;

    void updateName();
    void setChanged(bool fl = true);
    void setVisibleToolWidgets(bool visible);
    void setupToolWidgets(int type);
    QVector<DoubleSpinBox*> dsbx;
};

#endif // TOOLEDITFORM_H
