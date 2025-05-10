/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_sourcedialog.h"

#include "abstract_file.h"
#include "app.h"
#include "project.h"

namespace Dxf {

SourceDialog::SourceDialog(int fileId, QWidget* parent)
    : QDialog{parent} {
    setObjectName(u"this"_s);
    resize(600, 600);
    // Dialog->resize(400, 300);
    auto verticalLayout = new QVBoxLayout{this};
    verticalLayout->setObjectName(u"verticalLayout"_s);
    // tableView
    auto tableView = new QTableView{this};
    QFont f(font());
    f.setFamily("Consolas");
    tableView->setFont(f);
    tableView->setObjectName(u"tableView"_s);

    tableView->setModel(new Model{App::project().file(fileId)->lines()});
    // horizontal Header
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableView->horizontalHeader()->setDefaultSectionSize(QFontMetrics(tableView->font()).size(Qt::TextSingleLine, "123456789").width());
    tableView->horizontalHeader()->setSectionResizeMode(LineData, QHeaderView::Stretch);
    // vertical Header
    tableView->verticalHeader()->setVisible(false);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableView->verticalHeader()->setDefaultSectionSize(QFontMetrics(tableView->font()).height());

    tableView->setAlternatingRowColors(true);

    //    class ItemDelegate : public QItemDelegate {
    //    public:
    //        ItemDelegate(QObject* parent = nullptr)
    //            : QItemDelegate{parent} {};

    //        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    //        {
    //            if (option.state & QStyle::State_Selected)
    //                painter->fillRect(option.rect, QColor(255, 200, 200));
    //            auto option2(option);
    //            option2.state &= ~QStyle::State_Selected;
    //            QItemDelegate::paint(painter, option2, index);
    //        }
    //        // QItemDelegate interface
    //    protected:
    //        void drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const override
    //        {
    //            if (option.state & QStyle::State_HasFocus) {
    //                painter->setBrush(Qt::NoBrush);
    //                painter->setPen(Qt::red);
    //                painter->drawRect(QRect(rect.topLeft(), rect.size() - QSize(1, 1))); //без QSize(1, 1) вылезает на сетку, не красиво.
    //            }
    //        };
    //    };
    //    tableView->setItemDelegate(new ItemDelegate{tableView});
    verticalLayout->addWidget(tableView);

    {
        auto spinBox = new QSpinBox{this};
        spinBox->setObjectName(u"spinBox"_s);
        spinBox->setRange(0, tableView->model()->rowCount());
        connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), tableView, &QTableView::selectRow);
        verticalLayout->addWidget(spinBox);
    }
    // leFind
    auto leFind = new QLineEdit{this};
    leFind->setObjectName(u"leFind"_s);
    connect(leFind, &QLineEdit::textChanged, [tableView](const QString& text) {
        for(int row = 0; row < tableView->model()->rowCount(); ++row)
            if(tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(text, Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
    });
    verticalLayout->addWidget(leFind);
    // pbNext
    auto pbNext = new QPushButton{this};
    pbNext->setObjectName(u"pbNext"_s);
    pbNext->setText(DxfObj::tr("Next"));
    connect(pbNext, &QPushButton::clicked, [tableView, leFind] {
        for(int row = tableView->currentIndex().row() + 1; row < tableView->model()->rowCount(); ++row)
            if(tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(leFind->text(), Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
    });
    verticalLayout->addWidget(pbNext);
    // pbPrev
    auto pbPrev = new QPushButton{this};
    pbPrev->setObjectName(u"pbPrev"_s);
    pbPrev->setText(DxfObj::tr("Prev"));
    connect(pbPrev, &QPushButton::clicked, [tableView, leFind] {
        for(int row = tableView->currentIndex().row() - 1; row >= 0; --row)
            if(tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(leFind->text(), Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
    });
    verticalLayout->addWidget(pbPrev);

    verticalLayout->setContentsMargins(6, 6, 6, 6);

    verticalLayout->setSpacing(6);
}

} // namespace Dxf

#include "moc_dxf_sourcedialog.cpp"
