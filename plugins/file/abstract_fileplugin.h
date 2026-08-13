/********************************************************************************
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

#include "ft_view.h"
#include "plugindata.h"
// #include "settings.h"
// #include "app.h"
//

// #include "tool.h"

#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <any>
#include <map>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string_view>

class AbstractFile;

class AbstractFileSettings : public QWidget {
public:
    AbstractFileSettings(QWidget* parent)
        : QWidget{parent} {
    }
    virtual ~AbstractFileSettings() = default;
    virtual void readSettings(MySettings& settings) = 0;
    virtual void writeSettings(MySettings& settings) = 0;
};

class AbstractFilePlugin : public QObject, public PluginData {
    Q_OBJECT

public:
    explicit AbstractFilePlugin(QObject* parent = nullptr)
        : QObject{parent} { }
    virtual ~AbstractFilePlugin() = default;

    [[nodiscard]] virtual AbstractFileSettings* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };
    [[nodiscard]] virtual QString folderName() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;
    // Загрузка из JSON-проекта: json — сырой текст одного элемента "files"
    // ({"type":...,"base":{...},"data":{...}}); плагин парсит свой срез сам,
    // simdjson-типы через границу .so не гуляют.
    [[nodiscard]] virtual AbstractFile* loadFile([[maybe_unused]] std::string_view json) const { return nullptr; }
    // Стабильное имя типа, чей hash32 равен type(): "Gerber", "Drilling"...
    // (НЕ display-имя из description.json). Пишется в "type" JSON-проекта.
    [[nodiscard]] virtual std::string_view typeName() const { return {}; }
    [[nodiscard]] virtual bool thisIsIt(const QString& fileName) = 0;
    [[nodiscard]] virtual uint32_t type() const = 0;
    [[nodiscard]] virtual QString extension() const { return {}; } // = 0;

    virtual void createMainMenu(
        [[maybe_unused]] QMenu& menu,
        [[maybe_unused]] FileTree::View* tv) {
        menu.addAction(QIcon::fromTheme(u"document-close"_s), tr("&Close All Files"), [tv] {
            if(QMessageBox::question(tv, {}, tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                tv->closeFiles();
        });
    };

    virtual void updateFileModel([[maybe_unused]] AbstractFile* file) { };

    // Запросить отмену разбора. Зовётся из GUI-потока, разбор идёт в
    // parserThread -- отсюда мьютекс. Файл, ещё не дошедший до parseFileTask,
    // тоже отменяется: токен кладётся в реестр заранее и слот, добравшись до
    // очереди, сразу видит запрос и выходит, ничего не разбирая.
    void requestCancel(const QString& fileName);

signals:
    void fileError(const QString& fileName, const QString& error);
    void fileWarning([[maybe_unused]] const QString& fileName, [[maybe_unused]] const QString& warning);
    // fileName -- ПОЛНЫЙ путь: по нему окно прогресса находит свою строку, а
    // короткое имя не уникально (два Top.gbr из разных папок делили бы одну).
    void fileProgress(const QString& fileName, int max, int value);
    void fileReady(AbstractFile* file);
    void fileCanceled(const QString& fileName);

public slots:
    virtual AbstractFile* parseFile(const QString& fileName, uint32_t type) = 0;
    // Точка входа для загрузки: заводит область отмены вокруг parseFile и
    // ловит Geo::Cancelled. Разбор всегда запускать через неё, а не через
    // parseFile напрямую -- иначе отмены у файла не будет.
    void parseFileTask(const QString& fileName, uint32_t type);

private:
    std::map<QString, std::stop_source> tasks_;
    std::mutex tasksMutex_;
};

#define ParserInterface_iid "ru.xray3d.XrSoft.GGEasy.AbstractFilePlugin"

Q_DECLARE_INTERFACE(AbstractFilePlugin, ParserInterface_iid)
