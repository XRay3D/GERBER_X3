// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_plugin.h"
#include "dxf_file.h"
#include "dxf_node.h"
#include "dxf_settingstab.h"

#include "entities/dxf_circle.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"

#include "tables/dxf_layer.h"

#include "drill/drill_form.h"

#include <QtWidgets>

namespace Dxf {

Plugin::Plugin(QObject* parent)
    : AbstractFilePlugin(parent) {
}

AbstractFile* Plugin::parseFile(const QString& fileName, int type_) {
    if (type_ != type())
        return nullptr;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    file_ = new File;
    file_->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QTextStream in(&file);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    in.setCodec("Windows-1251");
#endif
    //    in.setAutoDetectUnicode(true);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        file_->lines().push_back(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if (!ok)
            throw QString("Unknown code: raw str %1, line %2!").arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        file_->lines().push_back(strValue);
        int multi = 0;
        while (strValue.endsWith("\\P")) {
            file_->lines().emplace_back(in.readLine());
            strValue.append("\n" + file_->lines().back());
            ++multi;
        }
        codes.emplace_back(code, strValue, line);
        line += 2 + multi;
        return *(codes.end() - 1);
    };

    try {
        int progress = 0;
        // int progressCtr = 0;
        do {
            if (auto code = getCode(); code.code() == 0 && code == "SECTION")
                ++progress;
        } while (!in.atEnd() || *(codes.end() - 1) != "EOF");
        codes.shrink_to_fit();
        file.close();

        // emit fileProgress(file_->shortName(), progress, progressCtr);
        Timer t {"Section Parser"};

        for (auto it = codes.begin(), from = codes.begin(), to = codes.begin(); it != codes.end(); ++it) {
            if (*it == "SECTION")
                from = it;
            if (auto it_ = it + 1; *it == "ENDSEC" && (*it_ == "SECTION" || *it_ == "EOF")) {
                // emit fileProgress(file_->shortName(), 0, progressCtr++);
                to = it;
                const auto type = SectionParser::toType(*(from + 1));
                switch (type) {
                case SectionParser::HEADER:
                    file_->sections_[type] = new SectionHEADER(file_, from, to);
                    continue;
                case SectionParser::CLASSES:
                    // dxfFile()->sections_[type] = new SectionCLASSES(dxfFile(), from, to);
                    continue;
                case SectionParser::TABLES:
                    file_->sections_[type] = new SectionTABLES(file_, from, to);
                    continue;
                case SectionParser::BLOCKS:
                    file_->sections_[type] = new SectionBLOCKS(file_, from, to);
                    continue;
                case SectionParser::ENTITIES:
                    file_->sections_[type] = new SectionENTITIES(file_, from, to);
                    continue;
                case SectionParser::OBJECTS:
                    // dxfFile()->sections_[type] = new SectionOBJECTS(dxfFile(), from, to);
                    continue;
                case SectionParser::THUMBNAILIMAGE:
                    // dxfFile()->sections_[type] = new SectionTHUMBNAILIMAGE(dxfFile(), from, to);
                    continue;
                default:
                    throw QString("Unknowh Section!");
                }
            }
        }
        if (file_->sections_.size() == 0) {
            delete file_;
            file_ = nullptr;
        } else {
            // emit fileProgress(file_->shortName(), 1, 1);
            emit fileReady(file_);
        }
    } catch (const QString& wath) {
        qWarning() << "exeption QString:" << wath;
        // emit fileProgress(file_->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), wath);
        delete file_;
        return nullptr;
    } catch (const std::exception& e) {
        qWarning() << "exeption:" << e.what();
        // emit fileProgress(file_->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString(e.what()));
        delete file_;
        return nullptr;
    } catch (...) {
        qWarning() << "exeption:" << errno;
        // emit fileProgress(file_->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString::number(errno));
        delete file_;
        return nullptr;
    }
    return file_;
}

std::any Plugin::getDataForGC(AbstractFile* file, GCode::Plugin* plugin, std::any param) {
    if (plugin->type() == ::GCode::Drill) {
        DrillPlugin::Preview retData;
        auto const dxfFile = static_cast<File*>(file);
        QTransform t {dxfFile->transform()};
        for (int ctr {}; auto&& [name, layer] : dxfFile->layers()) {
            for (auto&& go : layer->graphicObjects())
                if (auto circle = (const Circle*)go.entity(); go.entity()->type() == Entity::CIRCLE)
                    retData[{ctr, circle->radius * 2, false, name + ": CIRCLE"}].posOrPath.emplace_back(t.map(circle->centerPoint));
            ctr++;
        }
        return retData;
    }
    return {};
}

void Plugin::addToGcForm(AbstractFile* file, QComboBox* cbx) {
    auto const dxfFile = static_cast<File*>(file);
    for (auto&& layer : dxfFile->layers()) {
        for (auto&& go : layer.second->graphicObjects()) {
            if (go.entity()->type() == Entity::CIRCLE) {
                cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
                cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("drill-path"));
                cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
                return;
            }
        }
    }
}

bool Plugin::thisIsIt(const QString& fileName) {

    if (fileName.endsWith(".dxf", Qt::CaseInsensitive))
        return true;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);
    do {
        QString line(in.readLine());
        if (line.toInt() == 999) {
            line = in.readLine();
            line = in.readLine();
        }
        if (line.toInt() != 0)
            break;
        if (line = in.readLine(); line != "SECTION")
            break;
        if (line = in.readLine(); line.toInt() != 2)
            break;
        if (line = in.readLine(); line != "HEADER")
            break;
        return true;
    } while (false);
    return false;
}

int Plugin::type() const { return int(FileType::Dxf_); }

QString Plugin::folderName() const { return tr("Dxf Files"); }

AbstractFile* Plugin::loadFile(QDataStream& stream) { return  load<File>(stream); }

QIcon Plugin::icon() const { return decoration(Qt::lightGray, 'D'); }

AbstractFileSettings* Plugin::createSettingsTab(QWidget* parent) {
    auto settingsTab = new SettingsTab(parent);
    settingsTab->setWindowTitle("DXF");
    return settingsTab;
}

void Plugin::updateFileModel(AbstractFile* file) {
    const auto fm = App::fileModel();
    const QModelIndex& fileIndex(file->node()->index());
    const QModelIndex index = fm->createIndex_(0, 0, fileIndex.internalId());
    // clean before insert new layers
    if (int count = fm->getItem(fileIndex)->childCount(); count) {
        fm->beginRemoveRows_(index, 0, count - 1);
        auto item = fm->getItem(index);
        do {
            item->remove(--count);
        } while (count);
        fm->endRemoveRows_();
    }
    Dxf::Layers layers;
    for (auto& [name, layer] : reinterpret_cast<File*>(file)->layers()) {

        if (!layer->isEmpty())
            layers[name] = layer;
    }
    fm->beginInsertRows_(index, 0, int(layers.size() - 1));
    for (auto& [name, layer] : layers) {

        fm->getItem(index)->addChild(new Dxf::NodeLayer(name, layer));
    }
    fm->endInsertRows_();
}

// class DrillPrGI final : public AbstractDrillPrGI {
//     const GraphicObject& go;
//     const Circle* circle;
//     const LwPolyline* lwpline;

// public:
//     explicit DrillPrGI(const GraphicObject& go, Row& row)
//         : AbstractDrillPrGI(row)
//         , go(go)
//         , circle(dynamic_cast<const Circle*>(go.entity()))
//         , lwpline(dynamic_cast<const LwPolyline*>(go.entity())) {
//         sourceDiameter_ = circle ? circle->radius * 2 : (QLineF(lwpline->poly.front(), lwpline->poly.back()).length() + lwpline->constantWidth) * go.scaleX();

//        sourcePath_ = drawDrill();
//        type_ = GiType::PrApetrure;
//    }

// private:
//     QPainterPath drawDrill() const {
//         QPainterPath painterPath;
//         painterPath.addEllipse(circle ? circle->centerPoint : go.pos(),
//             sourceDiameter_ * 0.5, sourceDiameter_ * 0.5);
//         return painterPath;
//     }

//    Paths offset(const Path& path, double offset) const {
//        ClipperOffset cpOffset;
//        cpOffset.AddPath(path, JoinType::Round, EndType::Round);
//        Paths tmpPpaths;
//        cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
//        for (Path& path : tmpPpaths)
//            path.push_back(path.front());
//        return tmpPpaths;
//    }

//    // AbstractDrillPrGI interface
// public:
//    void updateTool() override {
//        toolId_ > -1 ? colorState |= Tool : colorState &= ~Tool;
//        toolPath_ = {};
//        changeColor();
//    }
//    Point pos() const override { return circle ? circle->centerPoint : go.pos(); }
//    Paths paths() const override {
//        Paths paths { go.path() };
//        return ReversePaths(paths);
//    }
//    bool fit(double depth) override { return sourceDiameter_ >= App::toolHolder().tool(toolId_).getDiameter(depth); }
//};

// FIXME DrillPreviewGiMap Plugin::createDrillPreviewGi(AbstractFile* file, mvector<Row>& data) {
//    auto const dxfFile = static_cast<File*>(file);
//    DrillPreviewGiMap giPeview;

//    int ctr {};
//    for (auto& [key, lay] : dxfFile->layers())
//        if (lay->isVisible()) {
//            for (const auto& go : lay->graphicObjects())
//                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle)
//                    ++ctr;
//                else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
//                         && lwp->poly.size() == 2
//                         && lwp->poly.front().bulge == 1.0
//                         && lwp->poly.back().bulge == 1.0
//                         && lwp->polylineFlag == 1)
//                    ++ctr;
//        }
//    data.reserve(ctr);

//    for (auto& [key, lay] : dxfFile->layers()) {
//        std::map<double, mvector<const GraphicObject*>> cacheHoles;
//        if (lay->isVisible())
//            for (const auto& go : lay->graphicObjects()) {

//                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle) {
//                    cacheHoles[circle->radius * 2.0].push_back(&go);
//                } else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
//                           && lwp->poly.size() == 2
//                           && lwp->poly[0].bulge == 1.0
//                           && lwp->poly[1].bulge == 1.0
//                           //                       && lwp->poly.front() == lwp->poly.back()
//                           && lwp->polylineFlag == 1) {
//                    cacheHoles[(QLineF(lwp->poly.front(), lwp->poly.back()).length() + lwp->constantWidth) * go.scaleX()].push_back(&go);
//                }
//            }

//        for (auto [diameter, gos] : cacheHoles) {
//            QString name(tr("Ø%1mm").arg(diameter));
//            int32_t id = static_cast<int>(data.size());
//            data.emplace_back(std::move(name), drawDrillIcon(lay->color()), id, diameter);
//            for (const auto go : gos) {
//                giPeview[id].emplace_back(std::make_shared<DrillPrGI>(*go, data.back()));
//            }
//        }
//    }

//    return giPeview;
//}

// FIXME void Plugin::addToGcForm(AbstractFile* file, QComboBox* cbx) {
//    int ctr {};
//    for (auto& [key, lay] : static_cast<File*>(file)->layers())
//        if (lay->isVisible()) {
//            for (const auto& go : lay->graphicObjects())
//                if (auto circle = dynamic_cast<const Circle*>(go.entity()); circle) {
//                    ++ctr;
//                    break;
//                } else if (auto lwp = dynamic_cast<const LwPolyline*>(go.entity()); lwp
//                           && lwp->poly.size() == 2
//                           && lwp->poly[0].bulge == 1.0
//                           && lwp->poly[1].bulge == 1.0
//                           //                       && lwp->poly.front() == lwp->poly.back()
//                           && lwp->polylineFlag == 1) {
//                    ++ctr;
//                    break;
//                }
//        }
//    if (ctr) {
//        cbx->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
//        cbx->setItemIcon(cbx->count() - 1, QIcon::fromTheme("crosshairs"));
//        cbx->setItemData(cbx->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
//    }
//}

} // namespace Dxf

#include "moc_dxf_plugin.cpp"
