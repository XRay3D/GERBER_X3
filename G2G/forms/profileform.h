#ifndef PROFILEFORM_H
#define PROFILEFORM_H

#include "formsutil.h"

namespace Ui {
class ProfileForm;
}

class ProfileForm : public FormsUtil {
    Q_OBJECT

public:
    explicit ProfileForm(QWidget* parent = nullptr);
    ~ProfileForm();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_pbAddBridge_clicked();
    void on_dsbxBridgeLenght_valueChanged(double arg1);
    void on_dsbxDepth_valueChanged(double arg1);
    void on_leName_textChanged(const QString& arg1);

signals:
    void createProfile(const Tool& tool, double depth);

private:
    Ui::ProfileForm* ui;
    double m_size = 0.0;
    double m_lenght = 0.0;
    void updateBridge();
    void updatePixmap();

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;

    // FormsUtil interface
protected:
    void create() override;
    void updateName() override;

public:
    virtual void editFile(GCode::File* file) override;
};

#endif // PROFILEFORM_H
