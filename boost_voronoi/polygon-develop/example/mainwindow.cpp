#include "mainwindow.h"
#include "glwidget.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>

MainWindow::MainWindow()
{
    glWidget_ = new GLWidget();
    file_dir_ = QDir(QDir::currentPath(), tr("*.txt"));
    file_name_ = tr("");

    QHBoxLayout* centralLayout = new QHBoxLayout;
    centralLayout->addWidget(glWidget_);
    centralLayout->addLayout(create_file_layout());
    setLayout(centralLayout);

    update_file_list();
    setWindowTitle(tr("Voronoi Visualizer"));
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void MainWindow::primary_edges_only()
{
    glWidget_->show_primary_edges_only();
}

void MainWindow::internal_edges_only()
{
    glWidget_->show_internal_edges_only();
}

void MainWindow::browse()
{
    QString new_path = QFileDialog::getExistingDirectory(
        0, tr("Choose Directory"), file_dir_.absolutePath());
    if (new_path.isEmpty()) {
        return;
    }
    file_dir_.setPath(new_path);
    update_file_list();
}

void MainWindow::build()
{
    file_name_ = file_list_->currentItem()->text();
    QString file_path = file_dir_.filePath(file_name_);
    message_label_->setText("Building...");
    glWidget_->build(file_path);
    message_label_->setText("Double click the item to build voronoi diagram:");
    setWindowTitle(tr("Voronoi Visualizer - ") + file_path);
}

void MainWindow::print_scr()
{
    if (!file_name_.isEmpty()) {
        QImage screenshot = glWidget_->grabFrameBuffer(true);
        QString output_file = file_dir_.absolutePath() + tr("/") + file_name_.left(file_name_.indexOf('.')) + tr(".png");
        screenshot.save(output_file, 0, -1);
    }
}

QGridLayout* MainWindow::create_file_layout()
{
    QGridLayout* file_layout = new QGridLayout;

    message_label_ = new QLabel("Double click item to build voronoi diagram:");

    file_list_ = new QListWidget();
    file_list_->connect(file_list_,
        SIGNAL(itemDoubleClicked(QListWidgetItem*)),
        this,
        SLOT(build()));

    QCheckBox* primary_checkbox = new QCheckBox("Show primary edges only.");
    connect(primary_checkbox, SIGNAL(clicked()),
        this, SLOT(primary_edges_only()));

    QCheckBox* internal_checkbox = new QCheckBox("Show internal edges only.");
    connect(internal_checkbox, SIGNAL(clicked()),
        this, SLOT(internal_edges_only()));

    QPushButton* browse_button = new QPushButton(tr("Browse Input Directory"));
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse()));
    browse_button->setMinimumHeight(50);

    QPushButton* print_scr_button = new QPushButton(tr("Make Screenshot"));
    connect(print_scr_button, SIGNAL(clicked()), this, SLOT(print_scr()));
    print_scr_button->setMinimumHeight(50);

    file_layout->addWidget(message_label_, 0, 0);
    file_layout->addWidget(file_list_, 1, 0);
    file_layout->addWidget(primary_checkbox, 2, 0);
    file_layout->addWidget(internal_checkbox, 3, 0);
    file_layout->addWidget(browse_button, 4, 0);
    file_layout->addWidget(print_scr_button, 5, 0);

    return file_layout;
}

void MainWindow::update_file_list()
{
    QFileInfoList list = file_dir_.entryInfoList();
    file_list_->clear();
    if (file_dir_.count() == 0) {
        return;
    }
    QFileInfoList::const_iterator it;
    for (it = list.begin(); it != list.end(); it++) {
        file_list_->addItem(it->fileName());
    }
    file_list_->setCurrentRow(0);
}
