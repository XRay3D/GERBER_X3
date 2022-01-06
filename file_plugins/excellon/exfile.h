/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "extypes.h"

#include "interfaces/file.h"

namespace Excellon {

class File : public FileInterface, public QList<Hole> {
    Tools m_tools;
    friend class Parser;
    Format m_format;

public:
    explicit File();
    ~File() override;

    FileType type() const override { return FileType::Excellon; }

    double tool(int t) const;
    Tools tools() const;

    Format format() const;
    void setFormat(const Format& value);

    // FileInterface interface
public:
    void createGi() override;
    void initFrom(FileInterface* file) override;
    FileTree::Node* node()  override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Paths merge() const override;
};
} // namespace Excellon
