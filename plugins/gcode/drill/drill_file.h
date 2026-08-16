/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  ХХ ХХХ 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gc_file.h"
#include "project.h"

namespace Drilling {

constexpr auto DRILLING = "Drilling"_hash32;

// Строка таблицы инструментов, из которой посчитана УП. Ключ -- ИМЯ: индекс
// строки зависит от порядка апертур в файле и от того, какой файл выбран в
// combobox'е, а имя переживает и то, и другое.
struct RowRef {
    QString name;
    Tool::ID toolId{};
    bool useForCalc{};
};

class[[= Serial::name("Drilling")]] File final : public GCode::File {
public:
    void serialize(Serial::Writer& sb) const override { Serial::writeInto(sb, *this); }
    using GCode::File::File;
    explicit File(GCode::Params&& newGcp)
        : GCode::File{std::move(newGcp)} {
        if(gcp.tools.front().diameter())
            regenerate();
    }
    // Чем заполнить таблицу при открытии УП на правку. Хранится в самой УП, а
    // не в Params: Variant умеет только числа и UsedItems, строки в него не лягут.
    const std::vector<RowRef>& rows() const { return rows_; }
    void setRows(std::vector<RowRef> rows) { rows_ = std::move(rows); }
    int worckType() const { return worckType_; }
    void setWorckType(int type) { worckType_ = type; }
    int32_t srcFileId() const { return srcFileId_; }
    void setSrcFileId(int32_t id) { srcFileId_ = id; }

    QIcon icon() const override { return QIcon::fromTheme(u"drill-path"_s); }
    uint32_t type() const override { return DRILLING; }
    void createGi() override { createGiDrill(), itemGroup()->setVisible(true); }


private:
    std::vector<RowRef> rows_;
    int worckType_{};
    int32_t srcFileId_{-1};
};

} // namespace Drilling
