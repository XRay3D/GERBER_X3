/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <QDialog>

class QDialogButtonBox;
class QGraphicsView;
class QSplitter;
class QVBoxLayout;
class QGraphicsScene;

namespace Gerber {

class ComponentsView;

class ComponentsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ComponentsDialog(QWidget* parent = nullptr);
    ~ComponentsDialog();
    void setFile(int fileId);
    static QGraphicsScene* scene() { return m_scene; };

private:
    //QDialogButtonBox* buttonBox;
    //QVBoxLayout* verticalLayout;
    ComponentsView* componentsView;
    QGraphicsView* graphicsView;
    QSplitter* splitter;
    static inline QGraphicsScene* m_scene;

    void setupUi(QDialog* dialog); // setupUi

    void retranslateUi(QDialog* dialog); // retranslateUi
    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event = nullptr) override;
};

}
