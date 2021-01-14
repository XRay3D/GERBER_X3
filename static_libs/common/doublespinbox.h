/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTimer>

class DoubleSpinBox : public QDoubleSpinBox {
//    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget* parent = nullptr)
        : QDoubleSpinBox(parent)
    {
        lineEdit()->installEventFilter(this);
        setToolTipDuration(0);
    }
    void setRange(double min, double max)
    {
        QDoubleSpinBox::setRange(min, max);
        setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
    }
    void setMaximum(double max)
    {
        QDoubleSpinBox::setMaximum(max);
        setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
    }
    void setMinimum(double min)
    {
        QDoubleSpinBox::setMinimum(min);
        setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
    }
    void flicker()
    {
        if (qFuzzyIsNull(value()))
            for (int i = 0, t = 0; i < 3; ++i) {
                QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::red);
                QTimer::singleShot(++t * 150, Qt::CoarseTimer, this, &DoubleSpinBox::normal);
            }
    }

    // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::MouseButtonRelease)
            lineEdit()->setSelection(0, lineEdit()->text().length() - suffix().length()); //->selectAll();
        return QDoubleSpinBox::eventFilter(watched, event);
    }

private:
    void red() { setStyleSheet("QWidget{ background-color: red; }"); }
    void normal() { setStyleSheet(""); }

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        //    if (event->key() == Qt::Key_Backspace) {
        //        QString text(lineEdit()->text());
        //        int start = lineEdit()->selectionStart();
        //        text.remove(--start, 1);
        //        lineEdit()->setText(text);
        //        lineEdit()->setSelection(start, 100);
        //        return;
        //    }
        if (event->text() == '.' || event->text() == ',') {
            QKeyEvent ke(event->type(), Qt::Key_Comma, event->modifiers(), QLocale().decimalPoint());
            QDoubleSpinBox::keyPressEvent(&ke);
        } else
            QDoubleSpinBox::keyPressEvent(event);
        //    lineEdit()->setSelection(lineEdit()->cursorPosition(), 100);
    }
};
