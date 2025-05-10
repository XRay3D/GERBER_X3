/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include <QWidget>

namespace Ui {
class GCodePropertiesForm;
}

namespace GCode {

class PropertiesForm : public QWidget {
    Q_OBJECT

    explicit PropertiesForm(QWidget* parent = nullptr);

public:
    static auto create(QWidget* parent = nullptr) {
        return std::unique_ptr<PropertiesForm>(new PropertiesForm{parent});
    }

    ~PropertiesForm() override;

    void updatePosDsbxs();
    void updateAll();

    void load();
    void save();

private slots:
    void on_pbResetHome_clicked();
    void on_pbResetZero_clicked();

private:
    Ui::GCodePropertiesForm* ui;
};

} // namespace GCode
