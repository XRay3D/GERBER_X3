/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QColor>
#include <QDebug>
#include <QPointF>
#include <QSettings>
#include <stdexcept>
#include <type_traits>

class DoubleSpinBox;
class QAbstractButton;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QTabWidget;

#define varName(val) val, #val

class MySettings : public QSettings {
public:
    template <typename T>
    auto setValue(const QString& key, const T& value)
    {
        QSettings::setValue(key, value);
        return value;
    }

    template <typename T>
    auto getValue(const QString& key, T& value, const QVariant& defaultValue = QVariant()) const
    {
        value = QSettings::value(key, defaultValue).value<T>();
        return value;
    }

    template <typename W, typename = std::enable_if_t<std::is_base_of_v<QWidget, W>>>
    auto setValue(W* widget)
    {
        const QString name { widget->objectName() };
        assert(!name.isEmpty());

        if constexpr (std::is_base_of_v<QAbstractButton, W>) {
            QSettings::setValue(name, widget->isChecked());
            return widget->isChecked();
        } else if constexpr (std::is_base_of_v<QDoubleSpinBox, W>) {
            QSettings::setValue(name, widget->value());
            return widget->value();
        } else if constexpr (std::is_same_v<W, QSpinBox>) {
            QSettings::setValue(name, widget->value());
            return widget->value();
        } else if constexpr (std::is_same_v<W, QComboBox>) {
            QSettings::setValue(name, widget->currentIndex());
            return widget->currentIndex();
        } else if constexpr (std::is_same_v<W, QLineEdit>) {
            QSettings::setValue(name, widget->text());
            return widget->text();
        } else if constexpr (std::is_same_v<W, QPlainTextEdit>) {
            QSettings::setValue(name, widget->toPlainText());
            return widget->toPlainText();
        } else if constexpr (std::is_same_v<W, QTabWidget>) {
            QSettings::setValue(name, widget->currentIndex());
            return widget->currentIndex();
        } else {
            throw std::logic_error(typeid(W).name());
        }
    }

    template <typename W, typename = std::enable_if_t<std::is_base_of_v<QWidget, W>>>
    auto getValue(W* widget, const QVariant& defaultValue = QVariant()) const
    {
        const QString name { widget->objectName() };
        assert(!name.isEmpty());

        if constexpr (std::is_base_of_v<QAbstractButton, W>) {
            widget->setChecked(QSettings::value(name, defaultValue).toBool());
            return widget->isChecked();
        } else if constexpr (std::is_base_of_v<QDoubleSpinBox, W>) {
            widget->setValue(QSettings::value(name, defaultValue).toDouble());
            return widget->value();
        } else if constexpr (std::is_same_v<W, QSpinBox>) {
            widget->setValue(QSettings::value(name, defaultValue).toInt());
            return widget->value();
        } else if constexpr (std::is_same_v<W, QComboBox>) {
            widget->setCurrentIndex(QSettings::value(name, defaultValue).toInt());
            return widget->currentIndex();
        } else if constexpr (std::is_same_v<W, QLineEdit>) {
            widget->setText(QSettings::value(name, defaultValue).toString());
            return widget->text();
        } else if constexpr (std::is_same_v<W, QPlainTextEdit>) {
            widget->setPlainText(QSettings::value(name, defaultValue).toString());
            return widget->toPlainText();
        } else if constexpr (std::is_same_v<W, QTabWidget>) {
            widget->setCurrentIndex(QSettings::value(name, defaultValue).toInt());
            return widget->currentIndex();
        } else {
            throw std::logic_error(typeid(W).name());
        }
    }

    template <typename V, typename = std::enable_if_t<std::is_fundamental_v<V>>>
    auto getValue(V& val, const char* name, V def) const
    {
        if constexpr (std::is_floating_point_v<V>) {
            val = QSettings::value(name, def).toDouble();
            return val;
        } else if constexpr (std::is_integral_v<V>) {
            val = QSettings::value(name, def).toInt();
            return val;
        } else {
            throw std::logic_error(typeid(V).name());
        }
    }

    template <typename V, typename = std::enable_if_t<std::is_fundamental_v<V>>>
    auto setValue(V val, const char* name)
    {
        QSettings::setValue(name, val);
        return val;
    }
};

enum class Colors : int {
    Background,
    Pin,
    CutArea,
    Grid1,
    Grid5,
    Grid10,
    Hole,
    Home,
    ToolPath,
    Zero,
    G0,
    Count
};

namespace HomePosition {
enum {
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight,
    AlwaysZero
};
}

class GlobalSettings {
public:
    /*G-Code*/
    static QString gcFileExtension();
    static QString gcFormat();
    static QString gcLaserConstOn();
    static QString gcLaserDynamOn();
    static QString gcSpindleLaserOff();
    static QString gcSpindleOn();

    static QString gcStart();
    static QString gcEnd();

    static QString gcLaserStart();
    static QString gcLaserEnd();

    static bool gcInfo();
    static bool gcSameFolder();

    /*GUI*/
    static QColor& guiColor(Colors id);
    static bool guiSmoothScSh();

    /*Gerber/G-Code*/
    static int gbrGcCircleSegments(double radius);

    /*Gerber*/
    static bool gbrCleanPolygons();
    static bool gbrSimplifyRegions();
    static bool gbrSkipDuplicates();

    static QPointF mkrHomeOffset();
    static int mkrHomePos();
    static QPointF mkrPinOffset();
    static QPointF mkrZeroOffset();
    static int mkrZeroPos();
    /*Other*/
    static double gridStep(double scale);
    static bool inch();
    static void setInch(bool val);
    static QPointF getSnappedPos(QPointF pt, Qt::KeyboardModifiers mod = Qt::NoModifier);
    static void setSnap(bool val) { m_snap = val; }
    static bool snap() { return m_snap; }

protected:
    /*G-Code*/
    inline static QString m_gcFileExtension = { "tap" };
    inline static QString m_gcFormat { "G?X?Y?Z?F?S?" };
    inline static QString m_gcLaserConstOn { "M3" };
    inline static QString m_gcLaserDynamOn { "M4" };
    inline static QString m_gcSpindleLaserOff { "M5" };
    inline static QString m_gcSpindleOn { "M3" };

    inline static QString m_gcStart { "G21 G17 G90\nM3 S?" };
    inline static QString m_gcEnd { "M5\nM30" };

    inline static QString m_gcLaserStart { "G21 G17 G90" };
    inline static QString m_gcLaserEnd { "M30" };

    inline static bool m_gcInfo { true };
    inline static bool m_gcSameFolder { true };

    /*GUI*/
    enum { gridColor = 100 };
    inline static QColor m_guiColor[static_cast<int>(Colors::Count)] {
        QColor(Qt::black), //Background
        QColor(255, 255, 0, 120), //Pin
        QColor(Qt::gray), //CutArea
        QColor(gridColor, gridColor, gridColor, 50), //Grid1
        QColor(gridColor, gridColor, gridColor, 100), //Grid5
        QColor(gridColor, gridColor, gridColor, 200), //Grid10
        QColor(), //Hole
        QColor(0, 255, 0, 120), //Home
        QColor(Qt::white), //ToolPath
        QColor(255, 0, 0, 120), //Zero
        QColor(Qt::red) //G0
    };
    inline static bool m_guiSmoothScSh;

    /*Gerber/G-Code*/
    inline static double m_gbrGcMinCircleSegmentLength { 0.5 };
    inline static int m_gbrGcMinCircleSegments { 36 };

    /*Gerber*/
    inline static bool m_gbrCleanPolygons;
    inline static bool m_gbrSimplifyRegions;
    inline static bool m_gbrSkipDuplicates;

    /*Markers*/
    inline static QPointF m_mrkHomeOffset;
    inline static QPointF m_mrkPinOffset;
    inline static QPointF m_mrkZeroOffset;
    inline static int m_mrkHomePos { Qt::BottomLeftCorner };
    inline static int m_mrkZeroPos { Qt::BottomLeftCorner };

    /*Other*/
    inline static bool m_inch;
    inline static bool m_snap;
};
