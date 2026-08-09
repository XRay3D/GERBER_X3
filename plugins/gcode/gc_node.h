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

#include "ft_node.h"

class AbstractFile;

namespace GCode {

class Node : public FileTree::Node {
    friend class ::AbstractFile;
    AbstractFile* file;

public:
    explicit Node(AbstractFile* file);
    ~Node() override;

    // Узел переживает замену УП на месте: объект файла меняется, а строка в
    // дереве остаётся той же. Без перепривязки внутри остался бы указатель на
    // уже удалённый файл.
    void setFile(AbstractFile* newFile) { file = newFile; }

    // FileTree::Node interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) override;
    bool doubleClicked(FileTree::View* tv) override;
    virtual int32_t id() const override;
};

} // namespace GCode
