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
#include <QStyleFactory>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QGLWidget>
#endif
#include <QDir>
#include <QElapsedTimer>
#include <QPluginLoader>
#include <QProxyStyle>
#include <QSettings>
#include <QSystemSemaphore>

#include "leakdetector.h"
#include "style.h"

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

#define ATTRIBUTES_OFF() "\033[m"

#define SET_FOREGROUND_COLOR(R, G, B) "\033[38;2" \
                                      ";" #R      \
                                      ";" #G      \
                                      ";" #B "m"

#define SET_BACKGROUND_COLOR(R, G, B) "\033[48;2" \
                                      ";" #R      \
                                      ";" #G      \
                                      ";" #B "m"

//    ANSI escape color codes :
#define BG_BLACK() "\033[40m"
#define BG_BLUE() "\033[44m"
#define BG_CYAN() "\033[46m"
#define BG_GREEN() "\033[42m"
#define BG_MAGENTA() "\033[45m"
#define BG_RED() "\033[41m"
#define BG_WHITE() "\033[47m"
#define BG_YELLOW() "\033[43m"
#define FG_BLACK() "\033[30m"
#define FG_BLUE() "\033[34m"
#define FG_CYAN() "\033[36m"
#define FG_GREEN() "\033[32m"
#define FG_MAGENTA() "\033[35m"
#define FG_RED() "\033[31m"
#define FG_WHITE() "\033[37m"
#define FG_YELLOW() "\033[33m"

#define BG_BRIGHT_BLACK() "\033[100m"
#define BG_BRIGHT_BLUE() "\033[104m"
#define BG_BRIGHT_CYAN() "\033[106m"
#define BG_BRIGHT_GREEN() "\033[102m"
#define BG_BRIGHT_MAGENTA() "\033[105m"
#define BG_BRIGHT_RED() "\033[101m"
#define BG_BRIGHT_WHITE() "\033[107m"
#define BG_BRIGHT_YELLOW() "\033[103m"
#define FG_BRIGHT_BLACK() "\033[90m"
#define FG_BRIGHT_BLUE() "\033[94m"
#define FG_BRIGHT_CYAN() "\033[96m"
#define FG_BRIGHT_GREEN() "\033[92m"
#define FG_BRIGHT_MAGENTA() "\033[95m"
#define FG_BRIGHT_RED() "\033[91m"
#define FG_BRIGHT_WHITE() "\033[97m"
#define FG_BRIGHT_YELLOW() "\033[93m"

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //    ANSI escape color codes :
    //    Name            FG  BG
    //    Black           30  40
    //    Red             31  41
    //    Green           32  42
    //    Yellow          33  43
    //    Blue            34  44
    //    Magenta         35  45
    //    Cyan            36  46
    //    White           37  47
    //    Bright Black    90  100
    //    Bright Red      91  101
    //    Bright Green    92  102
    //    Bright Yellow   93  103
    //    Bright Blue     94  104
    //    Bright Magenta  95  105
    //    Bright Cyan     96  106
    //    Bright White    97  107

    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(127, 255, 255) FG_BLACK() "Debug" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 255, 0) FG_BLACK() "Info" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 255) FG_BLACK() "Warning" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() "Critical" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, SET_BACKGROUND_COLOR(255, 0, 0) FG_BLACK() "Fatal" ATTRIBUTES_OFF() ": %s" //
            SET_FOREGROUND_COLOR(127, 127, 127) "\n\t(%s:%u, %s)\n" ATTRIBUTES_OFF(),
            localMsg.constData(), file, context.line, function);
        break;
    }
}

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

int main(int argc, char** argv)
{

#if defined(Q_OS_WIN) && !defined(__GNUC__)
    HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    qInstallMessageHandler(myMessageOutput);

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

    //    QApplication::setStyle(new Style);

    QApplication app(argc, argv);

    //#ifdef Q_OS_WIN
    //    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    //    if (settings.value("AppsUseLightTheme") == 0) {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;

    const QColor darkColor = QColor(50, 50, 50);
    const QColor disabledColor = QColor(127, 127, 127);
    const QColor linkColor = QColor(61, 174, 233);
    const QColor highlightColor = QColor(218, 68, 83);
    const QColor windowTextColor = QColor(220, 220, 220);
    const QColor baseColor = QColor(30, 30, 30);

    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Shadow, disabledColor);

    darkPalette.setColor(QPalette::Text, windowTextColor);
    darkPalette.setColor(QPalette::ToolTipText, windowTextColor);
    darkPalette.setColor(QPalette::WindowText, windowTextColor);
    darkPalette.setColor(QPalette::ButtonText, windowTextColor);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::BrightText, Qt::red);

    darkPalette.setColor(QPalette::Link, linkColor);
    darkPalette.setColor(QPalette::LinkVisited, highlightColor);

    darkPalette.setColor(QPalette::AlternateBase, darkColor);
    darkPalette.setColor(QPalette::Base, baseColor);
    darkPalette.setColor(QPalette::Button, darkColor);

    darkPalette.setColor(QPalette::Highlight, highlightColor);

    darkPalette.setColor(QPalette::ToolTipBase, windowTextColor);
    darkPalette.setColor(QPalette::Window, darkColor);

    qApp->setPalette(darkPalette);
    //        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    //    }
    //#endif

    //    QApplication::setStyle(new ProxyStyle(QApplication::style()));
#ifdef linux
    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    // поэтому необходимо избавиться от данного мусора
    QSharedMemory nixFixSharedMemory("AppSettings");
    if (nixFixSharedMemory.attach())
        nixFixSharedMemory.detach();
#endif
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
        if (dir.exists()) { // Поиск всех файлов в папке "plugins"
            QStringList listFiles(dir.entryList(QStringList(suffix), QDir::Files));
            QElapsedTimer t;
            for (const auto& str : listFiles) { // Проход по всем файлам
                splash->showMessage(QObject::tr("Load plugin %1\n\n\n").arg(str), Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
                t.start();
                QPluginLoader loader(dir.absolutePath() + "/" + str);
                QObject* pobj = loader.instance(); // Загрузка плагина
                if /**/ (auto parser = qobject_cast<FilePluginInterface*>(pobj); pobj && parser)
                    App::filePlugins().emplace(parser->type(), PIF { parser, pobj });
                else if (auto parser = qobject_cast<ShapePluginInterface*>(pobj); pobj && parser)
                    App::shapePlugins().emplace(parser->type(), PIS { parser, pobj });
                qDebug() << pobj << (t.nsecsElapsed() / 1000000.0) << "ms";
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
