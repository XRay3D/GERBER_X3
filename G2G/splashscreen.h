#pragma once

#include <QSplashScreen>
#include "app.h"

class SplashScreen : public QSplashScreen {
public:
    SplashScreen(const QPixmap& pixmap, Qt::WindowFlags f = Qt::WindowFlags())
        : QSplashScreen(pixmap, f)
    {
        setObjectName("SplashScreen");
        App::m_splashScreen = this;
    }
    virtual ~SplashScreen() { App::m_splashScreen = nullptr; }
};
