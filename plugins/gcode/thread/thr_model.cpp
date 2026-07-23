/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thr_model.h"
#include "thr_form.h"
#include "thr_gi_preview.h"
#include "thr_header.h"
#include "tool_pch.h"

#include <QBitmap>
#include <QDebug>
#include <QPainter>

namespace Threading {

Model::Model(size_t rowCount, QObject* parent)
    : QAbstractTableModel{parent} {
    data_.resize(rowCount);
}

void Model::setToolId(int row, Tool::ID id) {
    if(data_[row].toolId != id)
        data_[row].useForCalc = id > Tool::ID::Null;
    data_[row].toolId = id;
    for(auto item: data_[row].items) {
        // Флаг должен быть выставлен ДО updateTool(), т.к. тот в конце вызывает
        // changeColor(), которая читает ItemIsSelectable как признак "Used" —
        // иначе цвет пересчитывается по ещё не обновлённому (старому) флагу.
        item->setFlag(QGraphicsItem::ItemIsSelectable, id > Tool::ID::Null);
        item->updateTool();
    }
    emit set(row, id > Tool::ID::Null);
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
}

void Model::setCreate(int row, bool create) {
    if(data_[row].toolId == Tool::ID::Null)
        return;
    data_[row].useForCalc = create;
    for(auto item: data_[row].items) {
        item->setFlag(QGraphicsItem::ItemIsSelectable, create);
        item->changeColor();
    }
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    emit headerDataChanged(Qt::Vertical, row, row);
}

void Model::setCreate(bool create) {
    for(int row{}; row < rowCount(); ++row)
        data_[row].useForCalc = create && data_[row].toolId != Tool::ID::Null;
    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 1));
}

int Model::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(data_.size()); }

int Model::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant Model::data(const QModelIndex& index, int role) const {
    int row = index.row();
    if(index.column() == Name) {
        switch(role) {
        case Qt::DisplayRole:
            return data_[row].name.back();
        case Qt::DecorationRole: {
            if(data_[index.row()].toolId > Tool::ID::Null) {
                return data_[row].icon;
            } else {
                QImage image(data_[row].icon.pixmap(24, 24).toImage());
                for(int x{}; x < 24; ++x)
                    for(int y{}; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(100, 100, 100, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            }
        }
        case Qt::UserRole: return row;
        default          : break;
        }
    } else {
        if(data_[row].toolId == Tool::ID::Null)
            switch(role) {
            case Qt::DisplayRole      : return tr("Select Tool");
            case Qt::TextAlignmentRole: return Qt::AlignCenter;
            case Qt::UserRole         : return +data_[row].toolId;
            default                   : break;
            }
        else
            switch(role) {
            case Qt::DisplayRole   : return App::toolHolder().tool(data_[row].toolId).name();
            case Qt::DecorationRole: return App::toolHolder().tool(data_[row].toolId).icon();
            case Qt::UserRole      : return +data_[row].toolId;
            default                : break;
            }
    }
    return {};
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    switch(role) {
    case Qt::DisplayRole:
        if(orientation == Qt::Horizontal) {
            switch(section) {
            case Name: return {tr("Aperture") + u" / "_s + tr("Tool")};
            case Tool:;
                return tr("Tool");
            }
        }
        return data_[section].name.value(0);
    case Qt::SizeHintRole:
        if(orientation == Qt::Vertical)
            return QFontMetrics(QFont()).boundingRect(u"T999"_s).size() + QSize(Header::DelegateSize + 10, 1);
        return {};
    case Qt::TextAlignmentRole:
        if(orientation == Qt::Vertical)
            return static_cast<int>(Qt::AlignRight) | static_cast<int>(Qt::AlignVCenter);
        return Qt::AlignCenter;
    default: return {};
    }
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    if(index.column() == Name)
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

Row::~Row() { qDeleteAll(items); }

} // namespace Threading

#include "moc_thr_model.cpp"
