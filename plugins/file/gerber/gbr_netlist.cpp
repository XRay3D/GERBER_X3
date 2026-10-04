/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  October 04, 2026                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_netlist.h"
#include "gbr_file.h"
#include "gi.h"
#include "graphicsview.h"
#include "project.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace Gerber {

namespace {
enum Column {
    NetName,
    Objects,
};
constexpr int NetIdRole = Qt::UserRole;
QPointer<NetList> form;
} // namespace

NetList* NetList::existing() { return form; }

NetList* NetList::instance() {
    if(!form) {
        form = new NetList;
        // Отвязанная доком форма висит без родителя -- прибрать её до
        // разрушения QApplication; в доке QPointer обнулится сам.
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, [] { delete form.data(); });
    }
    return form;
}

NetList::NetList(QWidget* parent)
    : QWidget{parent}
    , fileLabel_{new QLabel{this}}
    , filter_{new QLineEdit{this}}
    , tree_{new QTreeWidget{this}} {
    setObjectName(u"GerberNetList"_s);
    setWindowTitle(GbrObj::tr("Netlist"));

    fileLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    filter_->setPlaceholderText(GbrObj::tr("Filter"));
    filter_->setClearButtonEnabled(true);
    connect(filter_, &QLineEdit::textChanged, this, &NetList::applyFilter);

    tree_->setColumnCount(2);
    tree_->setHeaderLabels({GbrObj::tr("Net"), GbrObj::tr("Objects")});
    tree_->setRootIsDecorated(false);
    tree_->setUniformRowHeights(true);
    tree_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree_->setSortingEnabled(true);
    tree_->sortByColumn(NetName, Qt::AscendingOrder);
    tree_->header()->setSectionResizeMode(NetName, QHeaderView::Stretch);
    tree_->header()->setSectionResizeMode(Objects, QHeaderView::ResizeToContents);
    tree_->header()->setStretchLastSection(false);
    tree_->setToolTip(GbrObj::tr("Double click zooms to the selected nets"));
    connect(tree_, &QTreeWidget::itemSelectionChanged, this, &NetList::syncSelection);
    connect(tree_, &QTreeWidget::itemDoubleClicked, this, &NetList::fitSelection);

    auto* layout = new QVBoxLayout{this};
    layout->setContentsMargins(6, 6, 6, 6);
    layout->addWidget(fileLabel_);
    layout->addWidget(filter_);
    layout->addWidget(tree_);
}

void NetList::setFile(const File* file) {
    // Сначала снять выделение прежнего файла -- через тот же путь, что и
    // пользовательское снятие строк.
    tree_->clearSelection();
    tree_->clear();
    fileId_ = file ? file->id() : -1;
    fileLabel_->setText(file ? file->shortName() : QString{});
    fileLabel_->setToolTip(file ? file->name() : QString{});
    if(!file) return;

    std::vector<int> objects(file->nets().size());
    for(const GrObject& go: file->graphicObjects2())
        if(go.state.net() >= 0) ++objects[go.state.net()];

    QList<QTreeWidgetItem*> rows;
    rows.reserve(file->nets().size());
    for(int32_t id{}; const QString& name: file->nets()) {
        auto* row = new QTreeWidgetItem;
        row->setText(NetName, name);
        row->setData(NetName, NetIdRole, id);
        row->setData(Objects, Qt::DisplayRole, objects[id]);
        row->setTextAlignment(Objects, Qt::AlignRight | Qt::AlignVCenter);
        rows.push_back(row);
        ++id;
    }
    tree_->addTopLevelItems(rows);
    applyFilter(filter_->text());
}

void NetList::fileClosed(int32_t fileId) {
    if(fileId != fileId_) return;
    // Элементы уже ушли вместе с файлом -- снимать на сцене нечего.
    const QSignalBlocker blocker{tree_};
    tree_->clear();
    fileId_ = -1;
    fileLabel_->clear();
}

void NetList::syncSelection() {
    auto* file = App::project().file<File>(fileId_);
    auto* scene = App::grView().scene();
    if(!file || !scene) return;
    // Выделение сцены -- ровно выбранные цепи: снятая строка снимает свои
    // элементы, а заодно и всё, что было выделено на сцене до того. Скрытые
    // элементы (группа другого вида отображения) Qt не выделит сам.
    scene->clearSelection();
    for(const QTreeWidgetItem* row: tree_->selectedItems())
        for(Gi::Item* item: file->netItems(row->data(NetName, NetIdRole).toInt()))
            item->setSelected(true);
}

void NetList::fitSelection() {
    auto* file = App::project().file<File>(fileId_);
    if(!file) return;
    QRectF rect;
    for(const QTreeWidgetItem* row: tree_->selectedItems())
        for(const Gi::Item* item: file->netItems(row->data(NetName, NetIdRole).toInt()))
            if(item->isVisible()) rect |= item->sceneBoundingRect();
    if(!rect.isEmpty()) App::grView().fitInView(rect);
}

void NetList::applyFilter(const QString& text) {
    for(int i{}; i < tree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem* row = tree_->topLevelItem(i);
        row->setHidden(!text.isEmpty() && !row->text(NetName).contains(text, Qt::CaseInsensitive));
    }
}

} // namespace Gerber

#include "moc_gbr_netlist.cpp"
