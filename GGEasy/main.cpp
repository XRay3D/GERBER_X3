// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#include "mainwindow.h"
#include "splashscreen.h"
#include "version.h"

#include "interfaces/shapepluginin.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLWidget>
#endif
#include <QDir>
#include <QElapsedTimer>
#include <QPluginLoader>
#include <QProxyStyle>
#include <QSettings>
#include <QSystemSemaphore>
#include <QTranslator>

#include "leakdetector.h"

class ProxyStyle : public QProxyStyle {
    //Q_OBJECT

public:
    ProxyStyle(QStyle* style = nullptr)
        : QProxyStyle(style)
    {
    }

    ProxyStyle(const QString& key)
        : QProxyStyle(key)
    {
    }

    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const override
    {
        //        switch (metric) {
        //        case QStyle::PM_SmallIconSize:
        //            return 22;
        //        default:
        //            return QProxyStyle::pixelMetric(metric, option, widget);
        //        }
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

void initIcon(const QString& path);
void translation(QApplication* app);

int main(int argc, char* argv[])
{

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

    // QApplication::setStyle(new ProxyStyle(QApplication::style()));

    App a;

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(16);
    QGLFormat::setDefaultFormat(glf);
#endif
    initIcon(qApp->applicationDirPath());

    if constexpr (0) {
        QSystemSemaphore semaphore("GGEasySemaphore", 1); // создаём семафор
        semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
#ifdef linux
        // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
        // поэтому необходимо избавиться от данного мусора
        QSharedMemory nix_fix_shared_memory("GGEasy_Memory");
        if (nix_fix_shared_memory.attach()) {
            nix_fix_shared_memory.detach();
        }
#endif
        MainWindow* mainWin = nullptr;
        QSharedMemory sharedMemory("GGEasy_Memory"); // Создаём экземпляр разделяемой памяти
        auto instance = [&sharedMemory]() -> MainWindow*& { return *static_cast<MainWindow**>(sharedMemory.data()); };
        bool is_running = false; // переменную для проверки ууже запущенного приложения
        if (sharedMemory.attach()) { // пытаемся присоединить экземпляр разделяемой памяти к уже существующему сегменту
            is_running = true; // Если успешно, то определяем, что уже есть запущенный экземпляр
        } else {
            sharedMemory.create(sizeof(mainWin)); // В противном случае выделяем 1 байт памяти
            is_running = false; // И определяем, что других экземпляров не запущено
        }
        semaphore.release(); // Опускаем семафор
        QCommandLineParser parser;
        parser.addPositionalArgument("url", "Url of file to open");
        parser.process(app);
        if (is_running) {
            system("pause");
            if (parser.positionalArguments().length()) {
                for (const QString& fileName : parser.positionalArguments()) {
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
        listFiles = dir.entryList(QStringList("*d.so"), QDir::Files);
#else
        listFiles = dir.entryList(QStringList("*.so"), QDir::Files);
#endif
#elif _WIN32
        const QString suffix("*.dll");
#else
        static_assert(false, "Select OS");
#endif
        // load plugins
        QDir dir(QApplication::applicationDirPath() + "/plugins");
        if (dir.exists()) { // Поиск всех файлов в папке "plugins"
            QStringList listFiles(dir.entryList(QStringList(suffix), QDir::Files));
            QElapsedTimer t;
            for (const auto& str : listFiles) { // Проход по всем файлам
                splash->showMessage(QObject::tr("Load plugin %1\n\n\n").arg(str), Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
                t.start();
                QPluginLoader loader(dir.absolutePath() + "/" + str);
                QObject* pobj = loader.instance(); // Загрузка плагина
                if /**/ (auto parser = qobject_cast<FilePluginInterface*>(pobj); pobj && parser)
                    App::fileInterfaces().emplace(parser->type(), PIF { parser, pobj });
                else if (auto parser = qobject_cast<ShapePluginInterface*>(pobj); pobj && parser)
                    App::shapeInterfaces().emplace(parser->type(), PIS { parser, pobj });
                qDebug() << __FUNCTION__ << pobj << (t.nsecsElapsed() / 1000000.0) << "ms";
            }
        }

        QSettings settings;
        settings.beginGroup("MainWindow");
        QString locale(settings.value("locale").toString());
        if (locale.isEmpty())
            locale = QLocale().name().left(2);
        settings.setValue("locale", locale);
        settings.endGroup();
        MainWindow::translate(locale);
    }

    MainWindow mainWin;
    mainWin.setObjectName("MainWindow");
    mainWin.setIconSize({ 24, 24 });

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Url of file to open");
    parser.process(app);
    for (const QString& fileName : parser.positionalArguments()) {
        mainWin.loadFile(fileName);
    }

    mainWin.show();
    splash->finish(&mainWin);

    return app.exec();
}

void initIcon(const QString& path)
{
    QIcon::setThemeSearchPaths({
        path + "/../icons/",
        path + "/../icons/breeze/",
        path + "/icons/",
        path + "/icons/breeze/",
    });
    QIcon::setThemeName("Breeze");
}
