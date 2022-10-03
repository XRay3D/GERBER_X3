// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "drill_header.h"
#include "drill_model.h"

#include <QTableView>

namespace DrillPlugin {

Header::Header(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent) {
    connect(this, &QHeaderView::sectionCountChanged, [this](int /*oldCount*/, int newCount) {
        checkRect_.resize(newCount);
    });
    setSectionsClickable(true);
    setHighlightSections(true);
}

Header::~Header() { }

void Header::setAll(bool ch) {
    for (int i = 0; i < count(); ++i) {
        if (checked(i) != ch) {
            setChecked(i, ch);
            updateSection(i);
        }
    }
    emit onChecked();
}

void Header::togle(int index) {
    setChecked(index, !checked(index));
    updateSection(index);
    emit onChecked(index);
}

void Header::set(int index, bool ch) {
    setChecked(index, ch);
    updateSection(index);
    emit onChecked(index);
}

QRect Header::getRect(const QRect& rect) {
    return QRect(
        rect.left() + XOffset,
        rect.top() + (rect.height() - DelegateSize) / 2,
        DelegateSize,
        DelegateSize);
}

void Header::mouseMoveEvent(QMouseEvent* event) {
    static int index = 0;
    do {

        if (index == logicalIndexAt(event->pos()))
            break;
        index = logicalIndexAt(event->pos());
        if (index < 0)
            break;
        if (event->buttons() != Qt::RightButton)
            break;
        if (orientation() == Qt::Horizontal) {
            // setSingle(index);
        } else
            togle(index);
        event->accept();
        return;
    } while (0);
    QHeaderView::mouseMoveEvent(event);
}

void Header::mousePressEvent(QMouseEvent* event) {
    int index = logicalIndexAt(event->pos());
    do {
        if (index < 0)
            break;
        if (!checkRect_[index].contains(event->pos()) && event->buttons() != Qt::RightButton)
            break;
        togle(index);
        event->accept();
        return;
    } while (0);
    QHeaderView::mousePressEvent(event);
}

void Header::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    QStyleOptionButton option;
    checkRect_[logicalIndex] = option.rect = getRect(rect);

    option.state = checked(logicalIndex) ? QStyle::State_On : QStyle::State_Off;
    option.state |= model()->toolId(logicalIndex) != -1 && isEnabled() ? QStyle::State_Enabled : QStyle::State_None;

    style()->drawPrimitive(orientation() == Qt::Horizontal ? QStyle::PE_IndicatorRadioButton : QStyle::PE_IndicatorCheckBox, &option, painter);
}

void Header::setChecked(int index, bool ch) { model()->setCreate(index, ch); }

bool Header::checked(int index) const { return model()->useForCalc(index); }

Model* Header::model() const { return static_cast<Model*>(static_cast<QTableView*>(parent())->model()); }

} // namespace DrillPlugin
#include "moc_drill_header.cpp"
