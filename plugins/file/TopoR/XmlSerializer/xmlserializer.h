#pragma once

#include "treeitem.h"
#include "xmlserializertypes.h"

#include <QBuffer>
#include <QDebug>
#include <QRegularExpression>
#include <QtXml/QDomDocument>

#include "pfr.hpp"
#include <ranges>
#include <set>
#include <source_location>

namespace Xml {

// };
// using namespace TopoR; // call to function 'stringToEnum' ADL

// namespace pfr = boost::pfr;
namespace ranges = std::ranges;
namespace views = std::ranges::views;
using sl = std::source_location;

template <typename R> concept Range = requires(R& r) { std::begin(r); std::end(r); };
template <typename T> concept Struct = !Range<T> && pfr::is_implicitly_reflectable_v<T, T>; //&& pfr::tuple_size_v<T>;
template <typename T> concept Numbers = std::is_arithmetic_v<T>;
template <typename T> concept Enums = std::is_enum_v<T>;
template <typename T> concept To = std::is_base_of_v<std::initializer_list<typename T::value_type>, T> == false || Struct<T>;

struct Serializer {

    Serializer(const QString& name);
    void save(const QString& dir);

    template <Struct /*To*/ T> operator T() && {
        return to<T>();
    }

    template <Struct T> decltype(auto) operator=(T&& val) && {
        if(write(val)) save();
        return *this;
    }

    template <Struct /*To*/ T> T to() {
        T val{};
        if(load()) read(val);
        return val;
    }

    template <Struct T> decltype(auto) from(const T& val) {
        if(write(val)) save();
        return *this;
    }

    /// \brief operator >>
    template <Struct T>
    decltype(auto) operator>>(T& val) {
        if(load()) read(val);
        return *this;
    }

    /// \brief operator <<
    template <Struct T>
    decltype(auto) operator<<(const T& val) {
        if(write(val)) save();
        return *this;
    }

    TreeItem* getItem() const { return item; }
    QString toString() const;

private:
    QString fileName;
    QDomDocument doc{"mydocument"};
    TreeItem* const item{new TreeItem};

    bool load();
    bool save();

    class once_flag {
        bool flag{};

    public:
        void set() { flag = true; }
        void reset() { flag = false; }
        operator bool() { return flag ? (flag = false, true) : false; }
    };

    TreeItem* dbgTree{item};
    QDomNode node;
    int depth{};
    mutable int fieldIndex{};
    bool isRead{};
    bool isVariant{};
    once_flag isArray{};
    mutable once_flag isAttribute{};
    mutable QString fieldName;
    mutable QDomDocument outDoc;
    mutable QDomNode outNode;

    void debugNode() const;

    template <typename T> struct Tag { };
    static constexpr auto ArrayNull = std::numeric_limits<int>::max();

    bool read(void*) { return true; }
    bool write(const void*) const { return true; }
    /**************************************************************************/
    /// QString
    bool read(QString& str) {
        if(node.isElement()) {
            node = node.firstChildElement(fieldName);
            str = node.toElement().text();
            node = node.parentNode();
            if(dbgTree) dbgTree->addItem(new TreeItem{fieldName, "QString", str, "", node.lineNumber()});
        } else if(node.isAttr()) {
            str = node.nodeValue();
            if(dbgTree) dbgTree->addItem(new TreeItem{node.toAttr().name(), "QString", str, "Attr", node.parentNode().lineNumber()});
        }
        return str.size();
    }

    bool write(const QString& str) const {
        if(isAttribute) {
            outNode.toElement().setAttribute(fieldName, str);
        } else {
            auto element = outDoc.createElement(fieldName);
            if(str.size()) element.appendChild(outDoc.createTextNode(str));
            outNode.appendChild(element);
        }
        return true;
    }
    /// Numbers
    template <Numbers T> bool read(T& value) { // float`ы int`ы
        QString text;
        if(node.isElement()) {
            auto element = node.toElement();
            if(text = element.attribute(TypeName<T>); text.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{TypeName<T>, TypeName<T>, text, "", node.lineNumber()});
            } else if(text = element.attribute(fieldName); text.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{fieldName, TypeName<T>, text, "", node.lineNumber()});
            }
        } else if(node.isAttr()) {
            if(text = node.nodeValue(); text.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{node.toAttr().name(), TypeName<T>, text, "Attr", node.parentNode().lineNumber()});
            }
        }
        value = QVariant{text}.value<T>();
        return text.size();
    }

    template <Numbers T> bool write(const T& value) const { // float`ы int`ы
        const auto numStr = QVariant{value}.toString();
        if(isAttribute) {
            outNode.toElement().setAttribute(fieldName, numStr);
        } else {
            auto element = outDoc.createElement(fieldName);
            element.appendChild(outDoc.createTextNode(numStr));
            outNode.appendChild(element);
        }
        return true;
    }

    /// Enums
    template <Enums T> bool read(T& e) { // enum`ы
        QString value;
        if(node.isElement()) {
            if(value = node.toElement().attribute(TypeName<T>); value.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{fieldName, TypeName<T>, value, "", node.lineNumber()});
            } else if(value = node.toElement().attribute(fieldName); value.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{fieldName, TypeName<T>, value, "", node.lineNumber()});
            }
        } else if(node.isAttr()) {
            if(value = node.nodeValue(); value.size()) {
                if(dbgTree) dbgTree->addItem(new TreeItem{node.toAttr().name(), TypeName<T>, value, "Attr", node.parentNode().lineNumber()});
            }
        }
        value.replace(' ', '_'); // NOTE workaround for enumerations with spaces
        e = stringToEnum<T>(value.toStdString());
        return value.size();
    }
    template <Enums T> bool write(const T& e) const { // enum`ы
        auto str = QString::fromStdString(std::string{enumToString(e)});
        str.replace('_', ' '); // NOTE workaround for enumerations with spaces
        if(!str.size()) return false;
        if(isAttribute) {
            outNode.toElement().setAttribute(fieldName, str);
        } else {
            auto element = outDoc.createElement(fieldName);
            element.appendChild(outDoc.createTextNode(str));
            outNode.appendChild(element);
        }
        return true;
    }

    /**************************************************************************/

    /// Optional
    template <typename T> bool read(Optional<T>& optional) { // перенаправление ↑↑↑
        if(T val; read(val)) optional = std::move(val);
        return optional.has_value();
    }

    template <typename T> bool write(const Optional<T>& optional) const { // перенаправление ↑↑↑
        if(optional) return write(*optional);
        return optional.has_value();
    }

    /// Attr<T>
    template <typename T, bool Opt> bool read(Attr<T, Opt>& attr) { // перенаправление ↑↑↑
        auto attributes = node.attributes();
        if(attributes.contains(TypeName<T>)) {
            node = attributes.namedItem(TypeName<T>);
            bool ok = read(attr.value);
            node = node.parentNode();
            return ok;
        } else if(attributes.contains(fieldName)) {
            node = attributes.namedItem(fieldName);
            bool ok = read(attr.value);
            node = node.parentNode();
            return ok;
        }
        return false;
    }

    template <typename T, bool Opt> bool write(const Attr<T, Opt>& attr) const { // перенаправление ↑↑↑
        if(!attr) return false;
        isAttribute.set();
        return write(attr.value);
    }

    /// Variant<Ts...>
    template <typename... Ts> bool read(Variant<Ts...>& variant) { // перенаправление ↑↑↑
        if(!node.isElement()) return false;
        // debugNode();
        int ctr{};
        auto reader = [&]<typename T>() {
            if(ctr) return;
            if(T val{}; node.toElement().tagName() == TypeName<T> && read(val)) {
                ++ctr, variant = std::move(val);
            } else {
                auto copy = node;
                node = node.firstChildElement(TypeName<T>);
                if(isVariant = true; !node.isNull() && read(val))
                    ++ctr, variant = std::move(val);
                isVariant = false;
                node = copy;
            }
        };
        (reader.template operator()<Ts>(), ...);
        assert(ctr < 2);
        return ctr > 0;
    }

    template <typename... Ts> bool write(const Variant<Ts...>& variant) const { // перенаправление ↑↑↑
        if(!variant.has_value()) return false;
        return variant.visit([this](auto&& val) { return write(val); });
    }

    /// ArrayElem<T>
    template <typename T, typename CanSkip> bool read(ArrayElem<T, CanSkip>& vector) { // перенаправление ↑↑↑
        QDomNode node_ = node.firstChildElement(fieldName);
        if(node_.isNull()) return false;
        auto childNodes = node_.childNodes();
        if(!childNodes.size())
            return true /*false*/; // NOTE Force add emty tag
        if(dbgTree) dbgTree = dbgTree->addItem(new TreeItem{fieldName, TypeName<T>, QString::number(childNodes.size()), "ArrObj", node.lineNumber()});
        vector.resize(childNodes.size());

        bool ok{true};

        for(int index{}; auto&& var: vector) {
            isArray.set(); // = true;
            node = childNodes.at(index++);
            ok &= read(var);
        }

        if(dbgTree) dbgTree = dbgTree->parent();

        return ok;
    }

    template <typename T, typename CanSkip> bool write(const ArrayElem<T, CanSkip>& vector) const { // перенаправление ↑↑↑
        if(vector.canSkip()) return false;                                                          // NOTE maybe true
        bool ok{true};
        outNode = outNode.appendChild(outDoc.createElement(fieldName));
        for(auto&& var: vector)
            ok &= write(var);
        outNode = outNode.parentNode();
        return ok;
    }

    /// Array<T>
    template <typename T, typename CanSkip> bool read(Array<T, CanSkip>& vector) { // перенаправление ↑↑↑
        if(node.isNull()) return false;

        auto childNodes = node.childNodes();

        auto nodeNameRange = views::iota(0, childNodes.size())
            | views::transform([&childNodes](int i) { return childNodes.at(i).nodeName(); });

        constexpr auto typeNames = Overload{
            []<typename... Ts>(Tag<Variant<Ts...>>) { return std::array{TypeName<Ts>...}; },
            []<typename Ty>(Tag<Ty>) { /*             */ return std::array{TypeName<T>}; },
        }(Tag<T>{});

        std::pair index{-1, -1};

        for(int i{}; auto&& name: nodeNameRange) {
            for(auto&& type: typeNames) {
                if(index.first == -1 && name == type) index.first = i;
                if(name == type) index.second = i;
            }
            ++i;
        }

        if(index.first == -1) return false;

        vector.resize((index.second - index.first) + 1);

        bool ok{true};
        if(dbgTree) dbgTree->itemData[TreeItem::IsAttr] = "Array";
        for(auto&& var: vector) {
            isArray.set(); // = true;
            node = childNodes.at(index.first++);
            ok &= read(var);
        }
        return ok;
    }

    template <typename T, typename CanSkip> bool write(const Array<T, CanSkip>& vector) const { // перенаправление ↑↑↑
        if(vector.canSkip()) return false;                                                      // NOTE maybe true
        int ok{};
        for(auto&& var: vector) ok += write(var);
        return ok == vector.size();
    }

    /// Skip<T> пропуск поля
    template <typename T> bool read(Skip<T>&) { return true; }
    template <typename T> bool write(const Skip<T>&) const { return true; }

    /// Named<T>
    template <typename T, Name NAME> bool read(NamedTag<T, NAME>& named) { // чтение именованного поля
        return read(*named, NAME);
    }

    template <typename T, Name NAME> bool write(const NamedTag<T, NAME>& named) const { // чтение именованного поля
        return write(*named, NAME);
    }

    // public:
    template <Struct T> bool read(T& str, const QString& name = {}) { // чтение полей структуры
        const QString tagName = name.size() ? name : TypeName<T>;

        if(!isArray && !isVariant)
            node = (node.isNull() ? doc : node).firstChildElement(tagName);
        isVariant = false;

        if(dbgTree) dbgTree = dbgTree->addItem(new TreeItem{tagName, TypeName<T>, "", "", node.lineNumber()});
        int ok{};
        if constexpr(pfr::tuple_size_v<T>)
            pfr::for_each_field_with_name(str, [this, &ok](auto name, auto&& field, auto index) {
                fieldName = QString::fromUtf8(name.data());
                if(fieldName.endsWith('_')) fieldName.resize(fieldName.size() - 1);
                fieldIndex = index;
                auto copy = node;
                ok += read(field);
                node = copy;
            });
        else ok = 1;

        node = node.parentNode();
        if(dbgTree) dbgTree = dbgTree->parent();
        return ok;
    }

    template <Struct T> bool write(const T& str, const QString& name = {}) const { // чтение полей структуры
        if constexpr(requires { str.isEmpty(); })
            if(str.isEmpty()) return false;

        const QString tagName = name.size() ? name : TypeName<T>;

        outNode = outNode.isNull()
            ? outDoc.appendChild(outDoc.createElement(tagName))
            : outNode.appendChild(outDoc.createElement(tagName));

        int ok{};
        pfr::for_each_field_with_name(str, [this, &ok](auto name, auto&& field, auto index) {
            fieldName = QString::fromUtf8(name.data());
            if(fieldName.endsWith('_')) fieldName.resize(fieldName.size() - 1);
            fieldIndex = index;
            auto copy = outNode;
            ok += write(field);
            outNode = copy;
        });

        outNode = outNode.parentNode();
        return ok;
    }
};

} // namespace Xml

inline Xml::Serializer operator""_xml(const char* name, size_t /*len*/) {
    return Xml::Serializer{name};
}
