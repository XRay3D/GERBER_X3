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
#pragma once

#include <QDialog>
#include <QHash>

class QLabel;
class QProgressBar;
class QPushButton;
class QVBoxLayout;
class QScrollArea;

// Одно окно на все загружаемые файлы: строка на файл, у каждой своя отмена,
// внизу общий счётчик и общая отмена.
//
// Раньше на каждый файл создавался отдельный QProgressDialog без кнопки отмены
// (mainwindow.cpp), и бросок пачки файлов давал стопку окон друг на друге.
//
// Окно живёт всё время работы приложения и лишь прячется, когда очередь пуста:
// пользователь может докинуть файлы в любой момент, и новая строка должна
// появиться в том же окне, а не поднять второе.
class LoadProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadProgressDialog(QWidget* parent = nullptr);

    // Завести строку. Ключ -- полный путь файла; повторный вызов для того же
    // пути ничего не создаёт (файл мог начать разбор до того, как о нём
    // сообщили, либо плагин прислал старт дважды).
    void addTask(const QString& fileName);
    // max == 0 -- «шагов не знаю»: полоса бежит сама, врать долей не надо.
    void setTaskProgress(const QString& fileName, int max, int value);
    // Строка уходит; когда уходит последняя -- окно прячется.
    void removeTask(const QString& fileName);

    bool hasTask(const QString& fileName) const { return rows_.contains(fileName); }
    int activeCount() const { return rows_.size(); }

signals:
    // Пользователь нажал отмену у строки или общую. Отменой занимается
    // владелец (MainWindow): окно само разбор не останавливает и строку не
    // убирает -- строка исчезнет, когда плагин отчитается о завершении.
    void cancelRequested(const QString& fileName);

private:
    struct Row {
        QWidget* widget;
        QLabel* label;
        QProgressBar* bar;
        QPushButton* cancel;
    };

    QHash<QString, Row> rows_;
    QVBoxLayout* rowsLayout_;
    QScrollArea* scroll_;
    QProgressBar* totalBar_;
    QLabel* totalLabel_;
    QPushButton* cancelAll_;

    // Сколько файлов всего было в этой «сессии» загрузки и сколько закрыто.
    // Счётчик обнуляется, когда очередь опустела: следующий бросок файлов --
    // это уже новая сессия, и «готово 7 из 7» в ней ни к чему.
    int total_{};
    int done_{};
    void updateTotal();

    // Окно показывается не сразу: мелкий файл разбирается за миллисекунды, и
    // без задержки окно мигало бы на каждой загрузке. Значение -- как у
    // QProgressDialog по умолчанию.
    static constexpr int showDelayMs = 400;
    void showLater();
    bool showScheduled_{};

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
};
