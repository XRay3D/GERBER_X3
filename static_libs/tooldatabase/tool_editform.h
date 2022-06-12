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

#include "doublespinbox.h"
#include "mvector.h"
#include "tool.h"
#include <QWidget>

namespace Ui {
class ToolEditForm;
}

class ToolItem;

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
        QMap<DoubleSpinBox*, double> min;
        QMap<DoubleSpinBox*, double> max;
        QMap<DoubleSpinBox*, double> val;
        void save(DoubleSpinBox* dsbx) {
            min[dsbx] = dsbx->minimum();
            max[dsbx] = dsbx->maximum();
            val[dsbx] = dsbx->value();
        }
        void restore(DoubleSpinBox* dsbx) {
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
    mvector<DoubleSpinBox*> dsbx;
};
