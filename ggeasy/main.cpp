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
#include "abstract_fileplugin.h"
#include "gc_plugin.h"
#include "gc_types.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include "shapepluginin.h"
#include "stacktrace_and_logs.h"
#include "tool_database.h"
#include "version.h"

#include <QCommandLineParser>
#include <QDir>
#include <QPluginLoader>
#include <QStandardPaths>
#include <QSystemSemaphore>

int main(int argc, char* argv[]) {
    stacktraceAndOutput();

    QApplication::setAttribute(Qt::AA_Use96Dpi);

    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0"_ba);

    // qputenv("QT_QPA_PLATFORM", "windows:darkmode=1"_ba); // 1|2

    Q_INIT_RESOURCE(resources);

    QApplication app{argc, argv};

    // #ifdef Q_OS_WIN
    // QSettings settings{u"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"_s, QSettings::NativeFormat};
    // if (settings.value(u"AppsUseLightTheme"_s) == 0) {
    // qApp->setStyleSheet(u"QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"_s);
    // }
    // #endif

#ifdef Q_OS_UNIX
    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    // поэтому необходимо избавиться от данного мусора
    {
        QSharedMemory nixFixSharedMemory{u"AppSettings"_s};
        if(nixFixSharedMemory.attach())
            nixFixSharedMemory.detach();
    }
#endif
    QApplication::setApplicationName(u"GGEasy"_s);
    QApplication::setOrganizationName(VER_COMPANYNAME_STR);
    QApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    [[maybe_unused]] App appSingleton;
    [[maybe_unused]] GCode::Settings gcSingleton;
    App::setGcSettings(&gcSingleton);
    App::settingsPath() = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).constFirst();
    App::toolHolder().readTools();

    if(QDir dir(App::settingsPath()); !dir.exists())
        dir.mkpath(App::settingsPath());
    // QSettings::setDefaultFormat(QSettings::IniFormat);
    // QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, {});

    // WTF ??? QGLFormat glf = QGLFormat::defaultFormat();
    // glf.setSampleBuffers(true);
    // glf.setSamples(16);
    // QGLFormat::setDefaultFormat(glf);

    if constexpr(0) {
        QSystemSemaphore semaphore{u"GGEasySemaphore"_s, 1}; // создаём семафор
        semaphore.acquire();                                 // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
#ifdef Q_OS_UNIX
        // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
        // поэтому необходимо избавиться от данного мусора
        QSharedMemory nix_fix_shared_memory{u"GGEasyMemory"_s};
        if(nix_fix_shared_memory.attach())
            nix_fix_shared_memory.detach();
#endif
        // MainWindow* mainWin = nullptr;
        QSharedMemory sharedMemory{u"GGEasyMemory"_s}; // Создаём экземпляр разделяемой памяти
        auto instance = [&sharedMemory]() -> MainWindow*& { return *static_cast<MainWindow**>(sharedMemory.data()); };
        bool is_running{};          // переменную для проверки ууже запущенного приложения
        if(sharedMemory.attach()) { // пытаемся присоединить экземпляр разделяемой памяти к уже существующему сегменту
            is_running = true;      // Если успешно, то определяем, что уже есть запущенный экземпляр
        } else {
            sharedMemory.create(sizeof(void*)); // В противном случае выделяем размером с указатель кусок памяти   xxx1 байт памяти
            is_running = false;                 // И определяем, что других экземпляров не запущено
        }
        semaphore.release(); // Опускаем семафор
        QCommandLineParser parser;
        parser.addPositionalArgument(u"url"_s, u"Url of file to open"_s);
        parser.process(app);
        if(is_running) {
            system("pause");
            if(parser.positionalArguments().length())
                for(const QString& fileName: parser.positionalArguments())
                    instance()->loadFile(fileName);
            return 1;
        } else {
        }
    }

    { // Splash Screen
        auto splash = new QSplashScreen{QPixmap{u":/256.png"_s}};
        splash->setAttribute(Qt::WA_DeleteOnClose);
        splash->show();
        splash->connect(splash, &QObject::destroyed, splash, [] { App::setSplashScreen(nullptr); });
        App::setSplashScreen(splash);
    }

    { // Translate
        QSettings settings;
        settings.beginGroup(u"MainWindow"_s);
        QString locale(settings.value(u"locale"_s).toString());
        if(locale.isEmpty())
            locale = QLocale().name().left(2);
        settings.setValue(u"locale"_s, locale);
        settings.endGroup();
        MainWindow::translate(locale);
    }

    if(0) return ToolDatabase{}.exec();

    MainWindow mainWin;
    mainWin.setObjectName(u"MainWindow"_s);

    /*
    Platform        Valid suffixes
    Windows         .dll, .DLL
    Unix/Linux      .so
    AIX             .a
    HP-UX           .sl, .so (HP-UXi)
    macOS and iOS .dylib, .bundle, .so
    */
#ifdef Q_OS_UNIX
    #ifdef QT_DEBUG
    const QString suffix{u"*.so"_s};
    #else
    const QString suffix{u"*.so"_s};
    #endif
#elif defined(Q_OS_WIN)
    const auto suffix = u"*.dll"_s;
#else
    static_assert(false, u"Select OS"_s);
#endif

    std::vector<std::unique_ptr<QPluginLoader>> loaders;

    // load plugins
    QDir dir(QApplication::applicationDirPath() + u"/plugins"_s);
    if(dir.exists()) { // Поиск всех файлов в папке u"plugins"_s
        QStringList listFiles(dir.entryList(QStringList(suffix), QDir::Files));
        auto text = QObject::tr("Load plugin %1\n\n\n");
        for(const auto& str: listFiles) { // Проход по всем файлам
            App::splashScreen().showMessage(text.arg(str),
                Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
            loaders.emplace_back(std::make_unique<QPluginLoader>(dir.absolutePath() + u"/"_s + str));
            if(auto* pobj = loaders.back()->instance(); pobj) { // Загрузка плагина
                PluginData* pd{};
                if(auto* gCode = qobject_cast<GCode::Plugin*>(pobj)) {
                    pd = App::gCodePlugins()
                             .try_emplace(gCode->type(), gCode)
                             .first->second;
                } else if(auto* file = qobject_cast<AbstractFilePlugin*>(pobj)) {
                    pd = App::filePlugins()
                             .try_emplace(file->type(), file)
                             .first->second;
                } else if(auto* shape = qobject_cast<Shapes::Plugin*>(pobj)) {
                    pd = App::shapePlugins()
                             .try_emplace(shape->type(), shape)
                             .first->second;
                } else
                    continue;
                pd->setInfo(loaders.back()
                        ->metaData()
                        .value(u"MetaData"_s)
                        .toObject());
            } else {
                qDebug() << str << loaders.back()->errorString();
                loaders.pop_back();
            }
        }
    }

    SettingsDialog();
    MainWindow::updateTheme();
    mainWin.init(); // connect plugins

    QCommandLineParser parser;
    parser.addPositionalArgument(u"url"_s, u"Url of file to open"_s);
    parser.process(app);
    std::vector<std::string_view> cli{
        std::from_range, std::span{++argv, static_cast<size_t>(--argc)}
    };
    mainWin.cli(cli);

    for(const QString& fileName: parser.positionalArguments())
        mainWin.loadFile(fileName);

    mainWin.show();
    App::splashScreen().finish(&mainWin);

    int retCode = app.exec();

    for(auto&& loader: loaders) loader->unload();

    return retCode;
}
