#pragma once

#include "plugindata.h"

class QAction;
class QMenu;
class QToolBar;
class QWidget;
class AbstractFile;

class GCodePlugin : public QObject, public PluginData {
    Q_OBJECT

public:
    explicit GCodePlugin(QObject* parent = nullptr);
    virtual ~GCodePlugin() = default;

    [[nodiscard]] virtual QIcon icon() const = 0;
    [[nodiscard]] virtual QKeySequence keySequence() const = 0;
    [[nodiscard]] virtual QWidget* createForm() = 0;
    [[nodiscard]] virtual bool canToShow() const;
    [[nodiscard]] virtual uint32_t type() const = 0;
    [[nodiscard]] virtual AbstractFile* loadFile(QDataStream& stream) const = 0;

    [[nodiscard]] QAction* addAction(QMenu* menu, QToolBar* toolbar);

signals:
    void setDockWidget(QWidget* w);

protected:
    enum { IconSize = 24 };
};

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCodePlugin"

Q_DECLARE_INTERFACE(GCodePlugin, GCodeInterface_iid)
