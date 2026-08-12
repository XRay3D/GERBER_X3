/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                             *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "openglcheck.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <optional>
#include <string_view>

using namespace Qt::Literals;

namespace OpenGlCheck {

namespace {

constexpr auto settingsKey = "Viewer/openGlMode";
constexpr auto markerName = "opengl-startup.lock";

// Свойства QCoreApplication -- единственное состояние, общее для exe и плагинов.
constexpr auto propMode = "ggeasy_opengl_mode";
constexpr auto propAvailable = "ggeasy_opengl_available";
constexpr auto propReport = "ggeasy_opengl_report";
constexpr auto propHint = "ggeasy_opengl_hint";

Mode mode_{Mode::Auto};
bool available_{};
bool probed_{};
QString markerPath_;
QString driverInfo_;
QString reason_;

QString toString(Mode m) {
    switch(m) {
    case Mode::Auto    : return u"auto"_s;
    case Mode::Desktop : return u"desktop"_s;
    case Mode::Software: return u"software"_s;
    case Mode::Off     : return u"off"_s;
    }
    return u"auto"_s;
}

std::optional<Mode> fromString(QStringView s) {
    const auto v = s.trimmed().toString().toLower();
    if(v == u"auto"_s) return Mode::Auto;
    if(v == u"desktop"_s || v == u"hardware"_s) return Mode::Desktop;
    if(v == u"software"_s || v == u"sw"_s) return Mode::Software;
    if(v == u"off"_s || v == u"none"_s || v == u"disabled"_s) return Mode::Off;
    return {};
}

// Порядок понижения: не завелось железо -- пробуем программный GL, не завёлся
// и он -- обходимся без GL совсем. Если рухнул даже режим без GL, виновата не
// видеокарта: возвращаемся к Auto, чтобы не сидеть на растре вечно.
Mode degrade(Mode attempted) {
    switch(attempted) {
    case Mode::Auto:
    case Mode::Desktop : return Mode::Software;
    case Mode::Software: return Mode::Off;
    case Mode::Off     : return Mode::Auto;
    }
    return Mode::Software;
}

std::optional<Mode> explicitMode(int argc, char** argv) {
    if(qEnvironmentVariableIsSet("GGEASY_OPENGL"))
        if(auto m = fromString(qEnvironmentVariable("GGEASY_OPENGL")))
            return m;

    QStringList args;
    args.reserve(argc);
    for(int i{}; i < argc; ++i)
        args << QString::fromLocal8Bit(argv[i]);

    const auto option = commandLineOption();
    QCommandLineParser parser;
    parser.addOption(option);
    // parse(), а не process(): QApplication ещё нет, а до остальных ключей и
    // до имён файлов нам дела нет -- parse() сложит их в unknownOptionNames,
    // разберёт что знает и вернёт управление, ничего не печатая и не выходя.
    parser.parse(args);
    if(parser.isSet(option))
        return fromString(parser.value(option));
    return {};
}

} // namespace

QCommandLineOption commandLineOption() {
    return {u"opengl"_s, u"Graphics backend: auto|desktop|software|off"_s, u"mode"_s};
}

void selectMode(int argc, char** argv) {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    markerPath_ = dir + u'/' + QString::fromLatin1(markerName);

    QSettings settings;
    Mode m = fromString(settings.value(QString::fromLatin1(settingsKey)).toString()).value_or(Mode::Auto);

    if(const auto forced = explicitMode(argc, argv)) {
        // Заданный руками режим запоминаем: чтобы разово подобранный рабочий
        // вариант не приходилось указывать при каждом запуске, а --opengl=auto
        // сбрасывал накопленную деградацию.
        m = *forced;
        reason_ = u"задан явно (GGEASY_OPENGL / --opengl)"_s;
        settings.setValue(QString::fromLatin1(settingsKey), toString(m));
    } else if(QFile marker{markerPath_}; marker.exists()) {
        Mode attempted = m;
        if(marker.open(QIODevice::ReadOnly)) {
            if(auto parsed = fromString(QString::fromLatin1(marker.readAll().trimmed())))
                attempted = *parsed;
            marker.close();
        }
        m = degrade(attempted);
        reason_ = u"прошлый запуск не пережил инициализацию графики (режим %1), понижено до %2"_s
                      .arg(toString(attempted), toString(m));
        settings.setValue(QString::fromLatin1(settingsKey), toString(m));
    } else if(m != Mode::Auto) {
        reason_ = u"сохранён с прошлых запусков"_s;
    }

    mode_ = m;

    switch(mode_) {
    case Mode::Software:
        qputenv("QT_OPENGL", "software");
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
        break;
    case Mode::Desktop:
        qputenv("QT_OPENGL", "desktop");
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
        break;
    case Mode::Auto:
    case Mode::Off:
        break;
    }

    // Маркер кладём до первого обращения к драйверу: если он утащит процесс за
    // собой, следующий запуск об этом узнает.
    QDir{}.mkpath(dir);
    if(QFile marker{markerPath_}; marker.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        marker.write(toString(mode_).toLatin1());
        marker.close();
    }
}

static void probeImpl() {
    probed_ = true;

    if(mode_ == Mode::Off) {
        available_ = false;
        if(reason_.isEmpty()) reason_ = u"выключен режимом off"_s;
        return;
    }

    // Headless-плагины QPA закадровый контекст создают исправно, а вот
    // QOpenGLWidget в них не работает: Qt ругается в лог и рисует мимо.
    if(const auto platform = QGuiApplication::platformName();
        platform == u"offscreen"_s || platform == u"minimal"_s) {
        available_ = false;
        reason_ = u"платформа %1 не поддерживает QOpenGLWidget"_s.arg(platform);
        return;
    }

    QOpenGLContext ctx;
    if(!ctx.create()) {
        available_ = false;
        reason_ = u"драйвер не создал контекст OpenGL"_s;
        return;
    }

    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    if(!surface.isValid() || !ctx.makeCurrent(&surface)) {
        available_ = false;
        reason_ = u"контекст OpenGL не удалось сделать текущим"_s;
        return;
    }

    if(auto* f = ctx.functions()) {
        f->initializeOpenGLFunctions();
        auto str = [f](GLenum name) {
            const auto* s = reinterpret_cast<const char*>(f->glGetString(name));
            return s ? QString::fromLatin1(s) : u"?"_s;
        };
        driverInfo_ = u"%1 / %2 / %3"_s.arg(str(GL_VENDOR), str(GL_RENDERER), str(GL_VERSION));
    }

    const auto version = ctx.format().version();
    ctx.doneCurrent();

    // QOpenGLWidget как viewport для QGraphicsView требует хотя бы GL 2.0.
    if(version.first < 2) {
        available_ = false;
        reason_ = u"драйвер отдаёт OpenGL %1.%2, нужно не ниже 2.0"_s.arg(version.first).arg(version.second);
        return;
    }

    available_ = true;
}

static QString composeReport() {
    QStringList parts{u"режим: "_s + toString(mode_)};
    if(!driverInfo_.isEmpty()) parts << driverInfo_;
    if(!available_) parts << (probed_ ? u"аппаратное ускорение выключено"_s : u"проверка не выполнялась"_s);
    if(!reason_.isEmpty()) parts << reason_;
    return parts.join(u"; "_s);
}

static QString composeHint() {
    // Software -- это тоже "ускорения нет": картинку считает процессор.
    if(available_ && mode_ != Mode::Software) return {};
    return u"Аппаратное ускорение 3D не используется (%1).\n"
           "Включить: запустить с ключом --opengl=desktop или с переменной "
           "окружения GGEASY_OPENGL=desktop.\n"
           "Режим запоминается между запусками, --opengl=auto возвращает "
           "автоопределение. Если с ускорением приложение не запустится, "
           "режим понизится сам."_s
               .arg(composeReport());
}

// Итог проверки кладём в свойства QCoreApplication, а читаем только оттуда.
//
// Иначе не работает: common линкуется статически и в GGEasy, и в каждый плагин,
// поэтому у плагина своя копия статики этого файла -- probe() отработал в exe,
// а плагин видел бы "проверка не выполнялась" и прятал 3D-панель навсегда.
// Экземпляр QCoreApplication живёт в разделяемой QtCore и на процесс он один,
// на любой платформе -- в отличие от фокусов компоновщика вроде STB_GNU_UNIQUE,
// которых в PE под Windows попросту нет.
static void publish() {
    auto* app = QCoreApplication::instance();
    if(!app) return;
    app->setProperty(propMode, toString(mode_));
    app->setProperty(propAvailable, available_);
    app->setProperty(propReport, composeReport());
    app->setProperty(propHint, composeHint());
}

void probe() {
    probeImpl();
    publish();
    // Один отчёт на всю проверку: подробности из веток probeImpl уже в reason_.
    if(available_)
        qInfo().noquote() << "OpenGL:" << composeReport();
    else
        qWarning().noquote() << "OpenGL:" << composeReport();
}

bool available() {
    auto* app = QCoreApplication::instance();
    return app && app->property(propAvailable).toBool();
}

Mode mode() {
    if(auto* app = QCoreApplication::instance())
        if(auto m = fromString(app->property(propMode).toString())) return *m;
    return mode_;
}

QString report() {
    auto* app = QCoreApplication::instance();
    return app ? app->property(propReport).toString() : composeReport();
}

QString howToEnable() {
    auto* app = QCoreApplication::instance();
    return app ? app->property(propHint).toString() : composeHint();
}

void logStatus() {
    if(const auto hint = howToEnable(); !hint.isEmpty())
        qWarning().noquote() << hint;
    else
        qInfo().noquote() << u"OpenGL: "_s + report();
}

void markStartupSucceeded() {
    if(markerPath_.isEmpty()) return;
    QFile::remove(markerPath_);
}

} // namespace OpenGlCheck
