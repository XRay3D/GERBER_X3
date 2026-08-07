/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "abstract_file.h"
#include "svg_types.h"

#include <QList>

namespace Svg {

class File : public AbstractFile, public QList<SvgElement> {
    friend class Parser;

public:
    explicit File();
    ~File() override = default;

    uint32_t type() const override { return SVG; }
    QIcon icon() const override { return QIcon::fromTheme(u"image-svg+xml"_s); }

    void createGi() override;
    void initFrom(AbstractFile* file) override;
    FileTree::Node* node() override;

protected:
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
    Curves merge() const override;
};

} // namespace Svg
