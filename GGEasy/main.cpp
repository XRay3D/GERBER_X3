// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#include "mainwindow.h"
#include "messageoutput.h"
#include "settingsdialog.h"
#include "splashscreen.h"
#include "version.h"

#include "ctre.hpp"
#include "interfaces/shapepluginin.h"
//#include "style.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLWidget>
#endif

#include <QCommandLineParser>
#include <QDir>
#include <QPluginLoader>
#include <QSystemSemaphore>

#include "style.h"

void translation(QApplication* app);

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#include "leakdetector.h"

#include <gcode/gcplugin.h>

int main(int argc, char** argv) {

    //QApplication::setStyle(new MyStyle);
    //    QString s1("FILE_FORMAT=3:3");
    //    QString s2("file_format=3:3");
    //    static constexpr ctll::fixed_string regexFormat_(R"(.*(?:FORMAT|format).*(\\d{1}).(\\d{1}))");// fixed_string(".*(?:FORMAT|format).*(\\d{1}).(\\d{1})");
    //    if (auto [matchFormat, integer, decimal] = ctre::match<regexFormat>(s1); matchFormat) {
    //        qDebug() << "regexFormat" << matchFormat.toString();
    //        qDebug() << "regexFormat" << integer.toString();
    //        qDebug() << "regexFormat" << decimal.toString();
    //    }
    //    if (auto [matchFormat, integer, decimal] = ctre::match<regexFormat>(s2); matchFormat) {
    //        qDebug() << "regexFormat" << matchFormat.toString();
    //        qDebug() << "regexFormat" << integer.toString();
    //        qDebug() << "regexFormat" << decimal.toString();
    //    }

    //    return 0;

    //#if defined(Q_OS_WIN) && !defined(__GNUC__)
    //    HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
    //    DWORD dwMode = 0;
    //    GetConsoleMode(hOut, &dwMode);
    //    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    //    SetConsoleMode(hOut, dwMode);
    //#endif

    //    qInstallMessageHandler(myMessageOutput);

#ifdef LEAK_DETECTOR
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
//QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
//QApplication::setAttribute(Qt::AA_Use96Dpi);
#else
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif

    Q_INIT_RESOURCE(resources);
    QApplication::setApplicationName("GGEasy");
    QApplication::setOrganizationName("settings" /*VER_COMPANYNAME_STR*/);
    QApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    QApplication app(argc, argv);

//#ifdef Q_OS_WIN
//    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
//    if (settings.value("AppsUseLightTheme") == 0) {

//        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
//    }
//#endif

//    QApplication::setStyle(new ProxyStyle(QApplication::style()));
#ifdef linux
    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    // поэтому необходимо избавиться от данного мусора
    QSharedMemory nixFixSharedMemory("AppSettings");
    if(nixFixSharedMemory.attach())
        nixFixSharedMemory.detach();
#endif

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());
    App a;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(16);
    QGLFormat::setDefaultFormat(glf);
#endif

    if constexpr(0) {
        QSystemSemaphore semaphore("GGEasySemaphore", 1); // создаём семафор
        semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
#ifdef linux
        // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
        // поэтому необходимо избавиться от данного мусора
        QSharedMemory nix_fix_shared_memory("GGEasy_Memory");
        if(nix_fix_shared_memory.attach()) {
            nix_fix_shared_memory.detach();
        }
#endif
        MainWindow* mainWin = nullptr;
        QSharedMemory sharedMemory("GGEasy_Memory"); // Создаём экземпляр разделяемой памяти
        auto instance = [&sharedMemory]() -> MainWindow*& { return *static_cast<MainWindow**>(sharedMemory.data()); };
        bool is_running = false; // переменную для проверки ууже запущенного приложения
        if(sharedMemory.attach()) { // пытаемся присоединить экземпляр разделяемой памяти к уже существующему сегменту
            is_running = true; // Если успешно, то определяем, что уже есть запущенный экземпляр
        } else {
            sharedMemory.create(sizeof(mainWin)); // В противном случае выделяем 1 байт памяти
            is_running = false; // И определяем, что других экземпляров не запущено
        }
        semaphore.release(); // Опускаем семафор
        QCommandLineParser parser;
        parser.addPositionalArgument("url", "Url of file to open");
        parser.process(app);
        if(is_running) {
            system("pause");
            if(parser.positionalArguments().length()) {
                for(const QString& fileName : parser.positionalArguments()) {
                    instance()->loadFile(fileName);
                }
            }
            return 1;
        } else {
        }
    }

    SplashScreen* splash = new SplashScreen(QPixmap(QLatin1String(":/256.png")));
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    {
        /*
        Platform        Valid suffixes
        Windows         .dll, .DLL
        Unix/Linux      .so
        AIX             .a
        HP-UX           .sl, .so (HP-UXi)
        macOS and iOS	.dylib, .bundle, .so
        */
#ifdef __unix__
#ifdef QT_DEBUG
        const QString suffix("*.so");
#else
        const QString suffix("*.so");
#endif
#elif _WIN32
        const QString suffix("*.dll");
#else
        static_assert(false, "Select OS");
#endif
        // load plugins
        QDir dir(QApplication::applicationDirPath() + "/plugins");
        if(dir.exists()) { // Поиск всех файлов в папке "plugins"
            QStringList listFiles(dir.entryList(QStringList(suffix), QDir::Files));
            for(const auto& str : listFiles) { // Проход по всем файлам
                splash->showMessage(QObject::tr("Load plugin %1\n\n\n").arg(str), Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
                QPluginLoader loader(dir.absolutePath() + "/" + str);
                QObject* pobj = loader.instance(); // Загрузка плагина
                if /**/ (auto file = qobject_cast<FilePluginInterface*>(pobj); pobj && file) {
                    App::filePlugins().emplace(file->type(), PIF {file, pobj});
                } else if(auto shape = qobject_cast<ShapePluginInterface*>(pobj); pobj && shape) {
                    App::shapePlugins().emplace(shape->type(), PIS {shape, pobj});
                }
            }
        }
        { // add dummy gcode plugin
            auto parser = new GCode::Plugin(&app);
            PIF pi {
                static_cast<FilePluginInterface*>(parser),
                static_cast<QObject*>(parser)};
            App::filePlugins().emplace(parser->type(), pi);
        }

        QSettings settings;
        settings.beginGroup("MainWindow");
        QString locale(settings.value("locale").toString());
        if(locale.isEmpty())
            locale = QLocale().name().left(2);
        settings.setValue("locale", locale);
        settings.endGroup();
        MainWindow::translate(locale);
    }

    SettingsDialog().accept();

    MainWindow mainWin;
    mainWin.setObjectName("MainWindow");
    mainWin.setIconSize({24, 24});

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Url of file to open");
    parser.process(app);
    for(const QString& fileName : parser.positionalArguments()) {
        mainWin.loadFile(fileName);
    }

    mainWin.show();
    splash->finish(&mainWin);

    return app.exec();
}
