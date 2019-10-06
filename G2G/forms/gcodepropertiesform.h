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
    explicit GCodePropertiesForm(QWidget* prnt = nullptr);
    ~GCodePropertiesForm() override;

    static void updatePosDsbxs();

    static Marker* homePoint;
    static Marker* zeroPoint;
    static double safeZ;
    static double thickness;
    static double copperThickness;
    static double clearence;
    static double plunge;
    static double glue;
    static GCodePropertiesForm* self;

private:
    Ui::GCodePropertiesForm* ui;
};

#endif // GCODEPROPERTIESFORM_H
