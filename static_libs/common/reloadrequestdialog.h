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
class QPushButton;
class QVBoxLayout;
class QScrollArea;

// Одно окно на все файлы, изменившиеся на диске: строка на файл со своими
// «Перезагрузить»/«Пропустить», внизу те же две кнопки для всех сразу.
//
// Раньше на каждый файл поднимался модальный QMessageBox::question
// (project.cpp). Пересборка платы в САПР переписывает все слои разом, и
// пользователь получал очередь из десятка модальных окон, каждое из которых
// блокировало работу, пока на него не ответишь.
//
// Немодальное намеренно: ответы не срочные, а файлы могут меняться и дальше --
// новая строка должна лечь в то же окно.
class ReloadRequestDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReloadRequestDialog(QWidget* parent = nullptr);

    // Ключ -- полный путь. Повторный вызов для того же пути ничего не
    // добавляет: файл может «измениться» несколько раз, пока строка висит.
    void addRequest(const QString& fileName);
    bool hasRequest(const QString& fileName) const { return rows_.contains(fileName); }

signals:
    void reloadRequested(const QString& fileName);
    void skipRequested(const QString& fileName);

private:
    struct Row {
        QWidget* widget;
        QLabel* label;
        QPushButton* reload;
        QPushButton* skip;
    };

    QHash<QString, Row> rows_;
    QVBoxLayout* rowsLayout_;
    QScrollArea* scroll_;
    QPushButton* reloadAll_;
    QPushButton* skipAll_;

    // Ответить за строку и убрать её. Окно прячется, когда строк не осталось.
    void answer(const QString& fileName, bool reload);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
};
