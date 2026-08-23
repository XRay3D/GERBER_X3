/********************************************************************************4
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "ggcore_export.h"

#include "depthform.h"
#include "gcode.h"

#include <QButtonGroup>
#include <QtWidgets>
#include <thread>

namespace GCode {

class File;

class GGCORE_EXPORT Form : public QWidget {
    Q_OBJECT

public:
    explicit Form(Plugin* plugin, Creator* tpc);
    ~Form() override;

    // Заполнить форму параметрами уже созданной УП и вернуть сцену в то
    // состояние, из которого её считали. Базовая реализация делает всё, что
    // живёт в самой базе: имя, глубину, сторону, выделение и видимость.
    // Плагину остаётся восстановить только свои виджеты -- инструмент,
    // переключатели, галочки, -- вызвав сначала эту.
    virtual void editFile(File* file);

    Creator* creator() const { return creator_; }

    void setCreator(Creator* newCreator);

signals:
    void createToolpath(Params* gcp);

protected:
    virtual void fileHandler(File* file);
    // Плагины с собственной независимой моделью "что сейчас правится" (Drilling:
    // свои карты toolId->fileId по каждой ветке вместо единого fileId/editMode_ --
    // один клик там может провернуть несколько независимых программ, а fileId в
    // базе один) переопределяют это в false, чтобы fileHandler() не совал туда
    // чужой fileId после свежего прогона.
    virtual bool autoEnterEditMode() const { return true; }
    void updateButtonIconSize() {
        for(auto* button: findChildren<QPushButton*>())
            button->setIconSize({16, 16});
    }
    void addUsedGi(class ::Gi::Item* gi);
    QWidget* widget() const { return ctrWidget; }

    // Уже существующая в проекте УП с таким именем программы, кроме exceptId.
    File* findProgram(const QString& programName, int32_t exceptId = -1) const;
    // Все УП с таким именем программы. Их может быть несколько: многоинструментные
    // плагины (PocketOffset) делают из одного запуска по файлу на инструмент.
    std::vector<int32_t> programIds(const QString& programName) const;
    // "Profile" -> "Profile (2)"; хвостовой номер наращивается, а не вкладывается.
    QString uniqueProgramName(QString name) const;
    // Спрашивает пользователя про коллизию имени и переименование при правке.
    // Возвращает false, если тот отказался считать вовсе.
    bool resolveFileName();
    // Вернуть сцену в состояние, из которого УП считали: видимость файлов по
    // снимку и выделение исходных элементов по Params::GrItems.
    void restoreContext(File* file);
    // Режим правки существующей УП. Меняет заодно надпись на кнопке -- иначе
    // «Create» врёт: файл будет перезаписан, а не создан.
    void setEditMode(bool on);

    // QObject interface
    virtual void timerEvent(QTimerEvent* event) override;
    // QWidget interface
    void showEvent(QShowEvent* event) override;
    // Form interface
    virtual void computePaths() = 0;
    virtual void updateName() = 0;

    const Params& getNewGcpWithGi();

    Direction direction = Climb;
    SideOfMilling side = Outer;
    UsedItems usedItems_;
    Side boardSide = Top;

    // Снимок видимости файлов проекта на момент нажатия Create. Уезжает в
    // созданную УП, чтобы её открытие на правку вернуло ту же картину.
    std::map<int32_t, bool> visibility_;

    // УП, которые текущий прогон должен заменить собой. Список, а не один id:
    // один запуск может дать несколько файлов, и прежних могло быть другое
    // число. Лишние удаляются в конце прогона, недостающие просто добавляются.
    std::vector<int32_t> replacedIds_;

    QString fileName_;

    QString trOutside{tr("Outside")};
    QString trDepth{tr("Depth:")};
    QString trTool{tr("Tool:")};

    bool editMode_{};
    int fileId{-1};

    int fileCount{1};
    Plugin* const plugin;

    DepthForm* dsbxDepth;
    // QLabel* label;
    QLineEdit* leName;
    QPushButton* pbClose;
    QPushButton* pbCreate;
    QWidget* content;
    QGridLayout* grid;
    Params gcp;

private:
    Creator* creator_{nullptr};

    QDialogButtonBox* errBtnBox;
    class TableView* errTable;

    QWidget* ctrWidget;
    QWidget* errWidget;

    void errContinue();
    void errBreak();

    void cancel();
    void errorHandler(int = 0);

    void startProgress();
    void stopProgress();
    // Форма на время счёта и обратно.
    void setComputing(bool on);

    // Прогон отменён пользователем. Расчёт узнаёт об этом не мгновенно, и всё,
    // что он успеет прислать следом, форма выбрасывает.
    bool canceled_{};
    bool computing_{};
    // Кнопку запуска плагины гасят по своим условиям -- на время счёта она
    // нужна включённой (это «Cancel»), потом состояние возвращается.
    bool createEnabled_{true};

    // Расчёт идёт ОТДЕЛЬНЫМ потоком, и иначе нельзя: считает он секунды и
    // минуты, а прогрессбар с отменой -- обычные виджеты, и живут они ровно
    // тем, что GUI-поток свободен. Прямой вызов (он тут и был) не только
    // морозил окно на всё время счёта, но и намертво вешал программу на
    // найденных ошибках: Creator::isContinueCalc ждёт ответа «Continue/Break»
    // на условной переменной, а нажать было некому.
    //
    // jthread выбран за то, ради чего он и сделан: у него есть стоп-токен --
    // тот самый, которым отменяются вычисления Geo, -- а деструктор сам
    // просит остановиться и дожидается. Отдельный QThread ради этого не
    // нужен: Creator ничьих сигналов не ПРИНИМАЕТ, а его собственные Qt
    // доставит в GUI-поток очередью, откуда бы их ни послали.
    //
    // Сцены и дерева проекта расчёт не касается вовсе -- готовый File уезжает
    // в GUI-поток сигналом fileReady, и уже там ему строят Gi и добавляют в
    // проект.
    std::jthread worker;

    // Запустить расчёт (слот сигнала createToolpath) и дождаться его конца.
    void startCompute(Params* gcp);
    void stopWorker();

    class QProgressBar* progressBar;
    int progressTimerId{};
};

} // namespace GCode
