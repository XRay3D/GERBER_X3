// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "mainwindow.h"
#include "settingsdialog.h"
#include "splashscreen.h"
#include "version.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QGLWidget>
#include <QSettings>
#include <QTranslator>

void initIcon(const QString& path);
void translation(QApplication* app);

int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    app.setApplicationName("G2G");
    app.setOrganizationName(VER_COMPANYNAME_STR);
    app.setApplicationVersion(VER_PRODUCTVERSION_STR);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());

    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(16);
    QGLFormat::setDefaultFormat(glf);

    initIcon(qApp->applicationDirPath());
    translation(&app);

    //    QSystemSemaphore semaphore("G2G_Semaphore", 1); // создаём семафор
    //    semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
    //#ifdef linux
    //    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    //    // поэтому необходимо избавиться от данного мусора
    //    QSharedMemory nix_fix_shared_memory("G2G_Memory");
    //    if (nix_fix_shared_memory.attach()) {
    //        nix_fix_shared_memory.detach();
    //    }
    //#endif
    //    MainWindow* mainWin = nullptr;
    //    QSharedMemory sharedMemory("G2G_Memory"); // Создаём экземпляр разделяемой памяти
    //    auto instance = [&sharedMemory]() -> MainWindow*& { return *static_cast<MainWindow**>(sharedMemory.data()); };
    //    bool is_running = false; // переменную для проверки ууже запущенного приложения
    //    if (sharedMemory.attach()) { // пытаемся присоединить экземпляр разделяемой памяти к уже существующему сегменту
    //        is_running = true; // Если успешно, то определяем, что уже есть запущенный экземпляр
    //    } else {
    //        sharedMemory.create(sizeof(mainWin)); // В противном случае выделяем 1 байт памяти
    //        is_running = false; // И определяем, что других экземпляров не запущено
    //    }
    //    semaphore.release(); // Опускаем семафор
    //    QCommandLineParser parser;
    //    parser.addPositionalArgument("url", "Url of file to open");
    //    parser.process(app);
    //    qDebug() << parser.positionalArguments().length() << parser.positionalArguments();
    //    if (is_running) {
    //        qDebug() << "instance()" << sharedMemory.data();
    //        system("pause");
    //        //        qDebug() << instance();
    //        //        if (parser.positionalArguments().length()) {
    //        //            for (const QString& fileName : parser.positionalArguments()) {
    //        //                instance()->loadFile(fileName);
    //        //            }
    //        //        }
    //        return 1;
    //    } else {
    //    }
    //    if (parser.positionalArguments().length()) {
    //        QSplashScreen* splash = nullptr;
    //        splash = new QSplashScreen(QPixmap(QLatin1String(":/256.png")));
    //        splash->setAttribute(Qt::WA_DeleteOnClose);
    //        splash->show();
    //        mainWin = new MainWindow();
    //        qDebug() << mainWin << sharedMemory.data();
    //        instance() = mainWin;
    //        mainWin->setIconSize({ 24, 24 });
    //        mainWin->show();
    //        splash->finish(mainWin);
    //        for (const QString& fileName : parser.positionalArguments()) {
    //            //            if (fileName.endsWith("g2g")) {
    //            //                MainWindow* mainWin = new MainWindow();
    //            //                mainWin->setIconSize({ 24, 24 });
    //            //                mainWin->show();
    //            //                mainWin->loadFile(fileName);
    //            //            } else
    //            mainWin->loadFile(fileName);
    //        }
    //    } else {
    //        QSplashScreen* splash = nullptr;
    //        splash = new QSplashScreen(QPixmap(QLatin1String(":/256.png")));
    //        splash->setAttribute(Qt::WA_DeleteOnClose);
    //        splash->show();
    //        mainWin = new MainWindow();
    //        qDebug() << mainWin << sharedMemory.data();
    //        instance() = mainWin;
    //        mainWin->setIconSize({ 24, 24 });
    //        mainWin->show();
    //        splash->finish(mainWin);
    //    }

    SplashScreen* splash = new SplashScreen(QPixmap(QLatin1String(":/256.png")));
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    {
        SettingsDialog().readSettings();
    }

    MainWindow mainWin;
    mainWin.setObjectName("MainWindow");
    mainWin.setIconSize({ 24, 24 });
    mainWin.show();
    splash->finish(&mainWin);

    QCommandLineParser parser;
    parser.addPositionalArgument("url", "Url of file to open");
    parser.process(app);
    for (const QString& fileName : parser.positionalArguments()) {
        mainWin.loadFile(fileName);
    }

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

void translation(QApplication* app)
{
    const QString loc(QLocale().name().left(2));
    qDebug() << "locale:" << loc;
    QString trFolder;

    if (qApp->applicationDirPath().contains("GERBER_X2/bin"))
        trFolder = qApp->applicationDirPath() + "/../G2G/translations/"; // for debug
    else
        trFolder = (qApp->applicationDirPath() + "/translations/");

    auto translator = [app](const QString& path) {
        if (QFile::exists(path)) {
            QTranslator* pTranslator = new QTranslator();
            if (pTranslator->load(path))
                app->installTranslator(pTranslator);
            else
                delete pTranslator;
        }
    };

    translator(trFolder + qApp->applicationDisplayName().toLower() + "_" + loc + ".qm");
    translator(trFolder + "qtbase_" + loc + ".qm");
    translator(trFolder + "qt_" + loc + ".qm");
}
