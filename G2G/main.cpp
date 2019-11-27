#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QGLWidget>
#include <QLocale>
#include <QSettings>
#include <QSplashScreen>
#include <QTranslator>

//#include "application.h"
#include "mainwindow.h"
#include "version.h"
#ifndef linux
//#include <qt_windows.h>
#endif

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    app.setApplicationName("G2G");
    app.setOrganizationName(VER_COMPANYNAME_STR);
    app.setApplicationVersion(VER_PRODUCTVERSION_STR);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());

    QIcon::setThemeSearchPaths({ "../../icons/breeze/", "../icons/breeze/", "icons/breeze/" });
    QIcon::setThemeName("Breeze");

    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(16);
    QGLFormat::setDefaultFormat(glf);

    const QString loc(QLocale().name().left(2));
    qDebug() << "locale:" << loc;
    QString trFolder;
    if (qApp->applicationDirPath().contains("build/G2G"))
        trFolder = "../../G2G/translations/";
    else
        trFolder = (qApp->applicationDirPath() + "/translations/");

    QString trFileName(trFolder + qApp->applicationDisplayName() + "_" + loc + ".qm");
    if (QFile::exists(trFileName)) {
        QTranslator* translator = new QTranslator();
        if (translator->load(trFileName))
            app.installTranslator(translator);
        else
            delete translator;
    }
    QString baseTrFileName(trFolder + "qtbase_" + loc + ".qm");
    if (QFile::exists(trFileName)) {
        QTranslator* baseTranslator = new QTranslator();
        if (baseTranslator->load(baseTrFileName))
            app.installTranslator(baseTranslator);
        else
            delete baseTranslator;
    }

    QSplashScreen* splash = nullptr;
    //int screenId = 0 QApplication::desktop()->screenNumber(tmp.geometry().center());
    splash = new QSplashScreen(/*QApplication::desktop()->screen(screenId), */ QPixmap(QLatin1String(":/256.png")));
    //    if (QApplication::desktop()->isVirtualDesktop()) {
    //        QRect srect(0, 0, splash->width(), splash->height());
    //        splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center());
    //    }
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    MainWindow mainWin;
    mainWin.setIconSize({ 24, 24 });
    mainWin.show();
    splash->finish(&mainWin);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(app);

    return app.exec();
}
