// Тесты Document::parse: корректные документы, все коды ошибок с позициями и
// работоспособность в constexpr (static_assert). Без внешних фреймворков.
#include <lxml.hpp>

#include <print>
#include <string_view>

using namespace std::string_view_literals;
using XML::Document;
using XML::ParseError;
using Code = XML::ParseError::Code;

static int failures = 0;

#define CHECK(cond) \
    do { \
        if(!(cond)) { \
            ++failures; \
            std::println(stderr, "FAIL {}:{}: {}", __FILE__, __LINE__, #cond); \
        } \
    } while(0)

static ParseError errorOf(std::string_view xml) {
    Document doc;
    auto parsed = doc.parse(xml);
    CHECK(!parsed.has_value());
    return parsed ? ParseError{} : parsed.error();
}

// ======================= constexpr =======================

static_assert([] { // корректный документ разбирается в константном вычислении
    Document doc;
    return doc.parse("<?xml version='1.0'?><a b='1'><c/>text</a>"sv).has_value();
}());

static_assert([] { // ошибки с позициями доступны в константном вычислении
    Document doc;
    auto parsed = doc.parse("<a>\n<b></c>"sv);
    return !parsed
        && parsed.error().code == Code::TagMismatch
        && parsed.error().line == 2
        && parsed.error().column == 4
        && parsed.error().expected == "b"sv
        && parsed.error().found == "c"sv;
}());

int main() {
    { // корректный документ: декларация, комментарий, вложенность, оба вида кавычек
        Document doc;
        auto parsed = doc.parse(
            "<?xml version='1.0' encoding=\"UTF-8\"?>\n"
            "<!-- header -->\n"
            "<Root>\n"
            "\t<Item id=\"1\" name='first'>text</Item>\n"
            "\t<Empty/>\n"
            "\t<Nested><Leaf x=\"2\"/></Nested>\n"
            "</Root>\n"sv);
        CHECK(parsed.has_value());
        CHECK(doc.version == "1.0"sv);
        CHECK(doc.encoding == "UTF-8"sv);
        CHECK(doc.root.size() == 2); // комментарий + Root

        auto* rootEl = doc.root.firstChild("Root"sv);
        CHECK(rootEl && rootEl->size() == 3);
        auto* item = rootEl ? rootEl->firstChild("Item"sv) : nullptr;
        CHECK(item && item->text() == "text"sv);
        CHECK(item && item->attrVal("id"sv) == "1"sv);
        CHECK(item && item->attrVal("name"sv) == "first"sv);
        auto* empty = rootEl ? rootEl->firstChild("Empty"sv) : nullptr;
        CHECK(empty && empty->empty() && empty->attributes.empty());
        auto* nested = rootEl ? rootEl->firstChild("Nested"sv) : nullptr;
        auto* leaf = nested ? nested->firstChild("Leaf"sv) : nullptr;
        CHECK(leaf && leaf->attrVal("x"sv) == "2"sv);
    }

    { // кавычка другого типа внутри значения -- часть значения
        Document doc;
        CHECK(doc.parse("<a q=\"it's ok\" w='say \"hi\"'/>"sv).has_value());
        auto* a = doc.root.firstChild("a"sv);
        CHECK(a && a->attrVal("q"sv) == "it's ok"sv);
        CHECK(a && a->attrVal("w"sv) == "say \"hi\""sv);
    }

    { // атрибуты, разделённые табами и переводами строк
        Document doc;
        CHECK(doc.parse("<a\n\tb=\"1\"\r\n\tc=\"2\"/>"sv).has_value());
        auto* a = doc.root.firstChild("a"sv);
        CHECK(a && a->attrVal("b"sv) == "1"sv && a->attrVal("c"sv) == "2"sv);
    }

    { // '>' внутри комментария не завершает его; текст комментария не «прилипает» к родителю
        Document doc;
        CHECK(doc.parse("<a><!-- x > y --><b/></a>"sv).has_value());
        auto* a = doc.root.firstChild("a"sv);
        CHECK(a && a->size() == 2);
        CHECK(a && a->front()->text() == "<!-- x > y -->"sv);
        CHECK(a && a->text().empty());
        CHECK(a && a->firstChild("b"sv));
    }

    { // BOM в начале
        Document doc;
        CHECK(doc.parse("\xEF\xBB\xBF<a/>"sv).has_value());
        CHECK(doc.root.firstChild("a"sv));
    }

    // ======================= ошибки =======================

    {
        auto err = errorOf("<a>\n  <b>\n</c>"sv);
        CHECK(err.code == Code::TagMismatch);
        CHECK(err.expected == "b"sv && err.found == "c"sv);
        CHECK(err.line == 3 && err.column == 1);
        CHECK(err.offset == 10);
    }
    {
        auto err = errorOf("<a><b></b>"sv);
        CHECK(err.code == Code::UnclosedElement);
        CHECK(err.expected == "a"sv);
    }
    CHECK(errorOf("<a><b attr"sv).code == Code::UnterminatedTag);
    CHECK(errorOf("<a b=\"x"sv).code == Code::UnterminatedAttrValue);
    CHECK(errorOf("<a \"v\"/>"sv).code == Code::AttrValueWithoutKey);
    CHECK(errorOf("<a><!-- foo"sv).code == Code::UnterminatedComment);
    CHECK(errorOf("<!DOCTYPE html><a/>"sv).code == Code::UnsupportedMarkup);
    {
        auto err = errorOf("<a/></a>"sv);
        CHECK(err.code == Code::ExtraClosingTag);
        CHECK(err.found == "a"sv);
    }
    CHECK(errorOf("<>"sv).code == Code::EmptyTagName);
    CHECK(errorOf("<a>text<"sv).code == Code::UnexpectedEof);

    { // message() человекочитаем и содержит позицию
        auto err = errorOf("<a>\n  <b>\n</c>"sv);
        auto msg = err.message();
        CHECK(msg.contains("3:1"sv));
        CHECK(msg.contains("</c>"sv));
        CHECK(msg.contains("<b>"sv));
    }

    if(failures)
        std::println(stderr, "{} check(s) FAILED", failures);
    else
        std::println("all checks passed");
    return failures;
}
