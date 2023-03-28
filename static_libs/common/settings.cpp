// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "settings.h"
#include "app.h"
#include "graphicsview.h"
#include <cmath>
#include <numbers>
using std::numbers::pi;

/*G-Code*/
// AppSettings::AppSettings() {
//     if (!settings_)
//         settings_ = this;
//}

// void AppSettings::set(AppSettings* appSettings) { settings_ = appSettings; }
// AppSettings* AppSettings::ptr() { return settings_; }

/*GUI*/
QColor& AppSettings::guiColor(int id) { return guiColor_[id]; }
bool AppSettings::animSelection() { return animSelection_; }
bool AppSettings::guiSmoothScSh() { return guiSmoothScSh_; }
bool AppSettings::scaleHZMarkers() { return scaleHZMarkers_; }
bool AppSettings::scalePinMarkers() { return scalePinMarkers_; }
int AppSettings::theme() { return theme_; }

/*Clipper*/
int AppSettings::clpCircleSegments(double radius) {
    const double length = clpMinCircleSegmentLength_; // mm
    const int destSteps = static_cast<int>(pi / asin((length * 0.5) / (radius)));
    int intSteps = clpMinCircleSegments_;
    while (intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Markers*/
QPointF AppSettings::mkrHomeOffset() { return mrkHomeOffset_; }
int AppSettings::mkrHomePos() { return mrkHomePos_; }
QPointF AppSettings::mkrPinOffset() { return mrkPinOffset_; }
QPointF AppSettings::mkrZeroOffset() { return mrkZeroOffset_; }
int AppSettings::mkrZeroPos() { return mrkZeroPos_; }

/*Other*/
double AppSettings::gridStep(double scale) { return inch_ ? pow(10.0, ceil(log10(30 / scale))) * .254 : pow(10.0, ceil(log10(8.0 / scale))); }
bool AppSettings::inch() { return inch_; }
void AppSettings::setInch(bool val) { inch_ = val; }

QPointF AppSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod) {
    if ((mod & Qt::ALT) || snap_) {
        const double gs = AppSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pt / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        return px;
    }
    return pt;
}

void AppSettings::setSnap(bool val) {
    snap_ = val;
}

bool AppSettings::snap() { return snap_; }
