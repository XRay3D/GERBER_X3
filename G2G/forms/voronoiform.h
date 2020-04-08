#pragma once
#ifndef VORONOIFORM_H
#define VORONOIFORM_H

#include "formsutil/formsutil.h"

namespace Ui {
class VoronoiForm;
}

class VoronoiForm : public FormsUtil {
    Q_OBJECT

public:
    explicit VoronoiForm(QWidget* parent = nullptr);
    ~VoronoiForm() override;

private slots:
    void on_leName_textChanged(const QString& arg1);
    void on_cbxSolver_currentIndexChanged(int index);

private:
    Ui::VoronoiForm* ui;

    double m_size = 0.0;
    double m_lenght = 0.0;
    void setWidth(double w);

    // FormsUtil interface
protected:
    void createFile() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

#endif // VORONOIFORM_H
