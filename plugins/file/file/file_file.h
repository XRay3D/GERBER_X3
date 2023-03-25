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

#include "file_types.h"

#include "file.h"

namespace TmpFile {

class File : public FileInterface {
    friend class Parser;

public:
    explicit File();
    ~File() override;

    FileType type() const override { return FileType::Excellon; }

    // FileInterface interface
public:
    void createGi() override;
    void initFrom(FileInterface* file) override;
    FileTree::Node* node() override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};

} // namespace TmpFile
