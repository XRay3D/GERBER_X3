#pragma once
#ifndef GCODEPROPERTIESFORM_H
#define GCODEPROPERTIESFORM_H

#include "point.h"
#include <QWidget>

namespace Ui {
class GCodePropertiesForm;
}

class GCodePropertiesForm : public QWidget {
    Q_OBJECT

public:
    explicit GCodePropertiesForm(QWidget* parent);
    ~GCodePropertiesForm() override;

    void updatePosDsbxs();
    void updateAll();

    static double safeZ;
    static double boardThickness;
    static double copperThickness;
    static double clearence;
    static double plunge;
    static double glue;

private slots:
    void on_pbResetHome_clicked();

    void on_pbResetZero_clicked();

private:
    Ui::GCodePropertiesForm* ui;
};

#endif // GCODEPROPERTIESFORM_H
