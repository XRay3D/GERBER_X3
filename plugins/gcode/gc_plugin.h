#pragma once

#include "abstract_fileplugin.h"

class QAction;
class QMenu;
class QToolBar;
class QWidget;
class AbstractFile;

namespace GCode {

class Plugin : public AbstractFilePlugin {
    Q_OBJECT

public:
    explicit Plugin(QObject* parent = nullptr);
    virtual ~Plugin() = default;

    //    [[nodiscard]] virtual GCode::File* loadFile(QDataStream& stream) const = 0;
    //    [[nodiscard]] virtual QIcon icon() const = 0;
    //    [[nodiscard]] virtual uint32_t type() const = 0;
    [[nodiscard]] virtual QKeySequence keySequence() const = 0;
    [[nodiscard]] virtual QWidget* createForm() = 0;
    [[nodiscard]] virtual bool canToShow() const { return true; }
    [[nodiscard]] virtual QAction* addAction(QMenu* menu, QToolBar* toolbar);

    //////////////////////

    //    AbstractFile* loadFile(QDataStream& stream) constoverride { return nullptr /*new File()*/; }
    //    QIcon icon() const override { return decoration(Qt::lightGray, 'G'); }
    //    uint32_t type() const override { return md5::hash32("GCode"); }
    //    AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    QString folderName() const override { return tr("Tool Paths"); }
    bool thisIsIt(const QString& fileName) override { return false; }
    void createMainMenu(QMenu& menu, FileTree::View* tv) override;

    AbstractFile* parseFile(const QString& fileName, int type) override { return nullptr; }
public slots:

signals:
    void setDockWidget(QWidget* w);

protected:
    enum { IconSize = 24 };
};

} // namespace GCode

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCode.Plugin"

Q_DECLARE_INTERFACE(GCode::Plugin, GCodeInterface_iid)
