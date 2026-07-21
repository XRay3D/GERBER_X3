/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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

protected:
    // Снимок значений делаем при каждом показе панели, а не только в конструкторе,
    // т.к. виджет переиспользуется (живёт в QDockWidget) между открытиями.
    void showEvent(QShowEvent* event) override;
    // Если панель закрыли не через "OK" (committed_ == false), откатываем все поля
    // формы к снимку — они применяются "живьём" по valueChanged (Project/маркеры).
    void hideEvent(QHideEvent* event) override;

private slots:
    void on_pbResetHome_clicked();
    void on_pbResetZero_clicked();

private:
    Ui::GCodePropertiesForm* ui;
    bool committed_ = false;

    struct Snapshot {
        double safeZ{}, clearence{}, plunge{};
        double thickness{}, copperThickness{}, glue{};
        double spaceX{}, spaceY{};
        int stepsX{}, stepsY{};
        double homeX{}, homeY{}, zeroX{}, zeroY{};
    } snapshot_;

    void applySnapshot(const Snapshot& s);
};

} // namespace GCode
