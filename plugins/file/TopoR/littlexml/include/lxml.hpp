#pragma once

#include <algorithm>
#include <cassert>
#include <expected>
#include <format>
#include <memory>
#include <meta>
#include <print>
#include <ranges>
#include <string>
#include <vector>

// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/xml_parser.hpp>
// #include <boost/pfr.hpp>

#ifdef LXML_INTERFACE_UNIT
#define LXML_BEGIN_MODULE_EXPORT export {
#define LXML_END_MODULE_EXPORT   }
#define LXML_EXPORT              export
#else
#define LXML_BEGIN_MODULE_EXPORT
#define LXML_END_MODULE_EXPORT
#define LXML_EXPORT
#endif

LXML_BEGIN_MODULE_EXPORT
// using namespace std::string_literals;
using namespace std ::string_view_literals;
namespace r = std ::ranges;
namespace v = std ::views;
LXML_END_MODULE_EXPORT

// LXML_BEGIN_MODULE_EXPORT
LXML_EXPORT
namespace XML {

using std ::print;
using std ::println;
using std ::string_view;

#if 0

struct Document {
    std::string buf;
    boost::property_tree::ptree root;
    string_view version;
    string_view encoding;
    bool load(string_view path) {
        std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(fp); })>
            file(fopen(std::string{path}.c_str(), "r"), {});

        if(!file) {
            println(stderr, "Could not load file from '{}'", path);
            return false;
        }

        fseek(file.get(), 0, SEEK_END);
        int size = ftell(file.get());
        fseek(file.get(), 0, SEEK_SET);

        buf.resize(size);
        fread(buf.data(), 1, size, file.get());

        return parse(buf);
    }
    constexpr bool parse(string_view xmlCode) { // XML-код для парсинга
        // Создаем поток
        std::stringstream stream{xmlCode};
        try {
            // Читаем XML
            boost::property_tree::read_xml(stream, root);
        } catch(boost::property_tree::xml_parser_error err) {
            println(stderr, "XML parser error!\n{}:{}: {}",
                err.filename(),
                err.line(),
                err.message());
            return false;
        }
        return true;
    }
    bool write(string_view path, int indent) {
        // Создаем поток
        std::stringstream stream;
        try {
            // Записываем в другой поток
            boost::property_tree::write_xml(stream, root);
        } catch(boost::property_tree::xml_parser_error err) {
            println(stderr, "XML parser error!\n{}:{}: {}",
                err.filename(),
                err.line(),
                err.message());
            throw;
        }

        std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(fp); })>
            file(fopen(std::string{path}.c_str(), "w"), {});

        if(!file) {
            println(stderr, "Could not open file '{}'", path);
            return false;
        }
        println(file.get(), "{}", stream.str());
        return true;
    }
};

#else

// =============== Definitions ===============
struct Data {
    string_view key;
    std::variant<string_view, std::string> value_;
    constexpr string_view value() const {
        return value_.visit([](auto&& arg) -> string_view { return arg; });
    }
};

using Attribute = Data;

using AttributeList = std::vector<Data>;
using NodeList = std::vector<struct NodeTag*>;

struct NodeTag : Data, std::vector<std::unique_ptr<NodeTag>> {
    struct NodeTag* parent{};
    AttributeList attributes{};

    constexpr explicit NodeTag(NodeTag* parent = nullptr);

    explicit NodeTag(NodeTag* parent, string_view key, string_view val = {})
        : Data{key, val}, parent{parent} {
        if(parent) parent->emplace_back(this);
    }

    explicit NodeTag(NodeTag* parent, string_view key, std::string val)
        : Data{key, val}, parent{parent} {
        if(parent) parent->emplace_back(this);
    }

    constexpr ~NodeTag() = default;
    NodeList children(string_view tag);
    constexpr string_view attrVal(string_view key);
    Attribute* attr(string_view key);

    constexpr NodeTag* firstChild(string_view tag) {
        auto it = r::find(*this, tag, &Data::key);
        return it != end() ? it->get() : nullptr;
    }

    constexpr string_view tag() const noexcept { return key; }
    constexpr string_view text() const noexcept { return value(); }

    constexpr void setTag(string_view newTag) noexcept;
    constexpr void setText(string_view newText) noexcept { value_ = newText; }
};

// Ошибка разбора: где (offset/line/column от начала документа) и что именно.
// Вьюхи expected/found смотрят в разбираемый буфер -- живы, пока жив он.
struct ParseError {
    enum class Code {
        UnexpectedEof,         // документ оборвался посреди разметки
        UnterminatedTag,       // '<' без соответствующего '>'
        UnterminatedAttrValue, // значение атрибута без закрывающей кавычки
        AttrValueWithoutKey,   // кавычка без имени атрибута перед ней
        UnterminatedComment,   // '<!--' без '-->'
        UnsupportedMarkup,     // '<!DOCTYPE', '<![CDATA[' и прочее
        EmptyTagName,          // '<>' -- тег без имени
        TagMismatch,           // закрывающий тег не совпал с открытым элементом
        ExtraClosingTag,       // закрывающий тег, когда всё уже закрыто
        UnclosedElement,       // конец документа внутри незакрытого элемента
    };
    Code code{};
    size_t offset{};             // байтов от начала документа
    size_t line{1}, column{1};   // 1-based
    string_view expected{};      // TagMismatch/UnclosedElement: имя открытого элемента
    string_view found{};         // имя встреченного тега либо фрагмент документа у ошибки

    std::string message() const {
        using enum Code;
        std::string what;
        switch(code) {
        case UnexpectedEof: what = "unexpected end of document"; break;
        case UnterminatedTag: what = "tag is never closed ('>' not found)"; break;
        case UnterminatedAttrValue: what = "attribute value is missing its closing quote"; break;
        case AttrValueWithoutKey: what = "attribute value has no name"; break;
        case UnterminatedComment: what = "comment is never closed ('-->' not found)"; break;
        case UnsupportedMarkup: what = "unsupported '<!' markup (only comments are supported)"; break;
        case EmptyTagName: what = "tag has no name"; break;
        case TagMismatch: what = std::format("closing tag </{}> does not match open element <{}>", found, expected); break;
        case ExtraClosingTag: what = std::format("closing tag </{}>, but no element is open", found); break;
        case UnclosedElement: what = std::format("end of document, but element <{}> is still open", expected); break;
        }
        const bool tagCtx = code == TagMismatch || code == ExtraClosingTag || code == UnclosedElement;
        if(!tagCtx && found.size()) what += std::format(", near '{}'", found);
        return std::format("{}:{}: {}", line, column, what);
    }
};

struct Document {
    std::string buf;
    NodeTag root;
    string_view version;
    string_view encoding;
    bool load(string_view path);
    constexpr std::expected<void, ParseError> parse(string_view buf);
    bool write(string_view path, int indent);
};

// =============== Implementation ===============

// =============== Node ===============
inline constexpr void NodeTag::setTag(string_view newTag) noexcept {
    if(key.size()) return;
    if(newTag.starts_with('<'))
        newTag = newTag.substr(1);
    if(size_t i = newTag.find_first_of("\r\n\t /"sv); i < newTag.size())
        newTag = newTag.substr(0, i);
    key = newTag;
}

inline constexpr NodeTag::NodeTag(NodeTag* parent)
    : parent{parent} {
    if(parent) parent->emplace_back(this);
}

inline NodeList NodeTag::children(string_view tag) {
    NodeList list;
    auto filter = [tag](auto&& child) { return child->tag() == tag; };
    list.assign_range(*this | v::filter(filter) | v::transform(&std::unique_ptr<NodeTag>::get));
    return list;
}

inline constexpr string_view NodeTag::attrVal(string_view key) {
    for(int i = 0; i < attributes.size(); i++) {
        Attribute attr = attributes /*.data*/[i];
        if(attr.key == key)
            return attr.value();
    }
    return {};
}

inline Attribute* NodeTag::attr(string_view key) {
    auto it = r::find(attributes, key, &Attribute::key);
    return (it != attributes.end()) ? it.base() : nullptr;
}
// =============== Document ===============
inline bool Document::load(string_view path) {
    std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(fp); })>
        file(fopen(std::string{path}.c_str(), "r"), {});

    if(!file) {
        println(stderr, "Could not load file from '{}'", path);
        return false;
    }

    fseek(file.get(), 0, SEEK_END);
    int size = ftell(file.get());
    fseek(file.get(), 0, SEEK_SET);

    root.clear();
    buf.resize(size);
    fread(buf.data(), 1, size, file.get());

    if(auto parsed = parse(buf); !parsed) {
        println(stderr, "{}:{}", path, parsed.error().message());
        return false;
    }
    return true;
}

// Никакого вывода внутри -- все ошибки через std::expected, поэтому parse
// пригоден и для constexpr (тесты в littlexml/tests проверяют это static_assert'ом).
inline constexpr std::expected<void, ParseError> Document::parse(string_view buf) {
    const string_view doc = buf; // весь документ -- для позиций ошибок
    using Code = ParseError::Code;

    auto fail = [doc](Code code, string_view at,
                    string_view expected = {}, string_view found = {}) {
        const size_t offset = size_t(at.data() - doc.data());
        const size_t prevNl = offset ? doc.rfind('\n', offset - 1) : doc.npos;
        if(found.empty()) { // нет явного контекста -- фрагмент документа у ошибки
            found = at.substr(0, std::min<size_t>(at.size(), 40u));
            if(size_t nl = found.find_first_of("\r\n"sv); nl < found.size())
                found = found.substr(0, nl);
        }
        return std::unexpected{
            ParseError{
                       .code = code,
                       .offset = offset,
                       .line = 1 + size_t(r::count(doc.substr(0, offset), '\n')),
                       .column = offset - (prevNl == doc.npos ? 0 : prevNl + 1) + 1,
                       .expected = expected,
                       .found = found}
        };
    };

    NodeTag* currNode = &root;
    // Remove bom
    if(buf.starts_with("\xEF\xBB\xBF"sv))
        buf = buf.substr(3);

    enum class TagType {
        START,
        INLINE
    };

    // Один тег от '<' до '>': имя + атрибуты. При ошибке оставляет buf на
    // проблемном месте и возвращает только код -- позицию и контекст
    // достраивает вызывающая сторона через fail.
    static constexpr auto parseAttrs = +[](string_view& buf, NodeTag& node) -> std::expected<TagType, Code> {
        Attribute attr;
        char quote{}; // кавычка, открывшая значение: другая внутри него -- часть значения
        for(;;) {
            const size_t i = quote
                ? buf.find_first_of(quote)
                : buf.find_first_of(attr.key.empty() ? " \t\r\n'\"=>"sv : "'\""sv);
            if(i >= buf.size())
                return std::unexpected{quote ? Code::UnterminatedAttrValue : Code::UnterminatedTag};
            switch(buf[i]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                node.setTag(buf.substr(1, i));
                buf = buf.substr(i + 1);
                continue;
            case '\'':
            case '"':
                if(quote) { // закрывающая
                    attr.value_ = buf.substr(0, i);
                    node.attributes.emplace_back(attr);
                    attr = {};
                    quote = {};
                } else if(attr.key.empty()) {
                    buf = buf.substr(i);
                    return std::unexpected{Code::AttrValueWithoutKey};
                } else
                    quote = buf[i];
                buf = buf.substr(i + 1);
                continue;
            case '=':
                attr.key = buf.substr(0, i);
                buf = buf.substr(i + 1);
                continue;
            case '>':
                if(i && buf[i - 1] == '/') {
                    node.setTag(buf.substr(0, i));
                    buf = buf.substr(i);
                    return TagType::INLINE;
                }
                node.setTag(buf.substr(1, i - 1));
                buf = buf.substr(i);
                return TagType::START;
            }
        }
    };

    size_t i;
    while((i = buf.find_first_of('<')) < buf.size()) {
        string_view lex;
        if(buf.front() == '>') lex = buf.substr(1, i - 1);
        if(size_t ws = lex.find_first_not_of("\r\n\t "sv); ws > lex.size()) lex = {};
        buf = buf.substr(i);
        // Inner text
        if(lex.size()) currNode->setText(lex);

        if(buf.size() < 2)
            return fail(Code::UnexpectedEof, buf);

        switch(buf[1]) {
        case '/': { // End of node
            i = buf.find_first_of('>');
            if(i == buf.npos)
                return fail(Code::UnterminatedTag, buf);
            string_view tag = buf.substr(2, i - 2);
            if(size_t ws = tag.find_first_of(" \t\r\n"sv); ws < tag.size())
                tag = tag.substr(0, ws);
            if(currNode == &root)
                return fail(Code::ExtraClosingTag, buf, {}, tag);
            if(currNode->tag() != tag)
                return fail(Code::TagMismatch, buf, currNode->tag(), tag);
            currNode = currNode->parent;
            buf = buf.substr(i);
            continue;
        }
        case '!': // Comments
            if(buf.starts_with("<!--"sv)) {
                const size_t end = buf.find("-->"sv);
                if(end == buf.npos)
                    return fail(Code::UnterminatedComment, buf);
                (new NodeTag{currNode})->setText(buf.substr(0, end + 3));
                buf = buf.substr(end + 2); // остаться на '>' -- как после обычного тега
                continue;
            }
            return fail(Code::UnsupportedMarkup, buf);
        case '?': { // Declaration tags
            NodeTag desc;
            if(auto tt = parseAttrs(buf, desc); !tt)
                return fail(tt.error(), buf);
            version = desc.attrVal("version"sv);
            encoding = desc.attrVal("encoding"sv);
            continue;
        }
        default: { // New node
            currNode = new NodeTag{currNode};
            const auto tt = parseAttrs(buf, *currNode);
            if(!tt) return fail(tt.error(), buf);
            if(currNode->tag().empty())
                return fail(Code::EmptyTagName, buf);
            if(*tt == TagType::INLINE)
                currNode = currNode->parent;
            continue;
        }
        }
    }
    if(currNode != &root)
        return fail(Code::UnclosedElement, doc.substr(doc.size()), currNode->tag());
    return {};
}

inline bool Document::write(string_view path, int indent) {
    std::unique_ptr<FILE, decltype([](FILE* fp) { if(fp) fclose(fp); })>
        file(fopen(std::string{path}.c_str(), "w"), {});

    if(!file) {
        println(stderr, "Could not open file '{}'", path);
        return false;
    }

    if(version.empty()) version = "1.0";
    if(encoding.empty()) encoding = "UTF-8";

    println(file.get(), R"(<?xml version="{}" encoding="{}"?>)", version, encoding);
    auto nodeOut = [file = file.get(), indent](this auto&& nodeOut, const NodeTag* node, int times = 0) -> void {
        const auto indentTag = v::repeat(' ', indent * times);
        const auto indentAttr = v::repeat(' ', indent * ++times - 1);
        for(auto&& child: *node) {
            // if(times > 0) print(file, "{:{}}", " "sv, indent * times);
            print(file, "{:s}", indentTag);
            if(child->text().starts_with("<!--"sv)) {
                println(file, "{}", child->text());
                continue;
            }

            print(file, "<{}", child->tag());
            r::sort(child->attributes, {}, &Data::key); // NOTE remove noise in diff
            for(Attribute attr: child->attributes) {
                // if(attr.value().empty()) continue;
                if(child->attributes.size() > 8)
                    print(file, "\n{:s}", indentAttr);
                print(file, R"( {}="{}")", attr.key, attr.value());
            }
            if(child->size() == 0 && child->text().empty())
                println(file, "/>");
            else {
                print(file, ">");
                if(child->size() == 0)
                    println(file, "{}</{}>", child->text(), child->tag());
                else {
                    println(file, "");
                    nodeOut(child.get(), times);
                    // if(times > 0) print(file, "{:{}}", " "sv, indent * times);
                    print(file, "{:s}", indentTag);
                    println(file, "</{}>", child->tag());
                }
            }
        }
    };

    nodeOut(&root);
    // fclose(file);
    return true;
}

#endif

} // namespace XML
// LXML_END_MODULE_EXPORT

#include "lxmlser.hpp"
