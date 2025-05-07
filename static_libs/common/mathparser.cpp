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
#include "mathparser.h"

#if MT == 1
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */
// #include "utils.h"
#include <QDebug>
#include <QStringBuilder>
#include <boost/stacktrace.hpp>
#include <charconv>
#include <cmath>
#include <sstream>

template <> struct QConcatenable<sv> : private QAbstractConcatenable {
    using type = sv;
    using ConvertTo = QString;
    static constexpr bool ExactSize = true;
    static int size(sv a) { return a.size(); }
    static inline void QT_ASCII_CAST_WARN appendTo(sv a, QChar*& out) {
        for(char c: a) *out++ = c;
    }
};

MathParser::MathParser(VarMap* variables)
    : variables{variables} {
}

double MathParser::getVariable(const QString& varName) {

    if(!variables || !variables->contains(varName)) {
        qWarning() << "Error: Try get unexists variable '" + varName + "'";
        return 0.0;
    }
    return variables->at(varName);
}

double MathParser::parse(const QString& s) {

    Result result;
    try {
        result = plusMinus(sv{reinterpret_cast<const char16_t*>(s.data()), size_t(s.size())});
        if(result.rest.size()) {
            //            std::stringstream ss;
            //            ss << boost::stacktrace::stacktrace();
            //            qWarning() << QString::fromStdString(ss.str());
            qWarning() << "Error: can't full parse'" << s << "'rest: " << result.rest.data();
        }
    } catch(const sv& str) {
        qWarning() << str.data();
    }
    return result.acc;
}

Result MathParser::plusMinus(sv s) { // throws Exception

    Result current = mulDiv(s);
    double acc = current.acc;

    while(current.rest.length() > 0) {
        if(!(current.rest.at(0) == '+' || current.rest.at(0) == '-'))
            break;

        QChar sign = current.rest.at(0);
        sv next = current.rest.substr(1);

        current = mulDiv(next);
        if(sign == '+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result(acc, current.rest);
}

Result MathParser::bracket(sv s) // throws Exception
{

    QChar zeroChar = s.at(0);
    if(zeroChar == '(') {
        Result r = plusMinus(s.substr(1));
        if(!r.rest.empty() && r.rest.at(0) == ')')
            r.rest = r.rest.substr(1);
        else
            qWarning() << "Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

Result MathParser::functionVariable(sv s) // throws Exception
{

    int sign{+1};
    if(s.starts_with('-')) {
        sign = -1;
        s = s.substr(1);
    }

    // ищем название функции или переменной имя обязательно должна начинаться с буквы
    sv f;
    int i{};

    while(i < s.length() && ((QChar(s.at(i)).isLetter() || s.at(i) == '$') || (QChar(s.at(i)).isDigit() && i > 0))) {
        //    while (i < s.length() && (QChar(s.at(i)).isLetter() || (QChar(s.at(i)).isDigit() && i > 0))) {
        // f += s.at(i);
        i++;
    }
    f = s.substr(0, i);

    if(!f.empty()) { // если что-нибудь нашли и следующий символ скобка значит - это функция
        if(s.length() > i && s.at(i) == '(') {
            Result r = bracket(s.substr(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result(getVariable(toString(f)) * sign, s.substr(f.length()));
    }
    return num(s);
}

Result MathParser::mulDiv(sv s) // throws Exception
{

    Result current = bracket(s);

    double acc = current.acc;
    while(true) {
        if(current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if((sign != '*' && sign != '/'))
            return current;

        sv next = current.rest.substr(1);
        Result right = bracket(next);

        if(sign == '*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result(acc, right.rest);
    }
}

Result MathParser::num(sv s) // throws Exception
{

    int i = 0;
    int dot_cnt = 0;
    bool negative = false;
    // число также может начинаться с минуса
    if(s.at(0) == '-') {
        negative = true;
        s = s.substr(1);
    }
    // разрешаем только цифры и точку
    while(i < s.length() && (iswdigit(s.at(i)) || s.at(i) == '.')) {
        // но также проверям, что в числе может быть только одна точка!
        if(s.at(i) == '.' && ++dot_cnt > 1)
            throw "not valid number '" % s.substr(0, i + 1) % "'";

        i++;
    }
    if(i == 0) // что-либо похожее на число мы не нашли
        throw "can't get valid number in '" % s % "'";

    double dPart = toDouble(s.substr(0, i));
    if(negative)
        dPart = -dPart;

    sv restPart = s.substr(i);

    return Result(dPart, restPart);
}

Result MathParser::processFunction(sv func, const Result& r) {

    //    if (func.starts_with("sin"))
    //        return Result(sin(r.acc), r.rest);
    //    if (func.starts_with("cos"))
    //        return Result(cos(r.acc), r.rest);
    //    if (func.starts_with("tan"))
    //        return Result(tan(r.acc), r.rest);
    //    return r;

    enum class Func {
        sin,
        cos,
        tan
    };
    switch(Func(QString("sin,cos,tan").split(',').indexOf(toString(func)))) {
    case Func::sin:
        return Result(sin(r.acc), r.rest);
    case Func::cos:
        return Result(cos(r.acc), r.rest);
    case Func::tan:
        return Result(tan(r.acc), r.rest);
    default:
        qWarning() << "function '" << func.data() << "' is not defined";
        break;
    }
    return r;
}

#elif MT == 2

#include <charconv>
using namespace std::literals;

#define USE_TREE 0
#if USE_TREE
#include <QTreeWidget>
extern QTreeWidget* tv;
#else
class QTreeWidgetItem;
#endif

// MathParser::Expression::Expression(std::string_view token)
//     : token{token} { }
// MathParser::Expression::Expression(std::string_view token, Expression a)
//     : token{token}
//     , args {a} { }
// MathParser::Expression::Expression(std::string_view token, Expression a, Expression b)
//     : token{token}
//     , args {a, b} { }
MathParser::MathParser(VarMap&& variables, std::string_view input)
    : input{input}
    , variables{std::move(variables)} { }
MathParser::MathParser(VarMap&& variables)
    : variables{std::move(variables)} { }
MathParser::MathParser(std::string_view input)
    : input{input} { }
std::string_view MathParser::parseToken() {
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
MathParser::Expression MathParser::parseSimpleExpression() {
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
MathParser::Expression MathParser::parseBinaryExpression(int minPriority) {
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
int MathParser::getPriority(std::string_view binaryOp) {
    if(binaryOp.size())
        if(auto it = binaryFunc.find(binaryOp); it != binaryFunc.end())
            return it->second.priority;
    return 0;
}
MathParser::Expression MathParser::parseExpression() {
    return parseBinaryExpression(0);
}
Double MathParser::parse(std::string_view input_) {
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
        auto e = stringToEnum<ParseError>(ex.what());
        auto ret = std::nan("");
        *(uint64_t*)(&ret) |= int(e);
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
Double MathParser::parse() { return eval(parseExpression()); }
////////////////////////////////////////////////////////////////////////////
Double MathParser::eval(const Expression& e, QTreeWidgetItem* twi) {
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

#elif MT == 3

#include "mathparser.h"
#include <QDebug>
// #include <QStringBuilder>
#include <cmath>
#include <functional>

MathParser::MathParser(VarMap* variables)
    : variables{variables} { }

double MathParser::getVariable(QStringView variableName) {
    if(!variables || !variables->contains(variableName.toString())) {
        qWarning() << "Error: Try get unexists variable '" + variableName.toString() + "'";
        return 0.0;
    }
    return variables->at(variableName.toString());
}

double MathParser::parse(const QString& s) {
    Result result;

    try {
        result = plusMinus(s);
        if(result.rest.size())
            qWarning() << "Error: can't full parse\"" + s + "\"rest: " << result.rest;
    } catch(const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

double MathParser::parse(QStringView s) {
    Result result;

    try {
        result = plusMinus(s);
        if(result.rest.size())
            qWarning() << "Error: can't full parse\"" + s.toString() + "\"rest: " << result.rest;
    } catch(const QString& str) {
        qWarning() << str;
    }
    return result.acc;
}

Result MathParser::plusMinus(QStringView s) // throws Exception
{
    Result current = mulDiv(s);
    double acc = current.acc;

    while(current.rest.length() > 0) {
        if(!(current.rest.at(0) == '+' || current.rest.at(0) == '-'))
            break;

        QChar sign = current.rest.at(0);
        QStringView next = current.rest.mid(1);
        if(next.empty())
            throw QString("next.empty()");
        current = mulDiv(next);
        if(sign == '+')
            acc += current.acc;
        else
            acc -= current.acc;
    }
    return Result{acc, current.rest};
}

Result MathParser::bracket(QStringView s) // throws Exception
{
    QChar zeroChar = s.at(0);
    if(zeroChar == '(') {
        Result r = plusMinus(s.mid(1));
        if(!r.rest.isEmpty() && r.rest.at(0) == ')')
            r.rest = r.rest.mid(1);
        else
            qWarning() << "Error: not close bracket";
        return r;
    }
    return functionVariable(s);
}

Result MathParser::functionVariable(QStringView s) // throws Exception
{
    QStringView f;
    int i = 0;
    int sign = +1;
    if(s.startsWith('-')) {
        sign = -1;
        s = s.mid(1); // s.remove(0, 1);
    }
    // ищем название функции или переменной
    // имя обязательно должна начинаться с буквы
    while(i < s.length() && ((s.at(i).isLetter() || s.at(i) == '$') || (s.at(i).isDigit() && i > 0))) {
        // while (i < s.length() && (s.at(i).isLetter() || (s.at(i).isDigit() && i > 0))) {
        // f += s.at(i);
        i++;
    }
    f = s.mid(0, i);

    if(!f.isEmpty()) {                         // если что-нибудь нашли
        if(s.length() > i && s.at(i) == '(') { // и следующий символ скобка значит - это функция
            Result r = bracket(s.mid(f.length()));
            return processFunction(f, r);
        } else // иначе - это переменная
            return Result{getVariable(f) * sign, s.mid(f.length())};
    }
    return num(s);
}

Result MathParser::mulDiv(QStringView s) // throws Exception
{
    if(s.empty())
        throw QString("mulDiv s.empty()");
    Result current = bracket(s);

    double acc = current.acc;
    while(true) {
        if(current.rest.length() == 0)
            return current;

        QChar sign = current.rest.at(0);
        if((sign != '*' && sign != '/'))
            return current;

        QStringView next = current.rest.mid(1);
        if(next.empty())
            throw QString("next.empty()");
        Result right = bracket(next);

        if(sign == '*')
            acc *= right.acc;
        else
            acc /= right.acc;

        current = Result{acc, right.rest};
    }
}

Result MathParser::num(QStringView s) // throws Exception
{
    int i = 0;
    int dot_cnt = 0;
    bool negative = false;
    // число также может начинаться с минуса
    if(s.at(0) == '-') {
        negative = true;
        s = s.mid(1);
    }
    // разрешаем только цифры и точку
    while(i < s.length() && (s.at(i).isDigit() || s.at(i) == '.')) {
        // но также проверям, что в числе может быть только одна точка!
        if(s.at(i) == '.' && ++dot_cnt > 1)
            throw QString("not valid number '" + s.mid(0, i + 1).toString() + "'");

        i++;
    }
    if(i == 0) // что-либо похожее на число мы не нашли
        throw QString("can't get valid number in '" + s.toString() + "'");

    double dPart = s.mid(0, i).toDouble();
    if(negative)
        dPart = -dPart;

    return Result{dPart, s.mid(i)};
}

Result MathParser::processFunction(QStringView func, Result r) {

    using F = double (*)(double);
    static std::unordered_map<QStringView, F> funcMap{
        { u"cos",  [](double val) { return cos(val); }},
        { u"sin",  [](double val) { return sin(val); }},
        {u"sqrt", [](double val) { return sqrt(val); }},
        { u"tan",  [](double val) { return tan(val); }},
    };

    if(funcMap.contains(func))
        return Result{funcMap[func](r.acc), r.rest};
    else
        qWarning() << "function '" + func.toString() + "' is not defined";

    //    enum class Func {
    //        sin,
    //        cos,
    //        tan
    //    };
    //    switch (Func(QString("sin,cos,tan").split(',').indexOf(func))) {
    //    case Func::sin:
    //        return Result {sin(r.acc), r.rest};
    //    case Func::cos:
    //        return Result {cos(r.acc), r.rest};
    //    case Func::tan:
    //        return Result {tan(r.acc), r.rest};
    //    default:
    //        qWarning() << "function '" + func + "' is not defined";
    //        break;
    //    }
    return r;
}
#endif
