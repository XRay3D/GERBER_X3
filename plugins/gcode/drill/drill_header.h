#pragma once

#include "drill_form.h"
#include "qwidget.h"

class Header : public QHeaderView {
    Q_OBJECT

public:
    Header(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~Header() override;

    enum {
        XOffset = 5,
        DelegateSize = 16
    };

    void setAll(bool ch);
    void togle(int index);
    void set(int index, bool ch);
    static QRect getRect(const QRect& rect);

signals:
    void onChecked(int = -1);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

private:
    int flag = Qt::Unchecked;
    mutable mvector<QRect> checkRect_;
    void setChecked(int index, bool ch);
    bool checked(int index) const;
    DrillModel* model() const;
};
