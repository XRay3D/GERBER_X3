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
#include "loadprogressdialog.h"

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

LoadProgressDialog::LoadProgressDialog(QWidget* parent)
    : QDialog{parent} {
    setObjectName(u"LoadProgressDialog"_s);
    setWindowTitle(tr("Loading files"));
    // Немодальное: пользователь должен иметь возможность докинуть ещё файлов,
    // не закрывая окна.
    setModal(false);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    auto* rowsHost = new QWidget;
    rowsLayout_ = new QVBoxLayout{rowsHost};
    rowsLayout_->setContentsMargins({});
    rowsLayout_->setSpacing(4);
    rowsLayout_->addStretch(); // строки добавляются ПЕРЕД распоркой

    scroll_ = new QScrollArea{this};
    scroll_->setWidget(rowsHost);
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    totalLabel_ = new QLabel{this};
    totalBar_ = new QProgressBar{this};
    totalBar_->setTextVisible(false);

    cancelAll_ = new QPushButton{QIcon::fromTheme(u"dialog-cancel"_s), tr("Cancel all"), this};
    connect(cancelAll_, &QPushButton::clicked, this, [this] {
        // Копия ключей: обработчик отмены в конце концов приведёт к
        // removeTask, а это правит rows_ прямо под нами.
        for(const QString& fileName: rows_.keys()) emit cancelRequested(fileName);
    });

    auto* bottom = new QHBoxLayout;
    bottom->addWidget(totalLabel_);
    bottom->addWidget(totalBar_, 1);
    bottom->addWidget(cancelAll_);

    auto* line = new QFrame{this};
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    auto* main = new QVBoxLayout{this};
    main->addWidget(scroll_, 1);
    main->addWidget(line);
    main->addLayout(bottom);

    resize(520, 240);
}

void LoadProgressDialog::addTask(const QString& fileName) {
    if(rows_.contains(fileName)) return;

    Row row{};
    row.widget = new QWidget;
    row.label = new QLabel{QFileInfo{fileName}.fileName(), row.widget};
    row.label->setToolTip(fileName); // полный путь -- в подсказке
    row.bar = new QProgressBar{row.widget};
    row.bar->setMaximum(0); // пока «шагов не знаю»
    row.cancel = new QPushButton{QIcon::fromTheme(u"dialog-cancel"_s), {}, row.widget};
    row.cancel->setToolTip(tr("Cancel loading %1").arg(QFileInfo{fileName}.fileName()));
    row.cancel->setFlat(true);

    connect(row.cancel, &QPushButton::clicked, this, [this, fileName] {
        // Второй раз жать бессмысленно: отмена уже запрошена, а строка
        // исчезнет, когда разбор до неё дойдёт.
        if(auto it = rows_.find(fileName); it != rows_.end())
            it->cancel->setEnabled(false);
        emit cancelRequested(fileName);
    });

    auto* layout = new QHBoxLayout{row.widget};
    layout->setContentsMargins({});
    layout->addWidget(row.label, 2);
    layout->addWidget(row.bar, 3);
    layout->addWidget(row.cancel);

    // Перед распоркой, чтобы строки прижимались кверху.
    rowsLayout_->insertWidget(rowsLayout_->count() - 1, row.widget);
    rows_.insert(fileName, row);

    ++total_;
    updateTotal();

    showLater();
}

void LoadProgressDialog::showLater() {
    if(isVisible() || showScheduled_) return;
    showScheduled_ = true;
    QTimer::singleShot(showDelayMs, this, [this] {
        showScheduled_ = false;
        // За время задержки всё могло успеть загрузиться -- тогда показывать
        // нечего.
        if(rows_.isEmpty()) return;
        show();
        raise();
    });
}

void LoadProgressDialog::setTaskProgress(const QString& fileName, int max, int value) {
    auto it = rows_.find(fileName);
    if(it == rows_.end()) return; // прогресс от файла, о котором не объявляли
    if(max > 0) it->bar->setMaximum(max);
    it->bar->setValue(value);
}

void LoadProgressDialog::removeTask(const QString& fileName) {
    auto it = rows_.find(fileName);
    if(it == rows_.end()) return;
    it->widget->deleteLater();
    rows_.erase(it);
    ++done_;
    updateTotal();

    if(rows_.isEmpty()) {
        // Очередь пуста -- сессия закончилась. Следующий бросок файлов считается
        // с нуля, иначе счётчик копил бы итоги за всё время работы.
        total_ = done_ = 0;
        hide();
    }
}

void LoadProgressDialog::updateTotal() {
    totalLabel_->setText(tr("Done: %1 of %2").arg(done_).arg(total_));
    totalBar_->setMaximum(total_);
    totalBar_->setValue(done_);
}

void LoadProgressDialog::closeEvent(QCloseEvent* event) {
    // Крестик окна = отменить всё: оставлять разбор идти при закрытом окне
    // нельзя -- вернуть окно пользователю нечем.
    for(const QString& fileName: rows_.keys()) emit cancelRequested(fileName);
    event->accept();
}

#include "moc_loadprogressdialog.cpp"
