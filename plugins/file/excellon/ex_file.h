/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "ex_types.h"

#include "abstract_file.h"

namespace Excellon {

class File : public AbstractFile, public QList<Hole> {
    Tools tools_;
    friend class Parser;
    Format format_;

public:

    explicit File();
    ~File() override;

    FileType type() const override { return FileType::Excellon_; }

    double tool(int t) const;
    Tools tools() const;

    Format format() const;
    void setFormat(const Format& value);

    // AbstractFile interface
    void createGi() override;
    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

} // namespace Excellon
