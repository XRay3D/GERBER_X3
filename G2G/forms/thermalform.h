#ifndef THERMALFORM_H
#define THERMALFORM_H

#include "forms/thermalmodel.h"
#include "forms/thermalpreviewitem.h"
#include "formsutil.h"
#include <QItemSelection>

namespace Ui {
class ThermalForm;
}

class QCheckBox;
class QGridLayout;
namespace Gerber {
class AbstractAperture;
}

class ThermalForm : public FormsUtil {
    Q_OBJECT

public:
    explicit ThermalForm(QWidget* parent = nullptr);
    ~ThermalForm() override;

    void updateFiles();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_leName_textChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);
    void on_dsbxDepth_valueChanged(double arg1);

    void on_pbExclude_clicked();

private:
    Ui::ThermalForm* ui;

    void createTPI(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    QVector<QSharedPointer<ThermalPreviewItem>> m_sourcePreview;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    ThermalModel* model;
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    QCheckBox* cbx;
    QGridLayout* lay;
    double m_depth;
    inline void redraw();

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#endif // THERMALFORM_H
