#pragma once

#include <QDockWidget>
#include <QStack>

class DockWidget : public QDockWidget {
    Q_OBJECT
    QStack<QWidget*> widgets;
    //    void setWidget(QWidget*) { }

public:
    explicit DockWidget(QWidget* parent = nullptr);
    ~DockWidget() override = default;

    void push(QWidget* w);
    void pop();

    // QWidget interface
protected:
    //    void closeEvent(QCloseEvent* event) override;
    //    void showEvent(QShowEvent* event) override;
}; ///// end DockWidget
