// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_drillmodel.h"
#include "gc_drillform.h"
#include "gc_gidrillpreview.h"
#include "gc_header.h"
#include "tool_pch.h"

#include <QBitmap>
#include <QDebug>
#include <QPainter>

DrillModel::DrillModel(QString type, int rowCount, QObject* parent)
    : QAbstractTableModel(parent)
    , type(type) {
    data_.reserve(rowCount);
}

DrillModel::DrillModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

void DrillModel::setToolId(int row, int id) {
    if (data_[row].toolId != id)
        data_[row].useForCalc = id > -1;
    data_[row].toolId = id;
    for (auto item : data_[row].items) {
        item->updateTool();
        item->setFlag(QGraphicsItem::ItemIsSelectable, id > -1);
    }
    emit set(row, id > -1);
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
}

int DrillModel::toolId(int row) { return data_[row].toolId; }

bool DrillModel::isSlot(int row) { return data_[row].isSlot; }

int DrillModel::apertureId(int row) { return data_[row].apertureId; }

bool DrillModel::useForCalc(int row) const { return data_[row].useForCalc; }

void DrillModel::setCreate(int row, bool create) {
    if (data_[row].toolId == -1)
        return;
    data_[row].useForCalc = create;
    for (auto item : data_[row].items) {
        item->setFlag(QGraphicsItem::ItemIsSelectable, create);
        item->changeColor();
    }
    emit dataChanged(createIndex(row, 0), createIndex(row, 1));
    emit headerDataChanged(Qt::Vertical, row, row);
}

void DrillModel::setCreate(bool create) {
    for (int row = 0; row < rowCount(); ++row) {
        data_[row].useForCalc = create && data_[row].toolId != -1;
    }
    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, 1));
}

int DrillModel::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(data_.size()); }

int DrillModel::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant DrillModel::data(const QModelIndex& index, int role) const {
    int row = index.row();
    if (index.column() == Name) {
        switch (role) {
        case Qt::DisplayRole:
            if (data_[row].isSlot)
                return QString(data_[row].name).replace(tr("Tool"), tr("Slot"));
            else
                return data_[row].name;
        case Qt::DecorationRole: {
            if (data_[index.row()].toolId > -1 && data_[row].isSlot) {
                QImage image(data_[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(255, 0, 0, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            } else if (data_[index.row()].toolId > -1) {
                return data_[row].icon;
            } else if (data_[row].isSlot) {
                QImage image(data_[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(255, 100, 100, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            } else {
                QImage image(data_[row].icon.pixmap(24, 24).toImage());
                for (int x = 0; x < 24; ++x)
                    for (int y = 0; y < 24; ++y)
                        image.setPixelColor(x, y, QColor(100, 100, 100, image.pixelColor(x, y).alpha()));
                return QIcon(QPixmap::fromImage(image));
            }
        }
        case Qt::UserRole:
            return data_[row].apertureId;
        default:
            break;
        }
    } else {
        if (data_[row].toolId == -1)
            switch (role) {
            case Qt::DisplayRole:
                return tr("Select Tool");
            case Qt::TextAlignmentRole:
                return Qt::AlignCenter;
            case Qt::UserRole:
                return data_[row].toolId;
            default:
                break;
            }
        else
            switch (role) {
            case Qt::DisplayRole:
                return App::toolHolder().tool(data_[row].toolId).name();
            case Qt::DecorationRole:
                return App::toolHolder().tool(data_[row].toolId).icon();
            case Qt::UserRole:
                return data_[row].toolId;
            default:
                break;
            }
    }
    return QVariant();
}

QVariant DrillModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case Name:
                // BUG return type == tAperture ? tr("Aperture") : tr("Tool");
            case Tool:;
                // BUG return tr("Tool");
            }
        }
        return {};
        // BUG return QString(m_type == tAperture ? "D%1" : "T%1").arg(m_data[section].apertureId);
    case Qt::SizeHintRole:
        if (orientation == Qt::Vertical)
            return QFontMetrics(QFont()).boundingRect(QString("T999")).size() + QSize(Header::DelegateSize + 10, 1);
        return QVariant();
    case Qt::TextAlignmentRole:
        if (orientation == Qt::Vertical)
            return static_cast<int>(Qt::AlignRight) | static_cast<int>(Qt::AlignVCenter);
        return Qt::AlignCenter;
    default:
        return QVariant();
    }
}

Qt::ItemFlags DrillModel::flags(const QModelIndex& index) const {
    if (index.column() == Name)
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
