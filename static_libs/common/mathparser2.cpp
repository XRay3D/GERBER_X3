///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  XXXXX XX, 2025                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2025                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
#include "mathparser2.h"

#include <charconv>
using namespace std::literals;

constexpr auto enumToString(ParseError err) {
    switch(err) {
    case ParseError::ExpectedRoundBracket: return "ExpectedRoundBracket"sv;
    case ParseError::InvalidInput: return "InvalidInput"sv;
    case ParseError::UnknownBinaryOperator: return "UnknownBinaryOperator"sv;
    case ParseError::UnknownExpressionType: return "UnknownExpressionType"sv;
    case ParseError::UnknownUnaryOperator: return "UnknownUnaryOperator"sv;
    }
}

#define USE_TREE 0
#if USE_TREE
#include <QTreeWidget>
extern QTreeWidget* tv;
#else
class QTreeWidgetItem;
#endif

// MathParser2::Expression::Expression(std::string_view token)
//     : token{token} {}
// MathParser2::Expression::Expression(std::string_view token, Expression a)
//     : token{token}
//     , args {a} {}
// MathParser2::Expression::Expression(std::string_view token, Expression a, Expression b)
//     : token{token}
//     , args {a, b} {}
MathParser2::MathParser2(VarMap&& variables, std::string_view input)
    : input{input}
    , variables{std::move(variables)} { }
MathParser2::MathParser2(VarMap&& variables)
    : variables{std::move(variables)} { }
MathParser2::MathParser2(std::string_view input)
    : input{input} { }
std::string_view MathParser2::parseToken() {
    if(input.empty())
        return {};
    while(std::isspace(input.front()))
        input = input.substr(1);
    if(std::isdigit(input.front())) {
        auto begin = input.begin();
        size_t i{};
        while(i < input.size() && (std::isdigit(input[i]) || input[i] == '.'))
            ++i; // input = input.substr(1);
        std::string_view ret{begin, begin + i};
        input = input.substr(i);
        return ret;
    }
    struct Token {
        constexpr Token(const char* sv)
            : sv{sv} { }
        std::string_view sv;
        int i{};
    };
    static constexpr Token tokens[]{"(", ")"};
    std::string_view sv;
    auto find = [&sv, this](auto& map) -> int { // жадный поиск
        for(auto&& [key, val]: map)
            if(input.starts_with(key))
                sv = key;
        return sv.size();
    };
    if(find(tokens))
        return input = input.substr(sv.size()), sv;
    if(find(binaryFunc))
        return input = input.substr(sv.size()), sv;
    if(find(unaryFunc))
        return input = input.substr(sv.size()), sv;
    if(find(variables))
        return input = input.substr(sv.size()), sv;
    return {};
}
MathParser2::Expression MathParser2::parseSimpleExpression() {
    auto token = parseToken();
    if(token.size() == 0)
        throw std::runtime_error(std::string{enumToString(ParseError::InvalidInput)});
    if(token == "(") {
        auto result = parseExpression();
        if(parseToken() != ")")
            throw std::runtime_error(std::string{enumToString(ParseError::ExpectedRoundBracket)});
        return result;
    }
    if(std::isdigit(token[0]))
        return Expression{token};
    if(auto it = variables.find(token); it != variables.end())
        return Expression{token};
    return Expression{token, {parseSimpleExpression()}};
}
MathParser2::Expression MathParser2::parseBinaryExpression(int minPriority) {
    auto leftExpr = parseSimpleExpression();
    for(;;) {
        auto op = parseToken();
        auto priority = getPriority(op);
        if(priority <= minPriority) {
            //            input -= op.size();
            // std::cerr << input << std::endl;
            auto begin = input.data();
            auto end = input.data() + input.size();
            input = {begin - op.size(), end};
            return leftExpr;
        }
        auto rightExpr = parseBinaryExpression(priority);
        leftExpr = Expression{
            op, {leftExpr, rightExpr}
        };
    }
}
int MathParser2::getPriority(std::string_view binaryOp) {
    if(binaryOp.size())
        if(auto it = binaryFunc.find(binaryOp); it != binaryFunc.end())
            return it->second.priority;
    return 0;
}
MathParser2::Expression MathParser2::parseExpression() {
    return parseBinaryExpression(0);
}
Double MathParser2::parse(std::string_view input_) {
    try {
#if USE_TREE
        input = input_;
        auto item = new QTreeWidgetItem(tv, {QString::fromLocal8Bit(input.data(), input.size())});
        auto val = eval(parseExpression(), item);
        item->setData(1, Qt::EditRole, val);
        tv->addTopLevelItems({item});
        tv->expandAll();
        for(int i{}; i < tv->topLevelItemCount(); ++i)
            tv->collapseItem(tv->topLevelItem(i));
        input = "";
        return val;
#else
        input = input_;
        auto val = eval(parseExpression(), nullptr);
        input = "";
        return val;
#endif
    } catch(std::exception& ex) {
        qCritical() << ex.what();
        // auto e = stringToEnum<ParseError>(ex.what());
        auto ret = std::nan("");
        // *std::bit_cast<uint64_t*>(&ret) |= int(e);
        return ret;
        //        static std::array<char, 8> arr {};
        //        std::fill_n(std::begin(arr), std::size(arr) - 1, '\0');
        //        auto res = std::to_chars(arr.data(), arr.data() + arr.size(), (int)stringToEnum<ParseError>(ex.what()));
        //        std::cerr << arr.data() << std::endl;
        //        return std::nan(arr.data()); // хз почему рантайм не работает
        //        switch (e) {
        //        case ParseError ::Expected_round_bracket:
        //            return std::nan("0");
        //        case ParseError ::Invalid_input:
        //            return std::nan("1");
        //        case ParseError ::Unknown_binary_operator:
        //            return std::nan("2");
        //        case ParseError ::Unknown_expression_type:
        //            return std::nan("3");
        //        case ParseError ::Unknown_unary_operator:
        //            return std::nan("4");
        //        }
    }
}
Double MathParser2::parse() { return eval(parseExpression()); }
////////////////////////////////////////////////////////////////////////////
Double MathParser2::eval(const Expression& e, QTreeWidgetItem* twi) {
#if USE_TREE
    auto addChild = [](QTreeWidgetItem* twi, const auto& e, Double val) -> Double {
        if(twi)
            twi->setData(1, Qt::EditRole, val);
        return val;
    };
    if(twi) {
        QTreeWidgetItem* tmp = twi;
        tmp->addChild(twi = new QTreeWidgetItem(tmp, {QString::fromLocal8Bit(e.token.data(), e.token.size())}));
    }
    if(auto it = variables.find(e.token); it != variables.end())
        return addChild(twi, e, it->second);
#else
    auto addChild = [](QTreeWidgetItem* /*twi*/, const auto& /*e*/, Double val) -> Double {
        return val;
    };
#endif

    Double val{std::nan("")};
    switch(e.args.size()) {
    case 2:
        if(auto it = binaryFunc.find(e.token); it != binaryFunc.end()) {
            if(0 && e.token == "^"sv && e.args.size() == 2 && e.args.back().token == "^"sv) {
                auto right = eval(Expression{"^", {}}, twi);
                return addChild(twi, e, it->second.func(eval(e.args.front(), twi), right));
            }
            return addChild(twi, e, it->second.func(eval(e.args.front(), twi), eval(e.args.back(), twi)));
        }
        throw std::runtime_error(std::string{enumToString(ParseError::UnknownBinaryOperator)});
    case 1:
        if(auto it = unaryFunc.find(e.token); it != unaryFunc.end())
            return addChild(twi, e, it->second(eval(e.args.front(), twi)));
        throw std::runtime_error(std::string{enumToString(ParseError::UnknownUnaryOperator)});
    case 0:
        /*[[maybe_unused]] auto [ptr, ec] =*/std::from_chars(e.token.data(), e.token.data() + e.token.size(), val);
        // return addChild(twi, e, val);
        char* end = (char*)e.token.data() + e.token.size();
        return addChild(twi, e, strtod(e.token.data(), &end)); // str to double
    }
    throw std::runtime_error(std::string{enumToString(ParseError::UnknownExpressionType)});
}