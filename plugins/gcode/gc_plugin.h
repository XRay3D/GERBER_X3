#pragma once

#include "app.h"

#include <QJsonObject>

class QAction;
class QMenu;
class QToolBar;
class QWidget;

class GCodePlugin : public QObject {
    Q_OBJECT

public:
    explicit GCodePlugin(QObject* parent = nullptr);
    virtual ~GCodePlugin() = default;

    [[nodiscard]] virtual QIcon icon() const = 0;
    [[nodiscard]] virtual QKeySequence keySequence() const = 0;
    [[nodiscard]] virtual QWidget* createForm()  = 0;
    [[nodiscard]] virtual bool canToShow() const;
    [[nodiscard]] virtual int type() const = 0;

    [[nodiscard]] QAction* addAction(QMenu* menu, QToolBar* toolbar);

    const QJsonObject& info() const;
    void setInfo(const QJsonObject& info);

signals:
    void setDockWidget(QWidget* w);

protected:
    QJsonObject info_;
    enum { IconSize = 24 };

private:
    bool fl {};
};

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCodePlugin"

Q_DECLARE_INTERFACE(GCodePlugin, GCodeInterface_iid)
