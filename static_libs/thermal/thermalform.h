/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "forms/formsutil/formsutil.h"

#include "thvars.h"

class ThermalModel;
class AbstractThermPrGi;
class QCheckBox;
class QItemSelection;

namespace Ui {
class ThermalForm;
}

namespace Gerber {
class AbstractAperture;
}

class ThermalForm : public FormsUtil {
    Q_OBJECT

public:
    explicit ThermalForm(QWidget* parent);
    ~ThermalForm() override;

    void updateFiles();
    static bool canToShow();

private slots:
    void on_leName_textChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);
    void on_dsbxDepth_valueChanged(double arg1);

    void on_dsbxAreaMax_editingFinished();
    void on_dsbxAreaMin_editingFinished();

private:
    Ui::ThermalForm* ui;

    void createTPI(FileInterface* file);

    mvector<std::shared_ptr<AbstractThermPrGi>> m_sourcePreview;

    ThermalModel* model = nullptr;
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void setSelection(const QModelIndex& selected, const QModelIndex& deselected);

    ThParam par;
    double lastMax;
    double lastMin;

    QCheckBox* chbx;
    double m_depth;
    inline void redraw();
    Tool tool;

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};
