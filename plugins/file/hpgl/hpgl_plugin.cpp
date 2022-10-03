// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "hpgl_plugin.h"
#include "hpgl_file.h"
#include "hpgl_node.h"

#include "ft_view.h"

#include <QtWidgets>

namespace Hpgl {

Plugin::Plugin(QObject* parent)
    : QObject(parent) {
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

bool Plugin::thisIsIt(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;
    QTextStream in(&file);
    QString line(in.readLine());
    if (line.startsWith("IN;") && fileName.endsWith(".plt", Qt::CaseInsensitive))
        return true;
    return false;
}

int Plugin::type() const { return int(FileType::Hpgl); }

QString Plugin::folderName() const { return tr("Dxf Files"); }

FileInterface* Plugin::createFile() { return new File(); }

QJsonObject Plugin::info() const {
    return QJsonObject {
        {        "Name",                   "HPGL"},
        {     "Version",                    "1.0"},
        {"VendorAuthor", "X-Ray aka Bakiev Damir"},
        {        "Info",     "Opening HPGL Files"}
    };
}

SettingsTabInterface* Plugin::createSettingsTab(QWidget* parent) {
    class Tab : public SettingsTabInterface, Settings {
        QCheckBox* chbxBoldFont;
        QCheckBox* chbxItalicFont;
        QCheckBox* chbxOverrideFonts;
        QFontComboBox* fcbxDxfDefaultFont;

    public:
        Tab(QWidget* parent = nullptr)
            : SettingsTabInterface(parent) {
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

            chbxBoldFont = new QCheckBox(" ", groupBox);
            chbxBoldFont->setObjectName(QString::fromUtf8("chbxDxfBoldFont"));
            formLayout->setWidget(1, QFormLayout::FieldRole, chbxBoldFont);
            // Italic Font
            auto labelItalicFont = new QLabel(groupBox);
            labelItalicFont->setObjectName(QString::fromUtf8("labelItalicFont"));
            formLayout->setWidget(2, QFormLayout::LabelRole, labelItalicFont);

            chbxItalicFont = new QCheckBox(" ", groupBox);
            chbxItalicFont->setObjectName(QString::fromUtf8("chbxDxfItalicFont"));
            formLayout->setWidget(2, QFormLayout::FieldRole, chbxItalicFont);
            // Override Fonts
            auto labelOverrideFonts = new QLabel(groupBox);
            labelOverrideFonts->setObjectName(QString::fromUtf8("labelOverrideFonts"));
            formLayout->setWidget(3, QFormLayout::LabelRole, labelOverrideFonts);

            chbxOverrideFonts = new QCheckBox(" ", groupBox);
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
        virtual void readSettings(MySettings& settings) override {
            settings.beginGroup("Dxf");
            m_defaultFont = settings.getValue(fcbxDxfDefaultFont, "Arial");
            m_boldFont = settings.getValue(chbxBoldFont, false);
            m_italicFont = settings.getValue(chbxItalicFont, false);
            m_overrideFonts = settings.getValue(chbxOverrideFonts, false);
            settings.endGroup();
        }
        virtual void writeSettings(MySettings& settings) override {
            settings.beginGroup("Dxf");
            m_defaultFont = settings.setValue(fcbxDxfDefaultFont);
            m_boldFont = settings.setValue(chbxBoldFont);
            m_italicFont = settings.setValue(chbxItalicFont);
            m_overrideFonts = settings.setValue(chbxOverrideFonts);
            settings.endGroup();
        }
    };
    //    auto tab = new Tab(parent);
    //    tab->setWindowTitle("HPGL");
    return nullptr;
}

void Plugin::updateFileModel(FileInterface* file) {
    const auto fm = App::fileModel();
    const QModelIndex& fileIndex(file->node()->index());
    const QModelIndex index = fm->createIndex_(0, 0, fileIndex.internalId());
    // clean before insert new layers
    //    if (int count = fm->getItem(fileIndex)->childCount(); count) {
    //        fm->beginRemoveRows_(index, 0, count - 1);
    //        auto item = fm->getItem(index);
    //        do {
    //            item->remove(--count);
    //        } while (count);
    //        fm->endRemoveRows_();
    //    }
    //    Hpgl::Layers layers;
    //    for (auto& [name, layer] : reinterpret_cast<File*>(file)->layers()) {
    //        qDebug() << name << layer;
    //        if (!layer->isEmpty())
    //            layers[name] = layer;
    //    }
    //    fm->beginInsertRows_(index, 0, int(layers.size() - 1));
    //    for (auto& [name, layer] : layers) {
    //        qDebug() << name << layer;
    //        fm->getItem(index)->addChild(new Hpgl::NodeLayer(name, layer));
    //    }
    //    fm->endInsertRows_();
}

} // namespace Hpgl

#include "moc_hpgl_plugin.cpp"
