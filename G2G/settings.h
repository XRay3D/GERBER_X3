#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QPointF>
#include <QSettings>

class DoubleSpinBox;
class QAbstractButton;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QRadioButton;
class QSpinBox;
class QTabWidget;

class MySettings : public QSettings {
public:
    template <typename T>
    auto setValue(const QString& key, const T& value)
    {
        qDebug(Q_FUNC_INFO);
        QSettings::setValue(key, value);
        return value;
    }

    template <typename T>
    auto getValue(const QString& key, T& value, const QVariant& defaultValue = QVariant()) const
    {
        qDebug(Q_FUNC_INFO);
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
    GlobalSettings();

    /*G-Code*/
    static QString gcEnd();
    static QString gcFileExtension();
    static QString gcFormat();
    static bool gcInfo();
    static QString gcLaserConstOn();
    static QString gcLaserDynamOn();
    static bool gcSameFolder();
    static QString gcSpindleLaserOff();
    static QString gcSpindleOn();
    static QString gcStart();

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

protected:
    /*G-Code*/
    static QString m_gcEnd;
    static QString m_gcFileExtension;
    static QString m_gcFormat;
    static bool m_gcInfo;
    static QString m_gcLaserConstOn;
    static QString m_gcLaserDynamOn;
    static bool m_gcSameFolder;
    static QString m_gcSpindleLaserOff;
    static QString m_gcSpindleOn;
    static QString m_gcStart;

    /*GUI*/
    static QColor m_guiColor[static_cast<int>(Colors::Count)];
    static bool m_guiSmoothScSh;

    /*Gerber/G-Code*/
    static double m_gbrGcMinCircleSegmentLength;
    static int m_gbrGcMinCircleSegments;

    /*Gerber*/
    static bool m_gbrCleanPolygons;
    static bool m_gbrSimplifyRegions;
    static bool m_gbrSkipDuplicates;

    /*Markers*/
    static QPointF m_mrkHomeOffset;
    static int m_mrkHomePos;
    static QPointF m_mrkPinOffset;
    static QPointF m_mrkZeroOffset;
    static int m_mrkZeroPos;

    /*Other*/
    static bool m_inch;
};

#endif // SETTINGS_H
