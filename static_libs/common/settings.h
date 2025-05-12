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
#pragma once
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QPointF>
#include <QSettings>
#include <stdexcept>
#include <type_traits>

class DoubleSpinBox;
class QAbstractButton;
class QComboBox;
class QDoubleSpinBox;
class QFontComboBox;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QTabWidget;

#define varName(val) val, #val

template <typename W> concept IsWidget = std::is_base_of_v<QWidget, W>;
template <typename T> concept IsArithmetic = std::is_arithmetic_v<T>;

class MySettings : public QSettings {
public:
    template <typename T>
    auto setValue(const QString& key, const T& value) {
        return QSettings::setValue(key, value), value;
    }

    template <typename T>
    auto getValue(const QString& key, T& value, const QVariant& defaultValue = {}) const {
        return value = QSettings::value(key, defaultValue).value<T>();
    }

    template <IsWidget W>
    auto setValue(W* widget) {
        const QString name{widget->objectName()};
        assert(!name.isEmpty());

        if constexpr(std::is_base_of_v<QAbstractButton, W>)
            return QSettings::setValue(name, widget->isChecked()),
                   widget->isChecked();
        else if constexpr(std::is_base_of_v<QDoubleSpinBox, W>)
            return QSettings::setValue(name, widget->value()),
                   widget->value();
        else if constexpr(std::is_same_v<W, QSpinBox>)
            return QSettings::setValue(name, widget->value()),
                   widget->value();
        else if constexpr(std::is_same_v<W, QComboBox>)
            return QSettings::setValue(name, widget->currentIndex()),
                   widget->currentIndex();
        else if constexpr(std::is_same_v<W, QFontComboBox>) //
            return QSettings::setValue(name, widget->currentFont().family()),
                   widget->currentFont().family();
        else if constexpr(std::is_same_v<W, QLineEdit>)
            return QSettings::setValue(name, widget->text()),
                   widget->text();
        else if constexpr(std::is_same_v<W, QPlainTextEdit>)
            return QSettings::setValue(name, widget->toPlainText()),
                   widget->toPlainText();
        else if constexpr(std::is_same_v<W, QTabWidget>)
            return QSettings::setValue(name, widget->currentIndex()),
                   widget->currentIndex();
        else
            throw std::logic_error(typeid(W).name());
    }

    template <IsWidget W>
    auto getValue(W* widget, const QVariant& defaultValue = {}) const {
        const QString name{widget->objectName()};
        assert(!name.isEmpty());

        if constexpr(std::is_base_of_v<QAbstractButton, W>)
            return widget->setChecked(QSettings::value(name, defaultValue).toBool()),
                   widget->isChecked();
        else if constexpr(std::is_base_of_v<QDoubleSpinBox, W>)
            return widget->setValue(QSettings::value(name, defaultValue).toDouble()),
                   widget->value();
        else if constexpr(std::is_same_v<W, QSpinBox>)
            return widget->setValue(QSettings::value(name, defaultValue).toInt()),
                   widget->value();
        else if constexpr(std::is_same_v<W, QComboBox>)
            return widget->setCurrentIndex(QSettings::value(name, defaultValue).toInt()),
                   widget->currentIndex();
        else if constexpr(std::is_same_v<W, QFontComboBox>) //
            return widget->setCurrentFont(QFont{QSettings::value(name, defaultValue).toString()}),
                   widget->currentFont().family();
        else if constexpr(std::is_same_v<W, QLineEdit>)
            return widget->setText(QSettings::value(name, defaultValue).toString()),
                   widget->text();
        else if constexpr(std::is_same_v<W, QPlainTextEdit>)
            return widget->setPlainText(QSettings::value(name, defaultValue).toString()),
                   widget->toPlainText();
        else if constexpr(std::is_same_v<W, QTabWidget>)
            return widget->setCurrentIndex(QSettings::value(name, defaultValue).toInt()),
                   widget->currentIndex();
        else
            throw std::logic_error(typeid(W).name());
    }

    template <IsArithmetic V>
    auto getValue(V& val, const char* name, V def = {}) const {
        return val = QSettings::value(name, def).template value<V>();
    }

    template <IsArithmetic V>
    auto setValue(V val, const char* name) {
        return QSettings::setValue(name, val), val;
    }
};

struct GuiColors {
    enum Name : int {
        Background,
        Pin,
        CutArea,
        Grid01,
        Grid05,
        Grid10,
        Hole,
        Home,
        ToolPath,
        Zero,
        G0,
        Count
    };
    Q_GADGET
    Q_ENUM(Name)
};

enum HomePosition : int {
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight,
    AlwaysZero
};

enum Theme : int {
    System,
    LightBlue,
    LightRed,
    DarkBlue,
    DarkRed,
};

class SettingsDialog;

class AppSettings {
    friend class SettingsDialog;

public:
    explicit AppSettings() = default;

    //    AppSettings(const AppSettings&) = delete;
    //    AppSettings(AppSettings&&) = delete;
    //    AppSettings& operator=(AppSettings&& a) = delete;
    //    AppSettings& operator=(const AppSettings& app) = delete;

    //    void set(AppSettings* appSettings);
    //    AppSettings* ptr();

    /*GUI*/
    QColor& guiColor(int32_t id);
    bool animSelection();
    bool guiSmoothScSh();
    bool scaleHZMarkers();
    bool scalePinMarkers();
    int theme();

    /*Clipper*/
    int clpCircleSegments(double radius);

    /*Markers*/
    QPointF mkrPinOffset();
    QPointF mkrHomeOffset();
    uint mkrHomePos();
    QPointF mkrZeroOffset();
    uint mkrZeroPos();

    /*Other*/
    bool isBanana();
    double lenUnit();
    void setBanana(bool val);
    void setSnap(bool val);
    bool snap();

private:
    //    inline static AppSettings* settings_ = nullptr;

    /*GUI*/
    enum {
        gridColor = 100
    };
    QColor guiColor_[GuiColors::Count]{
        QColor(Qt::black),                            // Background
        QColor(255, 255, 0, 120),                     // Pin
        QColor(Qt::gray),                             // CutArea
        QColor(gridColor, gridColor, gridColor, 50),  // Grid1
        QColor(gridColor, gridColor, gridColor, 100), // Grid5
        QColor(gridColor, gridColor, gridColor, 200), // Grid10
        QColor(),                                     // Hole
        QColor(0, 255, 0, 120),                       // Home
        QColor(Qt::white),                            // ToolPath
        QColor(255, 0, 0, 120),                       // Zero
        QColor(Qt::red)                               // G0
    };
    bool animSelection_ = true;
    bool guiSmoothScSh_;
    bool scaleHZMarkers_{};
    bool scalePinMarkers_{};
    int theme_ = false;

    /*Clipper*/
    double clpMinCircleSegmentLength_{0.5};
    int clpMinCircleSegments_{36};

    /*Markers*/
    QPointF mrkHomeOffset_;
    QPointF mrkPinOffset_;
    QPointF mrkZeroOffset_;
    uint mrkHomePos_{Qt::BottomLeftCorner};
    uint mrkZeroPos_{Qt::BottomLeftCorner};

    /*Other*/
    bool banana_ = false;
    bool snap_ = false;
};
