#pragma once

#include "ctre.hpp"
#include "qdebug.h"
#include <QString>
#include <algorithm>
#include <array>
#include <limits>
#include <ranges>
#include <string_view>

using namespace std::literals;

namespace TopoR {

enum ArcDir {
    CW,
    CCW
};

// 23:10:03: Прошло времени: 00:40.
namespace Enumerations { // Все enum в алфавитном порядке

namespace Impl {

namespace ranges = std::ranges;
using std::array;
using std::string_view;

template <class Ty>
inline constexpr bool hasStrings = false;

template <class Ty>
inline constexpr Ty Tokens = Ty{};

template <auto EnumVal>
static consteval auto enumName() {
#ifdef _MSC_VER
    // MSVC: auto __cdecl Impl::enumName<align::CM>(void)
    constexpr string_view sv{__FUNCSIG__};
    constexpr size_t last = sv.find_last_of(">");
#else
    // clang: auto Impl::name() [E = align::CM]
    // gcc: consteval auto Impl::name() [with auto E = align::CM]
    constexpr string_view sv{__PRETTY_FUNCTION__};
    constexpr auto last = sv.find_last_of("]");
#endif
    constexpr size_t first = sv.find_last_of(":", last) + 1;
    array<char, last - first + 1> buf{}; // +1 '\0' tetminated c_str
    ranges::copy(string_view{sv.data() + first, last - first}, buf.begin());
    return buf;
}

template <auto EnumVal>
inline constexpr auto ENameArr{enumName<EnumVal>()};
template <auto EnumVal>
inline constexpr string_view EnumName{ENameArr<EnumVal>.data(), ENameArr<EnumVal>.size() - 1};

template <typename Enum, auto... Enums>
class Tokenizer {
    struct Data {
        string_view name;
        Enum value;
    };
    static constexpr array tokens{
        Data{EnumName<Enums>, Enums}
        ...
    };
    using Ty = std::underlying_type_t<Enum>;
    static constexpr Enum errorValue{static_cast<Enum>(std::numeric_limits<Ty>::max())};

public:
    static constexpr auto toString(Enum e) noexcept {
        auto it = ranges::find(tokens, e, &Data::value);
        return it == tokens.end() ? string_view{""} : it->name;
    }
    static constexpr Enum toEnum(string_view str) noexcept {
        auto it = ranges::find(tokens, str, &Data::name);
        return it == tokens.end() ? errorValue : it->value;
    }
};

} // namespace Impl

#define XML_ENUM(Enum, ...)                                                            \
    enum class Enum : int {                                                            \
        __VA_ARGS__                                                                    \
    };                                                                                 \
    inline auto operator+(Enum e) noexcept { return std::underlying_type_t<Enum>(e); } \
    namespace Impl {                                                                   \
    template <>                                                                        \
    inline constexpr auto hasStrings<Enum> = true;                                     \
    template <>                                                                        \
    inline constexpr auto Tokens<Enum> = [] {                                          \
        using enum Enum; /* using enum ↓  P1099R5 */                                 \
        return Tokenizer<Enum, __VA_ARGS__>();                                         \
    }();                                                                               \
    }

template <typename E, typename Enum = std::remove_cvref_t<E>>
    requires Impl::hasStrings<Enum>
inline constexpr auto enumToString(E e) {
    return Impl::Tokens<Enum>.toString(e);
}

template <typename E, typename Enum = std::remove_cvref_t<E>>
    requires Impl::hasStrings<Enum>
inline constexpr E stringToEnum(std::string_view str) {
    return Impl::Tokens<Enum>.toEnum(str);
}

// Параметр надписей (ярлыков): способ выравнивания текста. Значение по умолчанию – CM.
XML_ENUM(align,
    CM, // по центру
    LT, // по левому верхнему углу
    CT, // по верхнему краю
    RT, // по правому верхнему углу
    LM, // по левому краю
    RM, // по правому краю
    LB, // по левому нижнему углу
    CB, // по нижнему краю
    RB  // по правому нижнему углу
)

// Параметр автоматической трассировки: использование функциональной эквивалентности. Значение по умолчанию – None.
XML_ENUM(autoEqu,
    None,  // не использовать функциональную эквивалентность
    Pins,  // переназначать выводы компонента
    Gates, // переназначать вентили компонентов (не поддерживается)
    Full   // разрешить все переназначения (не поддерживается)
)

// Настройка автоматической подвижки. Значение по умолчанию – MoveVias.
XML_ENUM(automove,
    MoveVias,           // двигаются только переходы
    MoveViasWithRefine, // двигаются только переходы; в процессе движения выполняется перекладка проводников
    MoveCompsWithRefine // двигаются компоненты и переходы; в процессе движения выполняется перекладка проводников
)

// Флаг, значение по умолчанию – off.
XML_ENUM(Bool,
    off, // off,
    on   // on
)

// Параметр области металлизации (полигона) стека: подключение контактных площадок. Значение по умолчанию – Direct.
XML_ENUM(connectPad,
    Direct, // прямое подключение
    Thermal // подключение с помощью термобарьера
)

// Параметр области металлизации (полигона): подключение площадок переходных отверстий. Значение по умолчанию – Direct.
XML_ENUM(connectVia,
    Direct, // прямое подключение
    Thermal // подключение с помощью термобарьера
)

// Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
XML_ENUM(dist,
    mm,  // миллиметр
    mkm, // микрометр
    cm,  // сантиметр
    dm,  // дециметр
    m,   // метр
    mil, // мил(тысячная дюйма)
    inch // дюйм
)

// Параметр области металлизации (полигона): тип заливки. Значение по умолчанию – Solid.
XML_ENUM(fillType,
    Solid,    // сплошная заливка
    Hatched,  // штриховка сеткой
    CRHatched // диагональная штриховка сеткой
)
// Настройка отображения сетки: тип сетки.

XML_ENUM(gridKind,
    Dots, // Dots,
    Lines // Lines
)

// Тип слоя. Значение по умолчанию – Signal.
XML_ENUM(LayerType,
    Signal,     // сигнальный слой
    Assy,       // сборочный слой (слой очертаний компонентов)
    Paste,      // слой паяльной пасты
    Silk,       // слой шелкографии
    Mask,       // слой маски
    Plane,      // опорный слой
    Mechanical, // механический слой
    Doc,        // документирующий слой
    Dielectric  // диэлектрический слой
)

// Настройка автоматической трассировки: режим трассировки. Значение по умолчанию – Multilayer.
XML_ENUM(mode_Autoroute,
    MultiLayer,       // многослойная трассировка
    SingleLayerTop,   // однослойная трассировка на верхнем слое
    SingleLayerBottom // однослойная трассировка на нижнем слое
)

// Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
XML_ENUM(mode_PadConnectSettings,
    AutoConnect, // возможность подключения к углам КП определяется автоматически.
    AllPads      // разрешено подключаться к углам всех КП
)

// Параметр области металлизации (полигона): точность аппроксимации контура. Значение по умолчанию – Med.
XML_ENUM(precision,
    Med, // средняя точность
    Low, // низкая точность
    High // высокая точность
)

// Настройка отображения: единицы измерения. Значение по умолчанию – Metric.
XML_ENUM(preference,
    Metric,   // метрические (конкретные единицы выбираются в зависимости от параметра)
    mkm,      // микрометр
    mm,       // миллиметр
    cm,       // сантиметр
    dm,       // дециметр
    m,        // метр
    Imperial, // английские (конкретные единицы выбираются в зависимости от параметра)
    mil,      // мил(тысячная дюйма)
    inch      // дюйм
)

// Настройка автоматической перекладки проводников. Значение по умолчанию – ChangeLayer.
XML_ENUM(refine,
    ChangeLayer,  // разрешён перенос проводников на другой слой.
    NoChangeLayer // без переноса проводников на другой слой.
)

// Тип запрета трассировки. Значение по умолчанию – Wires
XML_ENUM(role,
    Wires,         // запрет проводников
    Vias,          // запрет переходных отверстий
    Wires_and_Vias // запрет проводников и переходных отверстий
)

// Настройка фильтра сообщений: режим показа предупреждений. Значение по умолчанию – ShowChecked.
XML_ENUM(showWarnings,
    ShowChecked, // показывать только отмеченные предупреждения
    ShowAll,     // показывать все предупреждения
    ShowNothing  // ничего не показывать
)
// Сторона объекта.

/// \note !Значение Both возможно только при описании запретов размещения.
XML_ENUM(side,
    Top,    // верх
    Bottom, // низ
    Both    // обе стороны
)

// Параметр области металлизации (полигона): состояние. Значение по умолчанию – Unpoured.
XML_ENUM(state,
    Unpoured, // незалитая
    Poured,   // залитая
    Locked    // залитая и зафиксированная
)

// Единица измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
XML_ENUM(time,
    ps, // пикосекунда
    fs, // фемтосекунда
    ns, // наносекунда
    us  // микросекунда
)

// Тип предопределённого атрибута компонента. Значение по умолчанию - RefDes
XML_ENUM(type,
    RefDes,  // позиционное обозначение
    PartName // PartName
)

// Параметр стека контактной площадки: подключение к области металлизации (полигону). Значение по умолчанию – NoneConnect.
XML_ENUM(ConnectToCopper,
    NoneConnect, // тип подключения не задан(используются настройки полигона)
    Direct,      // прямое подключение
    Thermal      // подключение с помощью термобарьера
)

// Тип обработки углов прямоугольной контактной площадки.
XML_ENUM(Handling,
    None,     // без обработки
    Rounding, // скругление
    Chamfer   // срез
)

// Тип стека контактных площадок. Значение по умолчанию – Through.
XML_ENUM(type_padstack,
    Through,     // сквозной
    SMD,         // планарный
    MountingHole // монтажное отверстие
)

// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения. Значение по умолчанию – mm.
XML_ENUM(units,
    mm, // миллиметр
    mil // мил (тысячная дюйма)
)

// Параметр правил выравнивания задержек: тип значений констант и допусков. Значение по умолчанию: Dist
XML_ENUM(valueType,
    Dist, // длина
    Time  // время
)

// Параметр автоматической трассировки: форма проводников.
XML_ENUM(wireShape,
    Polyline, // Polyline
    Arcs      // Arcs
)

} // namespace Enumerations
using namespace Enumerations;

} // namespace TopoR
