// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
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
#include <QPluginLoader>
#include <QProxyStyle>
#include <QSettings>
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
        switch (metric) {
        case QStyle::PM_SmallIconSize:
            return 22;
            //        case QStyle::PM_LayoutBottomMargin:
            //        case QStyle::PM_LayoutTopMargin:
            //        case QStyle::PM_LayoutLeftMargin:
            //        case QStyle::PM_LayoutRightMargin:
            //            return 22;
            //        case QStyle::PM_LayoutHorizontalSpacing:
            //        case QStyle::PM_LayoutVerticalSpacing:
            //            return 22;
            //        case QStyle::PM_ToolBarItemMargin:
            //        case QStyle::PM_ToolBarItemSpacing:
            //            return 22;
            //        case QStyle::PM_MenuHMargin:
            //        case QStyle::PM_MenuVMargin:
            //        case QStyle::PM_MenuBarItemSpacing:
            //        case QStyle::PM_MenuBarVMargin:
            //        case QStyle::PM_MenuBarHMargin:
            //        case QStyle::PM_FocusFrameVMargin:
            //        case QStyle::PM_FocusFrameHMargin:
            //            return 22;
        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
        }
    }
    //    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* w = nullptr) const override
    //    {
    //        if (w && w->objectName() == "treeView") {
    //            qDebug() << __FUNCTION__ << cc;
    //            p->setBrush(Qt::red);
    //            p->drawRect(opt->rect);
    //            return;
    //        }
    //        QProxyStyle::drawComplexControl(cc, opt, p, w);
    //    }
    //    virtual void drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* w = nullptr) const override
    //    {
    //        if (w && w->objectName() == "treeView") {
    //            qDebug() << __FUNCTION__ << element;
    //            p->setBrush(Qt::red);
    //            p->drawRect(opt->rect);
    //            return;
    //        }
    //        QProxyStyle::drawControl(element, opt, p, w);
    //    }
    //    virtual void drawItemPixmap(QPainter* p, const QRect& rect, int alignment, const QPixmap& pixmap) const override
    //    {
    //        //if (w&&w->objectName() == "treeView") {
    //        qDebug() << __FUNCTION__ << alignment;
    //        p->setBrush(Qt::red);
    //        p->drawRect(rect);
    //        return;
    //        //}
    //    }
    //    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w = nullptr) const override
    //    {
    //        if (w && w->objectName() == "treeView") {
    //            qDebug() << __FUNCTION__ << pe;
    //            p->setBrush(Qt::red);
    //            p->drawRect(opt->rect);
    //            return;
    //        }
    //        QProxyStyle::drawPrimitive(pe, opt, p, w);
    //    }
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

    //    Q_INIT_RESOURCE(resources);
    QApplication::setApplicationName("GGEasy");
    QApplication::setOrganizationName("settings" /*VER_COMPANYNAME_STR*/);
    QApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

    QApplication app(argc, argv);

    QApplication::setStyle(new ProxyStyle);

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

    //    QSystemSemaphore semaphore("GGEasy_Semaphore", 1); // создаём семафор
    //    semaphore.acquire(); // Поднимаем семафор, запрещая другим экземплярам работать с разделяемой памятью
    //#ifdef linux
    //    // в linux/unix разделяемая память не освобождается при аварийном завершении приложения,
    //    // поэтому необходимо избавиться от данного мусора
    //    QSharedMemory nix_fix_shared_memory("GGEasy_Memory");
    //    if (nix_fix_shared_memory.attach()) {
    //        nix_fix_shared_memory.detach();
    //    }
    //#endif
    //    MainWindow* mainWin = nullptr;
    //    QSharedMemory sharedMemory("GGEasy_Memory"); // Создаём экземпляр разделяемой памяти
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

    //    if (is_running) {

    //        system("pause");

    //        //        if (parser.positionalArguments().length()) {
    //        //            for (const QString& fileName : parser.positionalArguments()) {
    //        //                instance()->loadFile(fileName);
    //        //            }
    //        //        }
    //        return 1;
    //    } else {
    //    }

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
        // Поиск всех файлов в папке "Plugins"
        QStringList listFiles;
        if (dir.exists()) {
            listFiles = dir.entryList(QStringList(suffix), QDir::Files);
            // Проход по всем файлам
            for (const auto& str : listFiles) {
                QPluginLoader loader(dir.absolutePath() + "/" + str);
                // Загрузка плагина
                QObject* pobj = loader.instance();
                qDebug() << __FUNCTION__ << "\n    " << str << "\n    " << pobj;
                if (auto parser = qobject_cast<FilePluginInterface*>(pobj); pobj && parser) {
                    parser->setupInterface(App::get());
                    App::fileInterfaces().emplace(parser->type(), PIF { parser, pobj });
                }
                if (auto parser = qobject_cast<ShapePluginInterface*>(pobj); pobj && parser) {
                    parser->setupInterface(App::get());
                    App::shapeInterfaces().emplace(parser->type(), PIS { parser, pobj });
                }
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
