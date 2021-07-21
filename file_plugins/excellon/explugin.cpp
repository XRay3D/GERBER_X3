// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "explugin.h"
#include "exfile.h"
#include "exnode.h"
#include "extypes.h"

#include "app.h"
#include "ctre.hpp"
#include "doublespinbox.h"
#include "drillitem.h"
#include "drillpreviewgi.h"
#include "ft_view.h"
#include "interfaces/file.h"
#include "utils.h"
#include <QtWidgets>

#include "leakdetector.h"

namespace Excellon {

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

//const int id1 = qRegisterMetaType<Gerber::File*>("Gerber::GFile*");

Plugin::Plugin(QObject* parent)
    : QObject(parent)
    , Parser(this)
{
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_)
{
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    QTextStream in(&file);
    Parser::parseFile(fileName);
    return Parser::file;
}

QIcon Plugin::drawDrillIcon()
{
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return QIcon(pixmap);
}

bool Plugin::thisIsIt(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    QString line;

    static constexpr ctll::fixed_string regex1(R"(^T(\d+))"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"((?:([CFS])(\d*\.?\d+))?)"
                                               R"(.*$)");
    static constexpr ctll::fixed_string regex2(R"(.*Holesize.*)"); // fixed_string(".*Holesize.*");

    while (in.readLineInto(&line)) {
        auto data { toU16StrView(line) };
        if (ctre::match<regex1>(data))
            return true;
        if (ctre::match<regex2>(data))
            return true;
    }

    return false;
}

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Excellon); }

QString Plugin::folderName() const { return tr("Excellon"); }

FileInterface* Plugin::createFile() { return new File(); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Excellon" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Opening drill files like Excellon" },
    };
}

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent)
{
    class Tab : public SettingsTabInterface, Settings {

        DoubleSpinBox* dsbxX;
        DoubleSpinBox* dsbxY;
        QRadioButton* rbInches;
        QRadioButton* rbLeading;
        QRadioButton* rbMillimeters;
        QRadioButton* rbTrailing;
        QSpinBox* sbxDecimal;
        QSpinBox* sbxInteger;

        QLineEdit* leParseDecimalAndInteger;
        QLineEdit* leParseUnit;
        QLineEdit* leParseZero;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent)
        {
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
                leParseZero->setObjectName(QString::fromUtf8("pteParseZero"));

                leParseUnit = new QLineEdit(this);
                leParseUnit->setObjectName(QString::fromUtf8("pteParseUnit"));

                leParseDecimalAndInteger = new QLineEdit(this);
                leParseDecimalAndInteger->setObjectName(QString::fromUtf8("pteParseDecimalAndInteger"));

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

        virtual ~Tab() override { }

        virtual void readSettings(MySettings& settings) override
        {
            settings.beginGroup("Excellon");
            //  static inline Format m_format;

            m_format.decimal = settings.getValue(sbxDecimal, m_format.decimal);
            m_format.integer = settings.getValue(sbxInteger, m_format.integer);

            m_format.offsetPos.rx() = settings.getValue(dsbxX, m_format.offsetPos.x());
            m_format.offsetPos.ry() = settings.getValue(dsbxY, m_format.offsetPos.y());

            m_format.unitMode = static_cast<UnitMode>(settings.getValue(rbInches, bool(m_format.unitMode == Inches)));
            m_format.unitMode = static_cast<UnitMode>(settings.getValue(rbMillimeters, bool(m_format.unitMode == Millimeters)));

            m_format.zeroMode = static_cast<ZeroMode>(settings.getValue(rbLeading, bool(m_format.zeroMode == LeadingZeros)));
            m_format.zeroMode = static_cast<ZeroMode>(settings.getValue(rbTrailing, bool(m_format.zeroMode == TrailingZeros)));

            //            m_parseZeroMode = settings.getValue(leParseZero, m_parseZeroMode);
            //            m_parseUnitMode = settings.getValue(leParseUnit, m_parseUnitMode);
            //            m_parseDecimalAndInteger = settings.getValue(leParseDecimalAndInteger, m_parseDecimalAndInteger);

            settings.endGroup();
        }

        virtual void writeSettings(MySettings& settings) override
        {
            settings.beginGroup("Excellon");

            m_format.decimal = settings.setValue(sbxDecimal);
            m_format.integer = settings.setValue(sbxInteger);

            m_format.offsetPos.rx() = settings.setValue(dsbxX);
            m_format.offsetPos.ry() = settings.setValue(dsbxY);

            settings.setValue(rbInches);
            m_format.unitMode = static_cast<UnitMode>(settings.setValue(rbMillimeters));

            settings.setValue(rbLeading);
            m_format.zeroMode = static_cast<ZeroMode>(settings.setValue(rbTrailing));

            //            m_parseZeroMode = settings.setValue(leParseZero);
            //            m_parseUnitMode = settings.setValue(leParseUnit);
            //            m_parseDecimalAndInteger = settings.setValue(leParseDecimalAndInteger);

            settings.endGroup();
        }
    };
    auto tab = new Tab(parent);
    tab->setWindowTitle("Excellon");
    return tab;
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx)
{
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

class DrillPrGI final : public AbstractDrillPrGI {
public:
    explicit DrillPrGI(const Excellon::Hole* hole, Row& row)
        : AbstractDrillPrGI(row)
        , hole(hole)
    {
        m_sourceDiameter = hole->state.currentToolDiameter();
        m_sourcePath = hole->state.path.isEmpty() ? drawDrill() : drawSlot();
        m_type = hole->state.path.isEmpty() ? GiType::PrDrill : GiType::PrSlot;
    }

private:
    QPainterPath drawDrill() const
    {
        QPainterPath painterPath;
        const double radius = hole->state.currentToolDiameter() * 0.5;
        painterPath.addEllipse(hole->state.offsetedPos(), radius, radius);
        return painterPath;
    }

    QPainterPath drawSlot() const
    {
        QPainterPath painterPath;
        for (Path& path : offset(hole->item->paths().front(), hole->state.currentToolDiameter()))
            painterPath.addPolygon(path);
        return painterPath;
    }

    Paths offset(const Path& path, double offset) const
    {
        ClipperOffset cpOffset;
        // cpOffset.AddPath(path, jtRound, etClosedLine);
        cpOffset.AddPath(path, jtRound, etOpenRound);
        Paths tmpPpaths;
        cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
        for (Path& path : tmpPpaths)
            path.push_back(path.front());
        return tmpPpaths;
    }

    const Hole* hole;

    // AbstractDrillPrGI interface
public:
    void
    updateTool() override
    {
        if (row.toolId > -1) {
            colorState |= Tool;
            if (m_type == GiType::PrSlot) {
                m_toolPath = {};

                auto& tool(App::toolHolder().tool(row.toolId));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;

                Paths tmpPpath;

                ClipperOffset offset;
                offset.AddPath(hole->item->paths().front(), jtRound, etOpenRound);
                offset.Execute(tmpPpath, diameter * 0.5 * uScale);

                for (Path& path : tmpPpath) {
                    path.push_back(path.front());
                    m_toolPath.addPolygon(path);
                }

                Path path(hole->item->paths().front());

                if (path.size()) {
                    for (IntPoint& pt : path) {
                        m_toolPath.moveTo(pt - QPointF(0.0, lineKoeff));
                        m_toolPath.lineTo(pt + QPointF(0.0, lineKoeff));
                        m_toolPath.moveTo(pt - QPointF(lineKoeff, 0.0));
                        m_toolPath.lineTo(pt + QPointF(lineKoeff, 0.0));
                    }
                    m_toolPath.moveTo(path.front());
                    for (IntPoint& pt : path) {
                        m_toolPath.lineTo(pt);
                    }
                }
            }
        } else {
            colorState &= ~Tool;
            m_toolPath = {};
        }

        changeColor();
    }
    IntPoint pos() const override { return hole->state.offsetedPos(); }
    Paths paths() const override
    {
        if (m_type == GiType::PrSlot)
            return hole->item->paths();
        Paths paths(hole->item->paths());
        return ReversePaths(paths);
    }
    bool fit(double depth) override
    {
        return m_sourceDiameter > App::toolHolder().tool(row.toolId).getDiameter(depth);
    }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data)
{
    DrillPreviewGiMap giPeview;

    auto const exFile = reinterpret_cast<File*>(file);

    std::map<int, mvector<const Excellon::Hole*>> cacheHoles;
    for (const Excellon::Hole& hole : *exFile)
        cacheHoles[hole.state.tCode] << &hole;

    data.reserve(cacheHoles.size()); // !!! reserve для отсутствия реалокаций, так как DrillPrGI хранит ссылки на него !!!

    for (auto [toolNum, diameter] : exFile->tools()) {
        QString name(tr("Tool Ø%1mm").arg(diameter));
        data.emplace_back(std::move(name), drawDrillIcon(), toolNum, diameter);
        for (const Excellon::Hole* hole : cacheHoles[toolNum]) {
            if (!hole->state.path.isEmpty())
                data.back().isSlot = true;
            giPeview[toolNum].emplace_back(std::make_shared<DrillPrGI>(hole, data.back()));
        }
    }
    return giPeview;
}

} // namespace Gerber
