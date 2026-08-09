/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "abstract_file.h"
#include "g2_parser.h"
#include "md5.h"

namespace Gerber2 {

// Тип файла плагина. Отличается от Gerber, чтобы оба плагина уживались.
constexpr auto GERBER2 = "Gerber2"_hash32;

class File : public AbstractFile {
    friend class Plugin;

public:
    File();
    ~File() override = default;

    enum ItemsType {
        NullType = -1,
        Normal,  // залитая медь
        ApPaths, // осевые линии трасс
    };

    // Исходный текст файла — то, что редактируется и сохраняется.
    const QString& source() const { return source_; }

    // Перечитать изображение из текста. Возвращает false при ошибке разбора,
    // в этом случае прежнее изображение остаётся нетронутым.
    bool setSource(const QString& text, QString* errorOut = nullptr);

    // Записать текущий исходный текст на диск.
    bool saveAs(const QString& fileName, QString* errorOut = nullptr) const;

    const ParseResult& parsed() const { return parsed_; }

    // AbstractFile interface
    uint32_t type() const override { return GERBER2; }
    void createGi() override;
    void setItemType(int type) override;
    int itemsType() const override { return itemsType_; }
    void setColor(const QColor& color) override;
    FileTree::Node* node() override;
    QIcon icon() const override;
    std::vector<GraphicObject> getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test = {}) const override;

protected:
    Geo::Polygons merge() const override;
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

private:
    void rebuild(); // пересобрать Gi-элементы по текущему parsed_
    void clearGi();

    QString source_;
    ParseResult parsed_;
};

} // namespace Gerber2
