#pragma once

#include "app.h"
#include <QAction>
#include <QIcon>
#include <QJsonObject>
#include <QMenu>
#include <QObject>
#include <QToolBar>

class QAction;
class QMenu;
class QToolBar;
class QWidget;

class GCodePlugin : public QObject {
    Q_OBJECT

public:
    explicit GCodePlugin(QObject* parent = nullptr)
        : QObject { parent } { App app; }
    virtual ~GCodePlugin() { }
    [[nodiscard]] virtual QIcon icon() const = 0;
    [[nodiscard]] virtual QKeySequence keySequence() const = 0;
    [[nodiscard]] virtual QWidget* createForm() const = 0;
    [[nodiscard]] virtual bool canToShow() const { return true; }
    [[nodiscard]] virtual int type() const = 0;

    [[nodiscard]] QAction* addAction(QMenu* menu, QToolBar* toolbar) {
        auto action = toolbar->addAction(icon(), info()["Name"].toString());
        connect(action, &QAction::toggled, [=, this](bool checked) {
            qDebug() << sender() << action->isChecked() << this << checked;
            if (!action->isChecked() && canToShow())
                emit setDockWidget(createForm());
        });
        action->setShortcut(keySequence());
        menu->addAction(action);
        return action;
    }

    const QJsonObject& info() const { return info_; }
    void setInfo(const QJsonObject& info) { info_ = info; }

signals:
    void setDockWidget(QWidget* w);

protected:
    QJsonObject info_;
    enum { IconSize = 24 };
};

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCodePlugin"

Q_DECLARE_INTERFACE(GCodePlugin, GCodeInterface_iid)
