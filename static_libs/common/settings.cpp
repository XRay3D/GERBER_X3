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
#include <QtWidgets>
#include <cmath>
#include <numbers>

using std::numbers::pi;

template <typename W, auto SET, auto GET>
struct WidgetIO {
    static std::optional<QVariant> save(QWidget* w) {
        if(auto cw = qobject_cast<W*>(w))
            return QVariant::fromValue((cw->*GET)());
        return {};
    }
    static bool load(QWidget* w, QVariant& var) {
        if(auto cw = qobject_cast<W*>(w))
            return (cw->*SET)(Cast{var}), true;
        return {};
    }
};

template <typename... Ws>
struct Widgets {
    static QVariant save(QWidget* w) {
        for(auto save: std::array{Ws::save...})
            if(auto opt = save(w))
                return std::move(*opt);
        return {};
    }
    static bool load(QWidget* w, QVariant& var) {
        if(var.isNull()) return {};
        for(auto load: std::array{Ws::load...})
            if(load(w, var)) return true;
        return {};
    }
};

constexpr Widgets<
    // clang-format off
    WidgetIO<QTimeEdit,       &QTimeEdit::setTime,           &QTimeEdit::time>,
    WidgetIO<QCheckBox,       &QCheckBox::setCheckState,     &QCheckBox::checkState>,
    WidgetIO<QAbstractButton, &QAbstractButton::setChecked,  &QAbstractButton::isChecked>,
    WidgetIO<QComboBox,       &QComboBox::setCurrentText,    &QComboBox::currentText>,
    WidgetIO<QDoubleSpinBox,  &QDoubleSpinBox::setValue,     &QDoubleSpinBox::value>,
    WidgetIO<QLineEdit,       &QLineEdit::setText,           &QLineEdit::text>,
    WidgetIO<QPlainTextEdit,  &QPlainTextEdit::setPlainText, &QPlainTextEdit::toPlainText>,
    WidgetIO<QSpinBox,        &QSpinBox::setValue,           &QSpinBox::value>,
    WidgetIO<QTabWidget,      &QTabWidget::setCurrentIndex,  &QTabWidget::currentIndex>
    // clang-format on
    >
    Widget;

/*G-Code*/
// AppSettings::AppSettings() {
// if (!settings_)
// settings_ = this;
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
    if(bool(mod & Qt::ALT) ^ snap_) {
        const double scale = AppSettings::gridStep(App::grView().getScale());
        const auto px = pt / scale;
        return {scale * std::round(px.x()), scale * std::round(px.y())};
    }
    return pt;
}

void AppSettings::setSnap(bool val) { snap_ = val; }
bool AppSettings::snap() { return snap_; }

#if 0
void MySettings::setValue(QWidget* w) {
    if(!w) return;
    const QString objectName{w->objectName()};
    if(!objectName.size() || objectName.contains(u'_')) {
        // qWarning() << w;
        return;
    }

    if(QVariant var = Widget.save(w); !var.isNull()) {
        // if(!widgetData.emplace(objectName, std::move(var)).second)
        widgetData[objectName] = std::move(var);
        return;
    }
}

void MySettings::getValue(QWidget* w, const QVariant& defaultValue) {
    if(!w) return;
    const QString objectName{w->objectName()};
    if(!objectName.size() || objectName.contains(u'_')) {
        // qWarning() << w;
        return;
    }
    if(auto it = widgetData.find(objectName);
        it != widgetData.end()
        && Widget.load(w, it->second))
        return;
}
#endif
