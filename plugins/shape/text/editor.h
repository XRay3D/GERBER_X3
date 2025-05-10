/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "shapepluginin.h"
#include <QAbstractTableModel>
#include <QActionGroup>
#include <QTableView>

class DoubleSpinBox;
class QCheckBox;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
class QRadioButton;

namespace ShTxt {

class Shape;

class Editor : public Shapes::Editor {
    Q_OBJECT
    friend class Shape;
    std::vector<Shape*> shapes;

    DoubleSpinBox* dsbxAngle;
    DoubleSpinBox* dsbxHeight;
    DoubleSpinBox* dsbxXY;
    DoubleSpinBox* dsbxX;
    DoubleSpinBox* dsbxY;
    QCheckBox* chbxBold;
    QCheckBox* chbxItalic;
    QComboBox* cbxSide;
    QFontComboBox* cbxFont;
    QPlainTextEdit* plainTextEdit;
    QRadioButton* rb_bc;
    QRadioButton* rb_bl;
    QRadioButton* rb_br;
    QRadioButton* rb_cc;
    QRadioButton* rb_lc;
    QRadioButton* rb_rc;
    QRadioButton* rb_tc;
    QRadioButton* rb_tl;
    QRadioButton* rb_tr;

    void setupUi();

    void updateText();
    void updateFont();
    void updateAngle();
    void updateHeight();
    void updateXY();
    void updateX();
    void updateY();
    void updateCenterAlign();
    void updateSide();
    bool resetFl{};

public:
    Editor(Shapes::Plugin* plugin);

    void add(Shapes::AbstractShape* shape) override;
    void remove(Shapes::AbstractShape* shape) override;
    void updateData() override { reset(); }

    void reset();

    ~Editor() override = default;
    //    Model* model;
    Shapes::Plugin* plugin;

    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
};

} // namespace ShTxt
