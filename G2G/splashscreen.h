#pragma once
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <app.h>

class SplashScreen : public QSplashScreen {
public:
    SplashScreen(const QPixmap& pixmap, Qt::WindowFlags f = Qt::WindowFlags())
        : QSplashScreen(pixmap, f)
    {
        setObjectName("SplashScreen");
        App::mInstance->m_splashScreen = this;
    }
    virtual ~SplashScreen() { App::mInstance->m_splashScreen = nullptr; }
};

#endif // SPLASHSCREEN_H
