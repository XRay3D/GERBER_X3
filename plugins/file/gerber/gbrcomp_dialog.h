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

#include <QDialog>

class QDialogButtonBox;
class QGraphicsView;
class QSplitter;
class QVBoxLayout;
class QGraphicsScene;

namespace Gerber::Comp {

class sView;

class Dialog : public QDialog {
    Q_OBJECT

public:
    explicit Dialog(QWidget* parent = nullptr);
    ~Dialog();
    void setFile(int fileId);
    static QGraphicsScene* scene() { return scene_; };

private:
    // QDialogButtonBox* buttonBox;
    // QVBoxLayout* verticalLayout;
    sView* componentsView;
    QGraphicsView* grView;
    QSplitter* splitter;
    static inline QGraphicsScene* scene_;

    void setupUi(QDialog* dialog);       // setupUi

    void retranslateUi(QDialog* dialog); // retranslateUi
    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event = nullptr) override;
};

} // namespace Gerber::Comp
