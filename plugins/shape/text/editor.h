/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include <QAbstractTableModel>
#include <QActionGroup>
#include <QTableView>
#include <QWidget>

class DoubleSpinBox;
class QCheckBox;
class QComboBox;
class QFontComboBox;
class QPlainTextEdit;
class QRadioButton;

namespace ShTxt {

class Editor : public QWidget {
    Q_OBJECT
    friend class Shape;
    std::vector<Shape*> shapes;

    DoubleSpinBox* dsbxAngle;
    DoubleSpinBox* dsbxHeight;
    DoubleSpinBox* dsbxXY;
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
    void updateCenterAlign();
    void updateSide();

public:
    Editor(class Plugin* plugin);

    void addShape(Shape* shape);

    void reset();

    ~Editor() override = default;
    //    Model* model;
    class Plugin* plugin;

    // QWidget interface
protected:
    void hideEvent(QHideEvent* event) override;
};

} // namespace ShTxt
