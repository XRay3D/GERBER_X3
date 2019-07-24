#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>

class QGridLayout;
class GLWidget;
class QListWidget;
class QLabel;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow();

private slots:
    void primary_edges_only();

    void internal_edges_only();

    void browse();

    void build();

    void print_scr();

private:
    QGridLayout* create_file_layout();

    void update_file_list();

    QDir file_dir_;
    QString file_name_;
    GLWidget* glWidget_;
    QListWidget* file_list_;
    QLabel* message_label_;
};

#endif // MAINWINDOW_H
