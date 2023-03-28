/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include "app.h"
#include <QSplashScreen>

class SplashScreen : public QSplashScreen {
public:
    SplashScreen(const QPixmap& pixmap, Qt::WindowFlags f = Qt::WindowFlags())
        : QSplashScreen(pixmap, f) {
        App::setSplashScreen(this);
        setObjectName("SplashScreen");
    }
    virtual ~SplashScreen() {
        App::setSplashScreen(nullptr);
    }
};
