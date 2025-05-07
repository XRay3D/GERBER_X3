/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
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
QColor& AppSettings::guiColor(int32_t id) { return guiColor_[id]; }
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
    while(intSteps < destSteps)
        intSteps <<= 1;
    return intSteps;
}

/*Markers*/
QPointF AppSettings::mkrPinOffset() { return mrkPinOffset_; }
QPointF AppSettings::mkrHomeOffset() { return mrkHomeOffset_; }
uint AppSettings::mkrHomePos() { return mrkHomePos_; }
QPointF AppSettings::mkrZeroOffset() { return mrkZeroOffset_; }
uint AppSettings::mkrZeroPos() { return mrkZeroPos_; }

/*Other*/
double AppSettings::gridStep(double scale) {
    if(banana_) return pow(10.0, ceil(log10(10.0 / 25.4 / scale))) * 25.4;
    else [[likely]] return pow(10.0, ceil(log10(10.0 / scale)));
}
bool AppSettings::isBanana() { return banana_; }
double AppSettings::lenUnit() { return banana_ ? 25.4 : 1.0; }
void AppSettings::setBanana(bool val) { banana_ = val; }

QPointF AppSettings::getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod) {
    if((mod & Qt::ALT) || snap_) {
        const double scale = AppSettings::gridStep(App::grView().getScale());
        const auto px = pt / scale;
        return {scale * std::round(px.x()), scale * std::round(px.y())};
    }
    return pt;
}

void AppSettings::setSnap(bool val) { snap_ = val; }
bool AppSettings::snap() { return snap_; }
