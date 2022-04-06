// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "explugin.h"
#include "exfile.h"
#include "exnode.h"
#include "exsettingstab.h"
#include "extypes.h"

#include "app.h"
#include "ctre.hpp"
#include "doublespinbox.h"
#include "drillitem.h"
#include "drillpreviewgi.h"
#include "ft_view.h"
#include "file.h"
#include "utils.h"

#include <QComboBox>
#include <QJsonObject>

namespace Excellon {

Plugin::Plugin(QObject* parent)
    : QObject(parent)
    , Parser(this) {
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_) {
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return nullptr;

    QTextStream in(&file);
    Parser::parseFile(fileName);
    return Parser::file;
}

QIcon Plugin::drawDrillIcon() {
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

bool Plugin::thisIsIt(const QString& fileName) {
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

QJsonObject Plugin::info() const {
    return QJsonObject {
        { "Name", "Excellon" },
        { "Version", "1.1" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Opening drill files like Excellon" },
    };
}

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent) {
    auto tab = new ExSettingsTab(parent);
    tab->setWindowTitle("Excellon");
    return tab;
}

void Plugin::addToDrillForm(FileInterface* file, QComboBox* cbx) {
    cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
    cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
    cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
}

class DrillPrGI final : public AbstractDrillPrGI {
public:
    explicit DrillPrGI(const Excellon::Hole* hole, Row& row)
        : AbstractDrillPrGI(row)
        , hole(hole) {
        m_sourceDiameter = hole->state.currentToolDiameter();
        m_sourcePath = hole->state.path.isEmpty() ? drawDrill() : drawSlot();
        m_type = hole->state.path.isEmpty() ? GiType::PrDrill : GiType::PrSlot;
    }

private:
    QPainterPath drawDrill() const {
        QPainterPath painterPath;
        const double radius = hole->state.currentToolDiameter() * 0.5;
        painterPath.addEllipse(hole->state.offsetedPos(), radius, radius);
        return painterPath;
    }

    QPainterPath drawSlot() const {
        QPainterPath painterPath;
        for (Path& path : offset(hole->item->paths().front(), hole->state.currentToolDiameter()))
            painterPath.addPolygon(path);
        return painterPath;
    }

    Paths offset(const Path& path, double offset) const {
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
    void updateTool() override {
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
    Paths paths() const override {
        if (m_type == GiType::PrSlot)
            return hole->item->paths();
        Paths paths(hole->item->paths());
        return ReversePaths(paths);
    }
    bool fit(double depth) override {
        return m_sourceDiameter > App::toolHolder().tool(row.toolId).getDiameter(depth);
    }
};

DrillPreviewGiMap Plugin::createDrillPreviewGi(FileInterface* file, mvector<Row>& data) {
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

} // namespace Excellon
