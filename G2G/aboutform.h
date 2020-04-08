#pragma once
#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#pragma once

#include <QDialog>

namespace Ui {
class AboutForm;
}

class AboutForm : public QDialog {
    Q_OBJECT

public:
    explicit AboutForm(QWidget* parent = nullptr);
    ~AboutForm() override;

private:
    Ui::AboutForm* ui;
};

#endif // ABOUTFORM_H
