/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  October 04, 2026                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include <QWidget>

class QLabel;
class QLineEdit;
class QTreeWidget;

namespace Gerber {

class File;

// Список X2-цепей (%TO.N) файла в доке главного окна. Выделенные строки --
// это выделение сцены: элементы выбранных цепей выделены, прочие нет.
class NetList : public QWidget {
    Q_OBJECT

public:
    // Одна на приложение: док при замене виджета лишь отвязывает прежний,
    // так что владеть им некому, кроме самой формы.
    static NetList* instance();
    static NetList* existing(); // без создания: nullptr, если форму не открывали

    void setFile(const File* file);
    void fileClosed(int32_t fileId);

private:
    explicit NetList(QWidget* parent = nullptr);
    void syncSelection();
    void fitSelection();
    void applyFilter(const QString& text);

    QLabel* fileLabel_;
    QLineEdit* filter_;
    QTreeWidget* tree_;
    int32_t fileId_{-1};
};

} // namespace Gerber
