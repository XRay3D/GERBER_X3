#pragma once

#include "forms/formsutil/formsutil.h"

class ThermalModel;
class ThermalPreviewItem;
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

    void on_pbExclude_clicked();

private:
    Ui::ThermalForm* ui;

    void createTPI(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    QVector<QSharedPointer<ThermalPreviewItem>> m_sourcePreview;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    ThermalModel* model = nullptr;
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

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
