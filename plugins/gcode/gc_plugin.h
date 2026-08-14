/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "ggcore_export.h"

#include "abstract_fileplugin.h"

class QAction;
class QMenu;
class QToolBar;
class QWidget;
class AbstractFile;

namespace GCode {

class GGCORE_EXPORT Plugin : public AbstractFilePlugin {
    Q_OBJECT

public:
    explicit Plugin(QObject* parent = nullptr);
    virtual ~Plugin() = default;

    // [[nodiscard]] virtual QIcon icon() const = 0;
    // [[nodiscard]] virtual uint32_t type() const = 0;
    [[nodiscard]] virtual QKeySequence keySequence() const = 0;
    [[nodiscard]] virtual QWidget* createForm() = 0;
    [[nodiscard]] virtual QString gcName() const = 0;
    [[nodiscard]] virtual bool canToShow() const { return true; }
    [[nodiscard]] virtual QAction* addAction(QMenu* menu, QToolBar* toolbar);

    // Показать форму плагина и заполнить её параметрами уже созданной УП.
    // Метод, а не сигнал: setDockWidget снаружи класса не испустить.
    void editFile(File* file);

    //////////////////////

    // QIcon icon() const override { return decoration(Qt::lightGray, u'G'); }
    // uint32_t type() const override { return "GCode"_hash32; }
    // AbstractFileSettings* createSettingsTab(QWidget* parent) override;
    QString folderName() const override { return tr("Tool Paths"); }
    bool thisIsIt(const QString& /*fileName*/) override { return false; }
    void createMainMenu(QMenu& menu, FileTree::View* tv) override;
    void updateFileModel(AbstractFile* file) override;

    AbstractFile* parseFile(const QString& /*fileName*/, uint32_t /*type*/) override { return nullptr; }
public slots:

signals:
    void setDockWidget(QWidget* w);

protected:
    enum {
        IconSize = 24
    };
};

} // namespace GCode

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCode.Plugin"

Q_DECLARE_INTERFACE(GCode::Plugin, GCodeInterface_iid)
