#pragma once

#include "gc_drillform.h"
#include "qtmetamacros.h"
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
    mutable mvector<QRect> m_checkRect;
    void setChecked(int index, bool ch);
    bool checked(int index) const;
    DrillModel* model() const;
};

class GCPluginImpl : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "drill.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    QKeySequence keySequence() const override { return { "Ctrl+Shift+D" }; }
    QWidget* createForm() override { return new DrillForm(this); };
    bool canToShow() const override { return DrillForm::canToShow(); }
    int type() const override { return GCode::Drill; }
};
