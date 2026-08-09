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

    friend QDataStream& operator<<(QDataStream& stream, const RowRef& row) {
        return stream << row.name << static_cast<int32_t>(row.toolId) << row.useForCalc;
    }
    friend QDataStream& operator>>(QDataStream& stream, RowRef& row) {
        int32_t toolId{};
        stream >> row.name >> toolId >> row.useForCalc;
        row.toolId = static_cast<Tool::ID>(toolId);
        return stream;
    }
};

class File final : public GCode::File {
public:
    using GCode::File::File;
    explicit File(GCode::Params&& newGcp)
        : GCode::File{std::move(newGcp)} {
        if(gcp.tools.front().diameter()) {
            initSave();
            addInfo();
            statFile();
            genGcodeAndTile();
            endFile();
        }
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

    void write(QDataStream& stream) const override { GCode::File::write(stream), stream << rows_ << worckType_ << srcFileId_; }
    void read(QDataStream& stream) override { GCode::File::read(stream), stream >> rows_ >> worckType_ >> srcFileId_; }
    void genGcodeAndTile() override {
        const QRectF rect = App::project().worckRect();
        for(size_t x{}; x < App::project().stepsX(); ++x) {
            for(size_t y{}; y < App::project().stepsY(); ++y) {
                const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);
                saveDrill(offset);
                if(gcp.params.contains(GCode::Params::NotTile))
                    return;
            }
        }
    }

private:
    std::vector<RowRef> rows_;
    int worckType_{};
    int32_t srcFileId_{-1};
};

} // namespace Drilling
