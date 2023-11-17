/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gc_baseform.h"
#include "gc_plugin.h"
#include "thread.h"
#include <QToolBar>
#include <array>

namespace Ui {
class ThreadForm;
}
class GiBridge;

namespace Thread {

class Form : public GCode::BaseForm {
    Q_OBJECT

public:
    explicit Form(GCode::Plugin* plugin, QWidget* parent = nullptr);
    ~Form() override;

private slots:
    void onNameTextChanged(const QString& arg1);

private:
    void updatePixmap();
    void rb_clicked();

    Ui::ThreadForm* ui;
    //    GiBridge* brItem = nullptr;

    const QStringList names{tr("Thread On"), tr("Thread Outside"), tr("Thread Inside")};
    static inline const std::array pixmaps{
        u"prof_on_climb"_qs,
        u"prof_out_climb"_qs,
        u"prof_in_climb"_qs,
        u"prof_on_conv"_qs,
        u"prof_out_conv"_qs,
        u"prof_in_conv"_qs,
    };

    enum Trimming {
        Line = 1,
        Corner = 2,
    };

    enum BridgeAlign {
        Manually,
        Horizontally,
        Vertically,
        HorizontallyVertically,
        ThroughTheDistance,
        EvenlyDround,
    };

    void updateBridgePos(QPointF pos);

    int trimming_ = 0;

protected:
    // QWidget interface
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    // FormsUtil interface
    void computePaths() override;
    void updateName() override;

public:
    void editFile(GCode::File* file) override;
};

class GCPluginImpl final : public GCode::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "thread.json")
    Q_INTERFACES(GCode::Plugin)
    Form form{this};

public:
    // GCode::Plugin interface
    QIcon icon() const override { return QIcon::fromTheme("crosshairs"); } // FIXME
    QKeySequence keySequence() const override { return {"Ctrl+Shift+T"}; }
    QWidget* createForm() override { return &form; };
    uint32_t type() const override { return THREAD; }
    AbstractFile* /*GCode::File*/ loadFile(QDataStream& stream) const override { return File::load<File>(stream); }

    // AbstractFilePlugin interface
    AbstractFileSettings* createSettingsTab(QWidget* parent) override {
        class Tab : public AbstractFileSettings {
            //            QComboBox* cbxThreadSort;
            QTableView* tableView;

            class Model : public QAbstractTableModel {
            public:
                Model(QWidget* parent)
                    : QAbstractTableModel{parent} { }
                virtual ~Model() { }

                // QAbstractItemModel interface
                int rowCount(const QModelIndex& parent) const override { return Settings::threads.size(); }
                int columnCount(const QModelIndex& parent) const override { return 5; }

                QVariant data(const QModelIndex& index, int role) const override {
                    auto& data = Settings::threads.at(index.row());
                    if (role == Qt::DisplayRole || role == Qt::EditRole) {
                        switch (index.column()) {
                        case 0: return data.D;
                        case 1: return data.P;
                        case 2: return data.D1;
                        case 3: return data.D2;
                        case 4: return data.D3;
                        }
                    }

                    if (role == Qt::TextAlignmentRole) return Qt::AlignCenter;

                    return {};
                }

                bool setData(const QModelIndex& index, const QVariant& value, int role) override {
                    auto& data = Settings::threads.at(index.row());
                    if (role == Qt::EditRole) {
                        switch (index.column()) {
                        case 0:
                            data.D = value.toDouble();
                            data.D2 = data.D - 0.6495 * data.P;
                            data.D1 = data.D - 1.0825 * data.P;
                            data.D3 = data.D - 1.2267 * data.P;
                            emit dataChanged(index.siblingAtColumn(3), index.siblingAtColumn(5));
                            return true;
                        case 1:
                            data.P = value.toDouble();
                            data.D2 = data.D - 0.6495 * data.P;
                            data.D1 = data.D - 1.0825 * data.P;
                            data.D3 = data.D - 1.2267 * data.P;
                            emit dataChanged(index.siblingAtColumn(3), index.siblingAtColumn(5));
                            return true;
                        case 2:
                            data.D1 = value.toDouble();
                            return true;
                        case 3:
                            data.D2 = value.toDouble();
                            return true;
                        case 4:
                            data.D3 = value.toDouble();
                            return true;
                        }
                    }
                    return {};
                }

                QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
                    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                        switch (section) {
                        case 0: return tr("Nominal\nD, mm");
                        case 1: return tr("Pitch\nP, mm");
                        case 2: return tr("Minor\nD1, mm");
                        case 3: return tr("Pitch\nD2, mm");
                        case 4: return tr("D3, mm");
                        }
                    }
                    return QAbstractTableModel::headerData(section, orientation, role);
                }

                Qt::ItemFlags flags(const QModelIndex& index) const override {
                    return /*QAbstractTableModel::flags(index) |*/ Qt::ItemIsEditable | Qt::ItemIsEnabled;
                }

                void add() {
                    beginInsertRows({}, Settings::threads.size(), Settings::threads.size());
                    Settings::threads.emplace_back(-1.0);
                    endInsertRows();
                }
            };

            class Delegate : public QStyledItemDelegate {
            public:
                Delegate(QWidget* parent)
                    : QStyledItemDelegate{parent} { }

                QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
                    auto dsbx = new QDoubleSpinBox{parent};
                    dsbx->setDecimals(3);
                    return dsbx;
                }
            };

        public:
            Tab(QWidget* parent)
                : AbstractFileSettings{parent} {
                setWindowTitle(tr("Thread"));

                tableView = new QTableView{this};
                auto model = new Model{tableView};
                tableView->setModel(model);
                tableView->setItemDelegate(new Delegate{tableView});

                tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

                auto pbAdd = new QPushButton{tr("Add New"), this};
                connect(pbAdd, &QPushButton::clicked, model, &Model::add);
                connect(pbAdd, &QPushButton::clicked, tableView, &QTableView::scrollToBottom);

                auto layout = new QVBoxLayout{this};
                layout->setContentsMargins(6, 6, 6, 6);
                layout->addWidget(tableView);
                layout->addWidget(pbAdd);
            }
            ~Tab() override = default;

            void readSettings(MySettings& settings) override {
                // settings.sort = settings.getValue(cbxThreadSort, settings.sort);
                QFile file{App::settingsPath() + "/threads.json"};
                if (file.open(QFile::ReadOnly | QFile::Text)) {
                    auto array = QJsonDocument::fromJson(file.readAll()).array();
                    Settings::threads.clear();
                    Settings::threads.reserve(array.size());
                    for (auto objRef: array)
                        Settings::threads.emplace_back(-1.0) = objRef.toObject();
                } else {
                    qWarning() << file.errorString();
                }
                if (!Settings::threads.size()) {
                    Settings::threads = {
                        {80,   6, 76.103, 73.505, 72.639},
                        {80,   4, 77.402, 75.670, 75.093},
                        {80,   3, 78.051, 76.752, 76.319},
                        {80,   2, 78.701, 77.835, 77.546},
                        {80, 1.5, 79.026, 78.376, 78.160},
                        {80,   1, 79.350, 78.917, 78.773},
                    };
                }
            }
            void writeSettings(MySettings& settings) override {
                QFile file{App::settingsPath() + "/threads.json"};
                if (file.open(QFile::WriteOnly | QFile::Text)) {
                    QJsonArray arr;
                    for (auto& thread: Settings::threads)
                        arr.append(thread.toObj());
                    file.write(QJsonDocument(arr).toJson());
                } else {
                    qWarning() << file.errorString();
                }
            }
        };
        return new Tab{parent};
    }
};

} // namespace Thread
