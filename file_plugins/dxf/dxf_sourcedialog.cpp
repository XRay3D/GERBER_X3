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
#include "dxf_sourcedialog.h"

#include "app.h"
#include "interfaces/file.h"
#include "project.h"

namespace Dxf {

SourceDialog::SourceDialog(int fileId, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QString::fromUtf8("this"));
    resize(600, 600);
    //Dialog->resize(400, 300);
    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    //tableView
    auto tableView = new QTableView(this);
    QFont f(font());
    f.setFamily("Consolas");
    tableView->setFont(f);
    tableView->setObjectName(QString::fromUtf8("tableView"));

    tableView->setModel(new Model(App::project()->file(fileId)->lines()));
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
    //            : QItemDelegate(parent) {};

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
    //    tableView->setItemDelegate(new ItemDelegate(tableView));
    verticalLayout->addWidget(tableView);

    {
        auto spinBox = new QSpinBox(this);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setRange(0, tableView->model()->rowCount());
        connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), tableView, &QTableView::selectRow);
        verticalLayout->addWidget(spinBox);
    }
    // leFind
    auto leFind = new QLineEdit(this);
    leFind->setObjectName(QString::fromUtf8("lineEdit"));
    connect(leFind, &QLineEdit::textChanged, [tableView](const QString& text) {
        for (int row = 0; row < tableView->model()->rowCount(); ++row) {
            if (tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(text, Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
        }
    });
    verticalLayout->addWidget(leFind);
    // pbNext
    auto pbNext = new QPushButton(this);
    pbNext->setObjectName(QString::fromUtf8("pbNext"));
    pbNext->setText(DxfObj::tr("Next"));
    connect(pbNext, &QPushButton::clicked, [tableView, leFind] {
        for (int row = tableView->currentIndex().row() + 1; row < tableView->model()->rowCount(); ++row) {
            if (tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(leFind->text(), Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
        }
    });
    verticalLayout->addWidget(pbNext);
    // pbPrev
    auto pbPrev = new QPushButton(this);
    pbPrev->setObjectName(QString::fromUtf8("pbPrev"));
    pbPrev->setText(DxfObj::tr("Prev"));
    connect(pbPrev, &QPushButton::clicked, [tableView, leFind] {
        for (int row = tableView->currentIndex().row() - 1; row >= 0; --row) {
            if (tableView->model()->data(tableView->model()->index(row, 2)).toString().contains(leFind->text(), Qt::CaseInsensitive)) {
                tableView->selectRow(row);
                break;
            };
        }
    });
    verticalLayout->addWidget(pbPrev);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    verticalLayout->setMargin(6);
#else
    verticalLayout->setContentsMargins(6, 6, 6, 6);
#endif

    verticalLayout->setSpacing(6);
}

}
