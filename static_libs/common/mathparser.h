///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  March 25, 2023                                                  *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2023                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
#pragma once
#ifndef MT
#define MT 1
#endif
#if MT == 1
/**
 * @link   https://habrahabr.ru/post/122397/
 * @author shurik
 */

#include <QMap>
#include <QObject>
#include <string_view>

using VarMap = std::map<QString, double>;
using sv = std::u16string_view;

class Result {
public:
    double acc; // Аккамулятор
    sv rest;    // остаток строки, которую мы еще не обработали
    Result(double v = {}, sv rest = {})
        : acc{v}
        , rest(rest = {}) {
    }
};

class MathParser {
public:
    MathParser(VarMap* variables);
    MathParser() = default;
    double getVariable(const QString& varName);
    double parse(const QString& s = {});

private:
    QString toString(sv s) { return QString((const QChar*)s.data(), s.size()); }

    double toDouble(sv s) { //    double val;        //    std::frochars_(s.data(), s.data() + s.size(), val);
        return toString(s).toDouble();
    }

    VarMap* variables{nullptr};
    Result plusMinus(sv s);
    Result bracket(sv s);
    Result functionVariable(sv s);
    Result mulDiv(sv s);
    Result num(sv s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(sv func, const Result& r);
};
#elif MT == 2
#define USE_ENUM 1
#include "utils.h"
#include <cmath>
#include <map>
#include <numbers>
#include <string_view>
#include <vector>
// #define PMR
#ifdef PMR
#include <array>
#include <iostream>
#include <memory_resource> // pmr core types
#endif
ENUM(ParseError,
    ExpectedRoundBracket, //)
    InvalidInput,
    UnknownBinaryOperator,
    UnknownExpressionType,
    UnknownUnaryOperator //
)
#ifdef PMR
static inline int total;
#endif
using namespace std::numbers;
using Double = double;
class MathParser {
    struct Expression {
        std::string_view token;
        std::vector<Expression> args{};
    };
#ifdef PMR
    static inline std::array<char, 4096 * 2> buffer{}; // a small buffer on the stack
    static inline int tmp = [] {
        if constexpr(0) {
            class noisy_allocator final : public std::pmr::memory_resource {
                void* do_allocate(std::size_t bytes, std::size_t alignment) override {
                    total += bytes;
                    void* p = std::pmr::new_delete_resource()->allocate(bytes, alignment);
                    std::cerr << "+ Allocating " << bytes << " bytes @ " << p << " total -> " << total << std::endl;
                    return p;
                }
                void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
                    std::cerr << "- Deallocating " << bytes << " bytes @ " << p << std::endl;
                    return std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
                }
                bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
                    return std::pmr::new_delete_resource()->is_equal(other);
                }
            };
            static noisy_allocator mem;
            std::pmr::set_default_resource(&mem);
        } else {
            std::fill_n(std::begin(buffer), std::size(buffer) - 1, '@');
            // std::cerr << std::string_view {buffer} << std::endl;
            static std::pmr::monotonic_buffer_resource pool{std::data(buffer), std::size(buffer)};
            std::pmr::set_default_resource(&pool);
        }
        return 0;
    }();
    template <typename Key, typename Val>
    using Map = std::pmr::map<Key, Val>;
#else
    template <typename Key, typename Val>
    using Map = std::map<Key, Val>;
#endif
public:
    using VarMap = std::map<std::string, Double, std::less<>>;
    explicit MathParser(VarMap&& variables, std::string_view input);
    explicit MathParser(VarMap&& variables);
    explicit MathParser(std::string_view input);
    void extracted();
    Double parse(std::string_view input_);
    Double parse();
    ~MathParser() = default;

private:
    Expression parseExpression();
    Double eval(const Expression& e, class QTreeWidgetItem* twi = nullptr);
    std::string_view parseToken();
    Expression parseSimpleExpression();
    Expression parseBinaryExpression(int minPriority);
    int getPriority(std::string_view binaryOp);
    std::string_view input;
    VarMap variables;
    // clang-format off
#define MATH_FUNC(F) { #F, [](Double val) -> Double { return std::F(val); }}
    static inline Map<std::string_view, Double (*const)(Double)> unaryFunc {
        // {
        {"+",   [](Double val)     -> Double { return +val;                 }},
        {"-",   [](Double val)     -> Double { return -val;                 }},
        {"~",   [](Double val)     -> Double { return ~(int32_t)val;        }},
        {"deg", [](Double radians) -> Double { return radians * (180 / pi); }},
        {"rad", [](Double degrees) -> Double { return degrees * (pi / 180); }},
        MATH_FUNC( abs       ),
        MATH_FUNC( abs       ),
        MATH_FUNC( abs       ),
        MATH_FUNC( acos      ),
        MATH_FUNC( acosh     ),
        MATH_FUNC( asin      ),
        MATH_FUNC( asinh     ),
        MATH_FUNC( atan      ),
        MATH_FUNC( atanh     ),
        MATH_FUNC( cbrt      ),
        MATH_FUNC( ceil      ),
        MATH_FUNC( cos       ),
        MATH_FUNC( cos       ),
        MATH_FUNC( cosh      ),
        MATH_FUNC( erf       ),
        MATH_FUNC( erfc      ),
        MATH_FUNC( exp       ),
        MATH_FUNC( exp2      ),
        MATH_FUNC( expm1     ),
        MATH_FUNC( floor     ),
        MATH_FUNC( ilogb     ),
        MATH_FUNC( lgamma    ),
        MATH_FUNC( log       ),
        MATH_FUNC( log       ),
        MATH_FUNC( log10     ),
        MATH_FUNC( log1p     ),
        MATH_FUNC( log2      ),
        MATH_FUNC( logb      ),
        MATH_FUNC( nearbyint ),
        MATH_FUNC( rint      ),
        MATH_FUNC( round     ),
        MATH_FUNC( sin       ),
        MATH_FUNC( sin       ),
        MATH_FUNC( sinh      ),
        MATH_FUNC( sqrt      ),
        MATH_FUNC( sqrt      ),
        MATH_FUNC( tan       ),
        MATH_FUNC( tan       ),
        MATH_FUNC( tanh      ),
        MATH_FUNC( tgamma    ),
        MATH_FUNC( trunc     ),
        // },
        // &pool
    };
#undef MATH_FUNC
    struct Func {
        const int priority;
        Double (*const func)(Double, Double);
    };
#define MATH_FUNC(F,PRIORITY,OP)  {#F, Func{PRIORITY, [](Double a, Double b) -> Double { return OP; }}}
    static inline Map<std::string_view, Func> binaryFunc {
        // {
        MATH_FUNC( %   , 2 , long(a) % long(b)                   ),
        MATH_FUNC( *   , 2 , a * b                               ),
        MATH_FUNC( +   , 1 , a + b                               ),
        MATH_FUNC( -   , 1 , a - b                               ),
        MATH_FUNC( /   , 2 , a / b                               ),
        MATH_FUNC( <   , 3 , a < b                               ),
        MATH_FUNC( <<  , 3 , long(a) << long(b)                  ),
        MATH_FUNC( ==  , 3 , (std::abs(a) - std::abs(b)) < 1.e-6 ),
        MATH_FUNC( >   , 3 , a > b                               ),
        MATH_FUNC( >>  , 3 , long(a) >> long(b)                  ),
        MATH_FUNC( ^   , 4 , pow(a , b)                          ),
        MATH_FUNC( and , 3 , long(a) & long(b)                   ),
        MATH_FUNC( mod , 2 , long(a) % long(b)                   ),
        MATH_FUNC( or  , 3 , long(a) | long(b)                   ),
        MATH_FUNC( xor , 3 , long(a) ^ long(b)                   ),
        // },
        // &pool
    };
#undef MATH_FUNC
    // clang-format on
};

using VarMap = MathParser::VarMap;
/*
Precedence    Operator            Description                                                     Associativity
1           ::                  Scope resolution                                                Left-to-right →
2           a++   a--           Suffix/postfix increment and decrement
            type()   type{}     Functional cast
            a()                 Function call
            a[]                 Subscript
            .   ->              Member access
3           ++a   --a           Prefix increment and decrement                                  Right-to-left ←
            +a   -a             Unary plus and minus
            !   ~               Logical NOT and bitwise NOT
            (type)              C-style cast
            *a Indirection (dereference)
            &a                  Address-of
            sizeof              Size-of[note 1]
            co_await            await-expression (C++20)
            new new[]         Dynamic memory allocation
            delete delete[]   Dynamic memory deallocation
4           .*   ->*            Pointer-to-member                                               Left-to-right →
5           a*b   a/b   a%b     Multiplication, division, and remainder
6           a+b   a-b           Addition and subtraction
7           <<   >>             Bitwise left shift and right shift
8           <=>                 Three-way comparison operator (since C++20)
9           <   <=   >   >=     For relational operators < and ≤ and > and ≥ respectively
10          ==   !=             For equality operators = and ≠ respectively
11          a&b                 Bitwise AND
12          ^                   Bitwise XOR (exclusive or)
13          |                   Bitwise OR (inclusive or)
14          &&                  Logical AND
15          ||                  Logical OR
16          a?b:c Ternary conditional[note 2] Right-to-left ←
            throw throw operator
            co_yield            yield-expression (C++20)
            =                   Direct assignment (provided by default for C++ classes)
            +=   -=             Compound assignment by sum and difference
            *=   /=   %=        Compound assignment by product, quotient, and remainder
            <<=   >>=           Compound assignment by bitwise left shift and right shift
            &=   ^=   |=        Compound assignment by bitwise AND, XOR, and OR
17          ,                   Comma                                       Left-to-right →
*/
#elif MT == 3

#include <QObject>

using VarMap = std::map<QString, double>;

class Result {
public:
    double acc{};       // Аккамулятор
    QStringView rest{}; // остаток строки, которую мы еще не обработали
    //    Result(double v = 0.0, const QStringView& r = {})
    //        : acc{v}
    //        , rest(r) {
    //    }
};

class MathParser {
public:
    MathParser(VarMap* variables);
    double getVariable(QStringView variableName);
    double parse(const QString& s = "");
    double parse(QStringView s = {});

private:
    VarMap* variables{nullptr};
    Result plusMinus(QStringView s);
    Result bracket(QStringView s);
    Result functionVariable(QStringView s);
    Result mulDiv(QStringView s);
    Result num(QStringView s);
    // Тут определяем все нашие функции, которыми мы можем пользоватся в формулах
    Result processFunction(QStringView func, Result r);
};
#endif
