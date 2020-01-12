#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
class SplashScreen : public QSplashScreen {
public:
    SplashScreen(const QPixmap& pixmap, Qt::WindowFlags f = Qt::WindowFlags())
        : QSplashScreen(pixmap, f)
    {
        instance = this;
    }
    virtual ~SplashScreen() { instance = nullptr; }
    static SplashScreen* instance;
};

#endif // SPLASHSCREEN_H
