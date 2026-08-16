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
#include "gc_post.h"
#include "gc_types.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace GCode {

namespace {

// Одноразовая загрузка файла поста в движок: глобальный объект `post`.
QJSValue evalPost(QJSEngine& engine, const QString& path) {
    QFile f{path};
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "GCode post: cannot open" << path;
        return {};
    }
    auto result = engine.evaluate(QString::fromUtf8(f.readAll()), path);
    if(result.isError()) {
        qWarning() << "GCode post error in" << path
                   << "line" << result.property(u"lineNumber"_s).toInt()
                   << ":" << result.toString();
        return {};
    }
    auto post = engine.globalObject().property(u"post"_s);
    if(!post.isObject()) {
        qWarning() << "GCode post:" << path << "does not define object `post`";
        return {};
    }
    return post;
}

QString strOr(const QJSValue& obj, const QString& key, const QString& def) {
    auto v = obj.property(key);
    return v.isString() ? v.toString() : def;
}

} // namespace

Post::Post()
    : engine_{std::make_unique<QJSEngine>()} {
    engine_->installExtensions(QJSEngine::ConsoleExtension);
    resetDefaults();
}

Post::~Post() = default;

QString Post::postsDir() {
    return QCoreApplication::applicationDirPath() + u"/scripts/posts"_s;
}

std::vector<Post::Info> Post::available() {
    std::vector<Info> list;
    QDir dir{postsDir()};
    for(const QString& fileName: dir.entryList({u"*.js"_s}, QDir::Files, QDir::Name)) {
        Info info{QFileInfo{fileName}.completeBaseName(), {}};
        QJSEngine engine;
        auto post = evalPost(engine, dir.filePath(fileName));
        info.name = strOr(post, u"name"_s, info.id);
        list.push_back(std::move(info));
    }
    // generic -- первым: он же дефолт.
    if(auto it = std::ranges::find(list, u"generic"_s, &Info::id); it != list.end())
        std::rotate(list.begin(), it, it + 1);
    return list;
}

void Post::resetDefaults() {
    // Ровно то, что было в полях настроек до появления постов.
    name_ = u"Generic ISO"_s;
    description_.clear();
    extension_ = u"tap"_s;
    parenComments_ = false;
    arcs_ = Arcs::IJ;
    arcTolerance_ = 0.01;
    millingLinear_ = u"G?X?Y?Z?F?S?"_s;
    millingArc_ = u"G?X?Y?I+J+Z?F?S?"_s;
    laserLinear_ = millingLinear_;
    laserArc_ = millingArc_;
    lineNumbers_ = {};
    post_ = QJSValue{};
}

bool Post::load(const QString& id) {
    id_ = id;
    path_ = postsDir() + u'/' + id + u".js"_s;
    resetDefaults();
    mtime_ = QFileInfo{path_}.lastModified();

    // Свежий движок: старые глобалы предыдущего поста не должны просачиваться.
    engine_ = std::make_unique<QJSEngine>();
    engine_->installExtensions(QJSEngine::ConsoleExtension);
    post_ = evalPost(*engine_, path_);
    if(!post_.isObject()) {
        qWarning() << "GCode post: falling back to built-in generic defaults";
        return false;
    }
    readFields();
    return true;
}

void Post::reloadIfChanged() {
    if(path_.isEmpty()) return;
    if(QFileInfo{path_}.lastModified() != mtime_)
        load(id_);
}

void Post::readFields() {
    name_ = strOr(post_, u"name"_s, id_);
    description_ = strOr(post_, u"description"_s, {});
    extension_ = strOr(post_, u"extension"_s, extension_);
    parenComments_ = strOr(post_, u"comment"_s, u";"_s).startsWith(u'(');

    const QString arcs = strOr(post_, u"arcs"_s, u"ij"_s).toLower();
    arcs_ = arcs == u"r"_s ? Arcs::R : arcs == u"linear"_s ? Arcs::Linear
                                                           : Arcs::IJ;
    if(auto v = post_.property(u"arcTolerance"_s); v.isNumber() && v.toNumber() > 0.0)
        arcTolerance_ = v.toNumber();

    auto format = post_.property(u"format"_s);
    auto readPair = [&format](const QString& key, QString& linear, QString& arc) {
        auto obj = format.property(key);
        if(obj.isString()) { // одна строка на всё -- как в старых настройках
            linear = arc = obj.toString();
            return;
        }
        if(!obj.isObject()) return;
        linear = strOr(obj, u"linear"_s, linear);
        arc = strOr(obj, u"arc"_s, arc);
    };
    if(format.isObject()) {
        readPair(u"milling"_s, millingLinear_, millingArc_);
        laserLinear_ = millingLinear_, laserArc_ = millingArc_; // лазер наследует фрезу
        readPair(u"laser"_s, laserLinear_, laserArc_);
    }

    auto ln = post_.property(u"lineNumbers"_s);
    if(ln.isObject()) {
        lineNumbers_.enabled = true;
        if(auto v = ln.property(u"start"_s); v.isNumber()) lineNumbers_.start = v.toInt();
        if(auto v = ln.property(u"step"_s); v.isNumber()) lineNumbers_.step = std::max(1, v.toInt());
    } else
        lineNumbers_.enabled = ln.toBool();
}

QString Post::comment(const QString& text) const {
    if(parenComments_) {
        QString safe = text;
        safe.replace(u'(', u'[').replace(u')', u']');
        return u'(' + safe + u')';
    }
    return u';' + text;
}

bool Post::isComment(const QString& line) const {
    return line.startsWith(parenComments_ ? u'(' : u';');
}

QJSValue Post::makeCtx(const Context& ctx) {
    QJSValue obj = engine_->newObject();
    obj.setProperty(u"laser"_s, ctx.laser);
    obj.setProperty(u"name"_s, ctx.name);
    obj.setProperty(u"programName"_s, ctx.programName);
    obj.setProperty(u"spindleSpeed"_s, ctx.spindleSpeed);
    obj.setProperty(u"feedRate"_s, ctx.feedRate);
    obj.setProperty(u"plungeRate"_s, ctx.plungeRate);
    obj.setProperty(u"tool"_s, ctx.tool);
    obj.setProperty(u"properties"_s, ctx.properties);
    return obj;
}

// Поле поста может быть строкой, массивом строк или функцией(ctx, extra),
// возвращающей строку либо массив строк. Пустое поле -- пусто.
QStringList Post::callLines(const QString& field, const QJSValue& ctx, const QJSValue& extra) {
    if(!post_.isObject()) return {};
    QJSValue value = post_.property(field);
    if(value.isCallable()) {
        QJSValueList args{ctx};
        if(!extra.isUndefined()) args.push_back(extra);
        value = value.call(args);
        if(value.isError()) {
            qWarning() << "GCode post" << id_ << field << "error line"
                       << value.property(u"lineNumber"_s).toInt() << ":" << value.toString();
            return {};
        }
    }
    QStringList lines;
    if(value.isArray()) {
        const int len = value.property(u"length"_s).toInt();
        for(int i{}; i < len; ++i)
            lines << value.property(i).toString();
    } else if(value.isString())
        lines << value.toString().split(u'\n');
    return lines;
}

QStringList Post::header(const Context& ctx) {
    if(!post_.isObject()) // дефолт generic
        return ctx.laser ? QStringList{u"G21 G17 G90"_s}
                         : QStringList{u"G21 G17 G90"_s, u"M3 S"_s + QString::number(ctx.spindleSpeed)};
    return callLines(u"header"_s, makeCtx(ctx));
}

QStringList Post::footer(const Context& ctx) {
    if(!post_.isObject())
        return ctx.laser ? QStringList{u"M30"_s} : QStringList{u"M5"_s, u"M30"_s};
    return callLines(u"footer"_s, makeCtx(ctx));
}

QString Post::spindleOn(const Context& ctx) {
    if(!post_.isObject()) return u"M3 S"_s + QString::number(ctx.spindleSpeed);
    return callLines(u"spindleOn"_s, makeCtx(ctx)).join(u'\n');
}

QString Post::spindleOff(const Context& ctx) {
    if(!post_.isObject()) return u"M5"_s;
    return callLines(u"spindleOff"_s, makeCtx(ctx)).join(u'\n');
}

QString Post::laserOn(const Context& ctx, bool dynamic) {
    if(!post_.isObject()) return dynamic ? u"M4"_s : u"M3"_s;
    return callLines(u"laserOn"_s, makeCtx(ctx), QJSValue{dynamic}).join(u'\n');
}

QString Post::laserOff(const Context& ctx) {
    if(!post_.isObject()) return u"M5"_s;
    return callLines(u"laserOff"_s, makeCtx(ctx)).join(u'\n');
}

QString Post::renderText(const std::vector<QString>& lines) const {
    QString str;
    int n = lineNumbers_.start;
    for(const QString& line: lines) {
        if(line.isEmpty()) continue;
        // Строки, которые нумеровать нельзя: '%', комментарии и уже
        // пронумерованные постом (N99999 у Heidenhain).
        const bool numbered = line.size() > 1 && line[0].toUpper() == u'N' && line[1].isDigit();
        if(lineNumbers_.enabled && !line.startsWith(u'%') && !numbered && !isComment(line)) {
            str += u'N' + QString::number(n) + u' ';
            n += lineNumbers_.step;
        }
        str += line;
        if(!str.endsWith(u'\n')) str += u'\n';
    }
    return str;
}

Post& Settings::post() {
    if(!post_) {
        post_ = std::make_unique<Post>();
        post_->load(postId_);
    } else if(post_->id() != postId_)
        post_->load(postId_);
    else
        post_->reloadIfChanged();
    return *post_;
}

} // namespace GCode
