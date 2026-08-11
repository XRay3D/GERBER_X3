/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "reloadrequestdialog.h"

#include <QCloseEvent>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

ReloadRequestDialog::ReloadRequestDialog(QWidget* parent)
    : QDialog{parent} {
    setObjectName(u"ReloadRequestDialog"_s);
    setWindowTitle(tr("Files changed on disk"));
    setModal(false);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    auto* caption = new QLabel{tr("These files have changed outside the program. Reload them into the project?"), this};
    caption->setWordWrap(true);

    auto* rowsHost = new QWidget;
    rowsLayout_ = new QVBoxLayout{rowsHost};
    rowsLayout_->setContentsMargins({});
    rowsLayout_->setSpacing(4);
    rowsLayout_->addStretch(); // строки вставляются ПЕРЕД распоркой

    scroll_ = new QScrollArea{this};
    scroll_->setWidget(rowsHost);
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    reloadAll_ = new QPushButton{QIcon::fromTheme(u"view-refresh"_s), tr("Reload all"), this};
    skipAll_ = new QPushButton{QIcon::fromTheme(u"dialog-cancel"_s), tr("Skip all"), this};

    // Ключи копией: answer() правит rows_ прямо под нами.
    connect(reloadAll_, &QPushButton::clicked, this, [this] {
        for(const QString& fileName: rows_.keys()) answer(fileName, true);
    });
    connect(skipAll_, &QPushButton::clicked, this, [this] {
        for(const QString& fileName: rows_.keys()) answer(fileName, false);
    });

    auto* bottom = new QHBoxLayout;
    bottom->addStretch();
    bottom->addWidget(reloadAll_);
    bottom->addWidget(skipAll_);

    auto* line = new QFrame{this};
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    auto* main = new QVBoxLayout{this};
    main->addWidget(caption);
    main->addWidget(scroll_, 1);
    main->addWidget(line);
    main->addLayout(bottom);

    resize(560, 240);
}

void ReloadRequestDialog::addRequest(const QString& fileName) {
    if(rows_.contains(fileName)) return;

    Row row{};
    row.widget = new QWidget;
    row.label = new QLabel{QFileInfo{fileName}.fileName(), row.widget};
    row.label->setToolTip(fileName); // полный путь -- в подсказке
    row.reload = new QPushButton{QIcon::fromTheme(u"view-refresh"_s), tr("Reload"), row.widget};
    row.skip = new QPushButton{QIcon::fromTheme(u"dialog-cancel"_s), tr("Skip"), row.widget};

    connect(row.reload, &QPushButton::clicked, this, [this, fileName] { answer(fileName, true); });
    connect(row.skip, &QPushButton::clicked, this, [this, fileName] { answer(fileName, false); });

    auto* layout = new QHBoxLayout{row.widget};
    layout->setContentsMargins({});
    layout->addWidget(row.label, 1);
    layout->addWidget(row.reload);
    layout->addWidget(row.skip);

    rowsLayout_->insertWidget(rowsLayout_->count() - 1, row.widget);
    rows_.insert(fileName, row);

    if(!isVisible()) show();
    raise();
}

void ReloadRequestDialog::answer(const QString& fileName, bool reload) {
    auto it = rows_.find(fileName);
    if(it == rows_.end()) return;
    it->widget->deleteLater();
    rows_.erase(it);

    reload ? emit reloadRequested(fileName) : emit skipRequested(fileName);

    if(rows_.isEmpty()) hide();
}

void ReloadRequestDialog::closeEvent(QCloseEvent* event) {
    // Крестик = «пропустить все»: молча забыть про изменения нельзя, иначе
    // файл останется в reloadPaths и о его следующем изменении не спросят.
    for(const QString& fileName: rows_.keys()) answer(fileName, false);
    event->accept();
}

#include "moc_reloadrequestdialog.cpp"
