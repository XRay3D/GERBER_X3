#ifndef POCKETFORM_H
#define POCKETFORM_H

#include "formsutil.h"

namespace Ui {
class PocketForm;
}

class PocketForm : public FormsUtil {
    Q_OBJECT

public:
    explicit PocketForm(QWidget* parent = nullptr);
    ~PocketForm();

signals:
    void createPocket(const Tool& tool, const double depth, const int steps);
    void createPocket2(const QPair<Tool, Tool>& tool, double depth);
    void createRaster(const Tool& tool, const double depth, const double angle, const int pPass);

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_pbSelect_2_clicked();
    void on_pbEdit_2_clicked();
    void on_sbxSteps_valueChanged(int arg1);
    void on_chbxUseTwoTools_clicked(bool checked);
    void on_leName_textChanged(const QString& arg1);

private:
    Ui::PocketForm* ui;

    int direction = 0;
    int type = 0;
    void updatePixmap();

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    // FormsUtil interface
protected:
    void create() override;
    void updateName() override;

public:
    virtual void editFile(GCode::File* file) override;
};

#endif // POCKETFORM_H
