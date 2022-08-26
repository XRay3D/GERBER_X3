#include "ex_settingstab.h"
#include <QtWidgets>
#include <doublespinbox.h>

namespace Excellon {
ExSettingsTab::ExSettingsTab(QWidget* parent)
    : SettingsTabInterface(parent) {
    setObjectName(QString::fromUtf8("tabExcellon"));

    auto vlayTab = new QVBoxLayout(this);
    vlayTab->setContentsMargins(6, 6, 6, 6);

    {
        auto gboxDefaulValues = new QGroupBox(this);

        gboxDefaulValues->setTitle(QApplication::translate("ExcellonDialog", "Default values", nullptr));
        auto grbxUnits = new QGroupBox(gboxDefaulValues);
        {
            grbxUnits->setTitle(QApplication::translate("ExcellonDialog", "Units", nullptr));
            rbInches = new QRadioButton(grbxUnits);
            rbInches->setObjectName(QString::fromUtf8("rbInches"));

            rbMillimeters = new QRadioButton(grbxUnits);
            rbMillimeters->setObjectName(QString::fromUtf8("rbMillimeters"));

            auto vlay = new QVBoxLayout(grbxUnits);
            vlay->setContentsMargins(6, 6, 6, 6);
            vlay->addWidget(rbInches);
            vlay->addWidget(rbMillimeters);
        }
        auto grbxZeroes = new QGroupBox(gboxDefaulValues);
        {
            grbxZeroes->setTitle(QApplication::translate("ExcellonDialog", "Zeroes", nullptr));

            rbLeading = new QRadioButton(grbxZeroes);
            rbLeading->setObjectName(QString::fromUtf8("rbLeading"));

            rbTrailing = new QRadioButton(grbxZeroes);
            rbTrailing->setObjectName(QString::fromUtf8("rbTrailing"));

            auto vlay = new QVBoxLayout(grbxZeroes);
            vlay->setContentsMargins(6, 6, 6, 6);
            vlay->addWidget(rbLeading);
            vlay->addWidget(rbTrailing);
        }
        auto grbxFormat = new QGroupBox(gboxDefaulValues);
        {
            grbxFormat->setTitle(QApplication::translate("ExcellonDialog", "Format", nullptr));

            sbxInteger = new QSpinBox(grbxFormat);
            sbxInteger->setObjectName(QString::fromUtf8("sbxInteger"));
            sbxInteger->setWrapping(false);
            sbxInteger->setAlignment(Qt::AlignCenter);
            sbxInteger->setProperty("showGroupSeparator", QVariant(false));
            sbxInteger->setMinimum(1);
            sbxInteger->setMaximum(8);

            sbxDecimal = new QSpinBox(grbxFormat);
            sbxDecimal->setObjectName(QString::fromUtf8("sbxDecimal"));
            sbxDecimal->setAlignment(Qt::AlignCenter);
            sbxDecimal->setMinimum(1);
            sbxDecimal->setMaximum(8);

            auto hlay = new QHBoxLayout(grbxFormat);
            hlay->setContentsMargins(6, 6, 6, 6);
            hlay->addWidget(sbxInteger);
            hlay->addWidget(new QLabel(QApplication::translate("ExcellonDialog", ":", nullptr), grbxFormat));
            hlay->addWidget(sbxDecimal);

            hlay->setStretch(0, 1);
            hlay->setStretch(2, 1);
        }

        auto grbxOffset = new QGroupBox(gboxDefaulValues);
        {
            grbxOffset->setTitle(QApplication::translate("ExcellonDialog", "Offset", nullptr));

            dsbxX = new DoubleSpinBox(grbxOffset);
            dsbxX->setObjectName(QString::fromUtf8("dsbxX"));
            dsbxX->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
            dsbxX->setDecimals(4);
            dsbxX->setMinimum(-1000.0);
            dsbxX->setMaximum(1000.0);

            dsbxY = new DoubleSpinBox(grbxOffset);
            dsbxY->setObjectName(QString::fromUtf8("dsbxY"));
            dsbxY->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
            dsbxY->setDecimals(4);
            dsbxY->setMinimum(-1000.0);
            dsbxY->setMaximum(1000.0);

            auto formLay = new QFormLayout(grbxOffset);
            formLay->setContentsMargins(6, 6, 6, 6);
            formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "X:", nullptr), grbxOffset), dsbxX);
            formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Y:", nullptr), grbxOffset), dsbxY);
        }

        auto vlay = new QVBoxLayout(gboxDefaulValues);
        vlay->setContentsMargins(6, 6, 6, 6);
        vlay->addWidget(grbxUnits);
        vlay->addWidget(grbxZeroes);
        vlay->addWidget(grbxFormat);
        vlay->addWidget(grbxOffset);
        vlayTab->addWidget(gboxDefaulValues);
    }
    if (0) {
        auto grbxParse = new QGroupBox(this);

        grbxParse->setTitle(QApplication::translate("ExcellonDialog", "Parse Reg.Expr.", nullptr));

        leParseZero = new QLineEdit(this);
        leParseZero->setObjectName(QString::fromUtf8("leParseZero"));

        leParseUnit = new QLineEdit(this);
        leParseUnit->setObjectName(QString::fromUtf8("leParseUnit"));

        leParseDecimalAndInteger = new QLineEdit(this);
        leParseDecimalAndInteger->setObjectName(QString::fromUtf8("leParseDecimalAndInteger"));

        auto leTestParseZero = new QLineEdit(this);
        auto leTestParseUnit = new QLineEdit(this);
        auto leTestParseDecimalAndInteger = new QLineEdit(this);
        auto leTestOutput = new QLineEdit(this);

        auto testParseZero = [leTestOutput, leTestParseZero, this](const QString&) {
            QRegularExpression re(leParseZero->text());
            QString text;
            if (re.isValid()) {
                auto match = re.match(leTestParseZero->text());
                for (int ctr {}; QString & string : match.capturedTexts()) {
                    text.append(QStringLiteral(R"(%1:("%2"), )").arg(ctr++).arg(string));
                }
                if (!match.hasMatch())
                    text = "No captured texts";
            } else {
                text = re.errorString();
            }
            leTestOutput->setText(text);
        };
        connect(leTestParseZero, &QLineEdit::textChanged, testParseZero);
        connect(leParseZero, &QLineEdit::textChanged, testParseZero);

        auto testParseUnit = [leTestOutput, leTestParseUnit, this](const QString&) {
            QRegularExpression re(leParseUnit->text());
            QString text;
            if (re.isValid()) {
                auto match = re.match(leTestParseUnit->text());
                for (int ctr {}; QString & string : match.capturedTexts()) {
                    text.append(QStringLiteral(R"(%1:("%2"), )").arg(ctr++).arg(string));
                }
                if (!match.hasMatch())
                    text = "No captured texts";
            } else {
                text = re.errorString();
            }
            leTestOutput->setText(text);
        };
        connect(leTestParseUnit, &QLineEdit::textChanged, testParseUnit);
        connect(leParseUnit, &QLineEdit::textChanged, testParseUnit);

        auto testlParseDecimalAndInteger = [leTestOutput, leTestParseDecimalAndInteger, this](const QString&) {
            QRegularExpression re(leParseDecimalAndInteger->text());
            QString text;
            if (re.isValid()) {
                auto match = re.match(leTestParseDecimalAndInteger->text());
                for (int ctr {}; QString & string : match.capturedTexts()) {
                    text.append(QStringLiteral(R"(%1:("%2"), )").arg(ctr++).arg(string));
                }
                if (!match.hasMatch())
                    text = "No captured texts";
            } else {
                text = re.errorString();
            }
            leTestOutput->setText(text);
        };
        connect(leTestParseDecimalAndInteger, &QLineEdit::textChanged, testlParseDecimalAndInteger);
        connect(leParseDecimalAndInteger, &QLineEdit::textChanged, testlParseDecimalAndInteger);

        auto formLay = new QFormLayout(grbxParse);
        formLay->setContentsMargins(6, 6, 6, 6);
        formLay->setLabelAlignment(Qt::AlignRight);

        QWidget* nullWidget = nullptr;

        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Parse Zero:", nullptr), grbxParse), leParseZero);
        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Test String:", nullptr), grbxParse), leTestParseZero);

        formLay->addRow(new QLabel("", grbxParse), nullWidget);

        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Parse Unit:", nullptr), grbxParse), leParseUnit);
        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Test String:", nullptr), grbxParse), leTestParseUnit);

        formLay->addRow(new QLabel("", grbxParse), nullWidget);

        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Parse Decimal And Integer:", nullptr), grbxParse), leParseDecimalAndInteger);
        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Test String:", nullptr), grbxParse), leTestParseDecimalAndInteger);

        formLay->addRow(new QLabel("", grbxParse), nullWidget);

        formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Test Output:", nullptr), grbxParse), leTestOutput);
        vlayTab->addWidget(grbxParse);
    }
    vlayTab->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    rbInches->setText(QApplication::translate("ExcellonDialog", "Inches", nullptr));
    rbLeading->setText(QApplication::translate("ExcellonDialog", "Leading", nullptr));
    rbMillimeters->setText(QApplication::translate("ExcellonDialog", "Millimeters", nullptr));
    rbTrailing->setText(QApplication::translate("ExcellonDialog", "Trailing", nullptr));
}

ExSettingsTab::~ExSettingsTab() { }

void ExSettingsTab::readSettings(MySettings& settings) {
    settings.beginGroup("Excellon");
    //  static inline Format format_;

    format_.decimal = settings.getValue(sbxDecimal, format_.decimal);
    format_.integer = settings.getValue(sbxInteger, format_.integer);

    format_.unitMode = static_cast<UnitMode>(settings.getValue(rbInches, bool(format_.unitMode == Inches)));
    format_.unitMode = static_cast<UnitMode>(settings.getValue(rbMillimeters, bool(format_.unitMode == Millimeters)));

    format_.zeroMode = static_cast<ZeroMode>(settings.getValue(rbLeading, bool(format_.zeroMode == LeadingZeros)));
    format_.zeroMode = static_cast<ZeroMode>(settings.getValue(rbTrailing, bool(format_.zeroMode == TrailingZeros)));

    //            parseZeroMode_ = settings.getValue(leParseZero, parseZeroMode_);
    //            parseUnitMode_ = settings.getValue(leParseUnit, parseUnitMode_);
    //            parseDecimalAndInteger_ = settings.getValue(leParseDecimalAndInteger, parseDecimalAndInteger_);

    settings.endGroup();
}

void ExSettingsTab::writeSettings(MySettings& settings) {
    settings.beginGroup("Excellon");

    format_.decimal = settings.setValue(sbxDecimal);
    format_.integer = settings.setValue(sbxInteger);

    settings.setValue(rbInches);
    format_.unitMode = static_cast<UnitMode>(settings.setValue(rbMillimeters));

    settings.setValue(rbLeading);
    format_.zeroMode = static_cast<ZeroMode>(settings.setValue(rbTrailing));

    //            parseZeroMode_ = settings.setValue(leParseZero);
    //            parseUnitMode_ = settings.setValue(leParseUnit);
    //            parseDecimalAndInteger_ = settings.setValue(leParseDecimalAndInteger);

    settings.endGroup();
}
} // namespace Excellon
