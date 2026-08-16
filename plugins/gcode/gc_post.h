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

#include <QDateTime>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

namespace GCode {

// Постпроцессор -- диалект станка, описанный JS-файлом scripts/posts/<id>.js.
// Файл объявляет глобальный объект `post` (см. scripts/posts/README.md):
// имя, расширение, стиль комментариев, способ записи дуг, формат слов для
// линейных строк и для дуг, шапка/концовка, коды шпинделя/лазера.
//
// Диалект -- это ТОЛЬКО текст. Что и когда писать (G0 на безопасную высоту,
// врезание, переезд домой) решает C++/скрипт плагина; пост лишь говорит,
// какими словами.
class Post {
public:
    struct Info {
        QString id;   // имя файла без .js
        QString name; // post.name
    };

    enum class Arcs {
        IJ,     // G2/G3 X Y I J
        R,      // G2/G3 X Y R
        Linear, // хордами, G1
    };

    struct LineNumbers {
        bool enabled{};
        int start{10};
        int step{10};
    };

    // Контекст для функций поста: что известно о программе на момент вызова.
    struct Context {
        bool laser{};
        QString name;
        QString programName;
        int spindleSpeed{};
        double feedRate{};
        double plungeRate{};
        QJSValue tool;       // снимок инструмента (как file.tool в скрипте)
        QJSValue properties; // свойства проекта (как file.properties)
    };

    Post();
    ~Post();

    static QString postsDir();
    static std::vector<Info> available();

    // Загрузить пост по id. При ошибке -- предупреждение и дефолты generic.
    bool load(const QString& id);
    // Перечитать, если файл на диске изменился.
    void reloadIfChanged();

    const QString& id() const { return id_; }
    const QString& name() const { return name_; }
    const QString& description() const { return description_; }
    const QString& extension() const { return extension_; }
    bool parenComments() const { return parenComments_; }
    Arcs arcs() const { return arcs_; }
    double arcTolerance() const { return arcTolerance_; }
    const QString& formatLinear(bool laser) const { return laser ? laserLinear_ : millingLinear_; }
    const QString& formatArc(bool laser) const { return laser ? laserArc_ : millingArc_; }
    const LineNumbers& lineNumbers() const { return lineNumbers_; }

    QString comment(const QString& text) const;
    // Является ли строка комментарием в этом диалекте.
    bool isComment(const QString& line) const;

    // Свои движки у Post и у скрипта плагина, поэтому контекст собирается
    // на движке поста: ctx-объекты из чужого движка сюда не переносятся.
    QJSEngine& engine() { return *engine_; }

    QStringList header(const Context& ctx);
    QStringList footer(const Context& ctx);
    QString spindleOn(const Context& ctx);
    QString spindleOff(const Context& ctx);
    QString laserOn(const Context& ctx, bool dynamic);
    QString laserOff(const Context& ctx);

    // Итоговый текст файла: строки + нумерация N по посту.
    QString renderText(const std::vector<QString>& lines) const;

private:
    void resetDefaults();
    void readFields();
    QJSValue makeCtx(const Context& ctx);
    QStringList callLines(const QString& field, const QJSValue& ctx, const QJSValue& extra = {});

    std::unique_ptr<QJSEngine> engine_;
    QJSValue post_;
    QString id_, path_;
    QDateTime mtime_;

    QString name_, description_, extension_;
    bool parenComments_{};
    Arcs arcs_{Arcs::IJ};
    double arcTolerance_{0.01};
    QString millingLinear_, millingArc_, laserLinear_, laserArc_;
    LineNumbers lineNumbers_;
};

} // namespace GCode
