// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "dxf_plugin.h"
#include "dxf_file.h"
#include "dxf_node.h"

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "section/dxf_headerparser.h"
#include "section/dxf_tables.h"
//#include "section/dxf_classes.h"
//#include "section/dxf_objects.h"
//#include "section/dxf_thumbnailimage.h"

#include "tables/dxf_layer.h"

#include "treeview.h"

#include <QtWidgets>

namespace Dxf {

Plugin::Plugin(QObject* parent)
    : QObject(parent)
{
}

FileInterface* Plugin::parseFile(const QString& fileName, int type_)
{
    if (type_ != type())
        return nullptr;
    QFile file_(fileName);
    if (!file_.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    file = new File;
    file->setFileName(fileName);

    int line = 1;

    Codes codes;
    codes.reserve(10000);

    QTextStream in(&file_);
    in.setAutoDetectUnicode(true);

    auto getCode = [&in, &codes, &line, this] {
        // Code
        QString strCode(in.readLine());
        file->lines().append(strCode);
        bool ok;
        auto code(strCode.toInt(&ok));
        if (!ok)
            throw QString("Unknown code: raw str %1, line %2!").arg(strCode).arg(line);
        // Value
        QString strValue(in.readLine());
        file->lines().append(strValue);
        int multi = 0;
        while (strValue.endsWith("\\P")) {
            file->lines().append(in.readLine());
            strValue.append("\n" + file->lines().last());
            ++multi;
        }
        codes.emplace_back(code, strValue, line);
        line += 2 + multi;
        return *(codes.end() - 1);
    };

    try {
        int progress = 0;
        //int progressCtr = 0;
        do {
            if (auto code = getCode(); code.code() == 0 && code == "SECTION")
                ++progress;
        } while (!in.atEnd() || *(codes.end() - 1) != "EOF");
        codes.shrink_to_fit();
        file_.close();

        //emit fileProgress(m_file->shortName(), progress, progressCtr);

        for (auto it = codes.begin(), from = codes.begin(), to = codes.begin(); it != codes.end(); ++it) {
            if (*it == "SECTION")
                from = it;
            if (auto it_ = it + 1; *it == "ENDSEC" && (*it_ == "SECTION" || *it_ == "EOF")) {
                //emit fileProgress(m_file->shortName(), 0, progressCtr++);
                to = it;
                const auto type = SectionParser::toType(*(from + 1));
                switch (type) {
                case SectionParser::HEADER:
                    file->m_sections[type] = new SectionHEADER(file, from, to);
                    break;
                case SectionParser::CLASSES:
                    //dxfFile()->m_sections[type] = new SectionCLASSES(dxfFile(), from, to);
                    break;
                case SectionParser::TABLES:
                    file->m_sections[type] = new SectionTABLES(file, from, to);
                    break;
                case SectionParser::BLOCKS:
                    file->m_sections[type] = new SectionBLOCKS(file, from, to);
                    break;
                case SectionParser::ENTITIES:
                    file->m_sections[type] = new SectionENTITIES(file, from, to);
                    break;
                case SectionParser::OBJECTS:
                    //dxfFile()->m_sections[type] = new SectionOBJECTS(dxfFile(), from, to);
                    break;
                case SectionParser::THUMBNAILIMAGE:
                    //dxfFile()->m_sections[type] = new SectionTHUMBNAILIMAGE(dxfFile(), from, to);
                    break;
                default:
                    throw QString("Unknowh Section!");
                    break;
                }
                if (file->m_sections.contains(type))
                    file->m_sections[type]->parse();
            }
        }
        if (file->m_sections.size() == 0) {
            delete file;
            file = nullptr;
        } else {
            //emit fileProgress(m_file->shortName(), 1, 1);
            emit fileReady(file);
        }
    } catch (const QString& wath) {
        qWarning() << "exeption QString:" << wath;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), wath);
        delete file;
        return nullptr;
    } catch (const std::exception& e) {
        qWarning() << "exeption:" << e.what();
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString(e.what()));
        delete file;
        return nullptr;
    } catch (...) {
        qWarning() << "exeption:" << errno;
        //emit fileProgress(m_file->shortName(), 1, 1);
        emit fileError(QFileInfo(fileName).fileName(), "Unknown Error! " + QString::number(errno));
        delete file;
        return nullptr;
    }
    return file;
}

bool Plugin::thisIsIt(const QString& fileName)
{
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

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return int(FileType::Dxf); }

std::shared_ptr<FileInterface> Plugin::createFile() { return std::make_shared<File>(); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Dxf" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Opening DXF Files" }
    };
}

std::pair<SettingsTabInterface*, QString> Plugin::createSettingsTab(QWidget* parent)
{
    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxBoldFont;
        QCheckBox* chbxItalicFont;
        QCheckBox* chbxOverrideFonts;
        QFontComboBox* fcbxDxfDefaultFont;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent)
        {
            setObjectName(QString::fromUtf8("tabDxf"));
            auto verticalLayout = new QVBoxLayout(this);
            verticalLayout->setObjectName(QString::fromUtf8("verticalLayout_9"));
            verticalLayout->setContentsMargins(6, 6, 6, 6);

            auto groupBox = new QGroupBox(this);
            groupBox->setObjectName(QString::fromUtf8("groupBox_3"));

            auto formLayout = new QFormLayout(groupBox);
            formLayout->setObjectName(QString::fromUtf8("formLayout_4"));
            formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
            formLayout->setContentsMargins(6, 6, 6, 6);
            // DefaultFont
            auto labelDefaultFont = new QLabel(groupBox);
            labelDefaultFont->setObjectName(QString::fromUtf8("labelDefaultFont"));
            formLayout->setWidget(0, QFormLayout::LabelRole, labelDefaultFont);

            fcbxDxfDefaultFont = new QFontComboBox(groupBox);
            fcbxDxfDefaultFont->setObjectName(QString::fromUtf8("fcbxDxfDefaultFont"));
            formLayout->setWidget(0, QFormLayout::FieldRole, fcbxDxfDefaultFont);
            // Bold Font
            auto labelBoldFont = new QLabel(groupBox);
            labelBoldFont->setObjectName(QString::fromUtf8("labelBoldFont"));
            formLayout->setWidget(1, QFormLayout::LabelRole, labelBoldFont);

            chbxBoldFont = new QCheckBox(groupBox);
            chbxBoldFont->setObjectName(QString::fromUtf8("chbxDxfBoldFont"));
            formLayout->setWidget(1, QFormLayout::FieldRole, chbxBoldFont);
            // Italic Font
            auto labelItalicFont = new QLabel(groupBox);
            labelItalicFont->setObjectName(QString::fromUtf8("labelItalicFont"));
            formLayout->setWidget(2, QFormLayout::LabelRole, labelItalicFont);

            chbxItalicFont = new QCheckBox(groupBox);
            chbxItalicFont->setObjectName(QString::fromUtf8("chbxDxfItalicFont"));
            formLayout->setWidget(2, QFormLayout::FieldRole, chbxItalicFont);
            // Override Fonts
            auto labelOverrideFonts = new QLabel(groupBox);
            labelOverrideFonts->setObjectName(QString::fromUtf8("labelOverrideFonts"));
            formLayout->setWidget(3, QFormLayout::LabelRole, labelOverrideFonts);

            chbxOverrideFonts = new QCheckBox(groupBox);
            chbxOverrideFonts->setObjectName(QString::fromUtf8("chbxDxfOverrideFonts"));
            formLayout->setWidget(3, QFormLayout::FieldRole, chbxOverrideFonts);

            verticalLayout->addWidget(groupBox);
            auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            verticalLayout->addItem(verticalSpacer);

            chbxBoldFont->setText(QString());
            chbxItalicFont->setText(QString());
            chbxOverrideFonts->setText(QString());

            groupBox->setTitle(QApplication::translate("SettingsDialog", "Font", nullptr));

            labelBoldFont->setText(QApplication::translate("SettingsDialog", "Bold:", nullptr));
            labelDefaultFont->setText(QApplication::translate("SettingsDialog", "Default Font:", nullptr));
            labelItalicFont->setText(QApplication::translate("SettingsDialog", "Italic:", nullptr));
            labelOverrideFonts->setText(QApplication::translate("SettingsDialog", "Override declared fonts in DXF:", nullptr));
        }
        virtual ~Tab() override { }
        virtual void readSettings(MySettings& settings) override
        {
            settings.beginGroup("Dxf");
            m_defaultFont = settings.getValue(fcbxDxfDefaultFont, "Arial");
            m_boldFont = settings.getValue(chbxBoldFont, false);
            m_italicFont = settings.getValue(chbxItalicFont, false);
            m_overrideFonts = settings.getValue(chbxOverrideFonts, false);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override
        {
            settings.beginGroup("Dxf");
            m_defaultFont = settings.setValue(fcbxDxfDefaultFont);
            m_boldFont = settings.setValue(chbxBoldFont);
            m_italicFont = settings.setValue(chbxItalicFont);
            m_overrideFonts = settings.setValue(chbxOverrideFonts);
            settings.endGroup();
        }
    };
    return { new Tab(parent), "DXF" };
}

void Plugin::updateFileModel(FileInterface* file)
{
    const auto fm = App::fileModel();
    const QModelIndex& fileIndex(file->fileIndex());
    const QModelIndex index = fm->createIndex_(0, 0, fileIndex.internalId());
    qDebug() << __FUNCTION__ << (index == fileIndex);
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
        qDebug() << __FUNCTION__ << name << layer;
        if (!layer->isEmpty())
            layers[name] = layer;
    }
    fm->beginInsertRows_(index, 0, int(layers.size() - 1));
    for (auto& [name, layer] : layers) {
        qDebug() << __FUNCTION__ << name << layer;
        fm->getItem(index)->addNode(new Dxf::NodeLayer(name, layer));
    }
    fm->endInsertRows_();
}

}
