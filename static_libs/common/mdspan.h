#pragma once

#include <array>
#include <span>

#if 0
// Synopsis
// namespace std {
// // class template extents
// template <class IndexType, size_t... Extents>
// class extents;

// // alias template dextents
// template <class IndexType, size_t Rank>
// using dextents = /* see description */;

// // layout mapping
// struct layout_left;
// struct layout_right;
// struct layout_stride;

// // class template default_accessor
// template <class ElementType>
// class default_accessor;

// // class template mdspan
// template <class ElementType, class Extents, class LayoutPolicy = layout_right,
//     class AccessorPolicy = default_accessor<ElementType>>
// class mdspan;

// // submdspan creation
// template <class OffsetType, class LengthType, class StrideType>
// struct strided_slice;

// template <class LayoutMapping>
// struct submdspan_mapping_result;

// struct full_extent_t {
//     explicit full_extent_t() = default;
// };
// inline constexpr full_extent_t full_extent{};

// template <class IndexType, class... Extents, class... SliceSpecifiers>
// constexpr auto submdspan_extents(const extents<IndexType, Extents...>&,
//     SliceSpecifiers...);

// // submdspan function template
// template <class ElementType, class Extents, class LayoutPolicy,
//     class AccessorPolicy, class... SliceSpecifiers>
// constexpr auto submdspan(
//     const mdspan<ElementType, Extents, LayoutPolicy, AccessorPolicy>& src,
//     SliceSpecifiers... slices) -> /* see description */;

// template <class T> concept /*integral-constant-like*/ = // exposition only
//     is_integral_v<decltype(T::value)> && !is_same_v<bool, remove_const_t<decltype(T::value)>> && convertible_to<T, decltype(T::value)> && equality_comparable_with<T, decltype(T::value)> && bool_constant<T() == T::value>::value && bool_constant<static_cast<decltype(T::value)>(T()) == T::value>::value;

// template <class T, class IndexType> concept /*index-pair-like*/ = // exposition only
//     /*pair-like*/<T> && convertible_to<tuple_element_t<0, T>, IndexType> && convertible_to<tuple_element_t<1, T>, IndexType>;
// } // namespace std

// Class template std::mdspan
namespace std {
template <class ElementType, class Extents, class LayoutPolicy, class AccessorPolicy>
class mdspan {
public:
    using extents_type = Extents;
    using layout_type = LayoutPolicy;
    using accessor_type = AccessorPolicy;
    using mapping_type = typename layout_type::template mapping<extents_type>;
    using element_type = ElementType;
    using value_type = remove_cv_t<element_type>;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using data_handle_type = typename accessor_type::data_handle_type;
    using reference = typename accessor_type::reference;

    static constexpr rank_type rank() noexcept { return extents_type::rank(); }
    static constexpr rank_type rank_dynamic() noexcept {
        return extents_type::rank_dynamic();
    }
    static constexpr size_t static_extent(rank_type r) noexcept { return extents_type::static_extent(r); }
    constexpr index_type extent(rank_type r) const noexcept {
        return extents().extent(r);
    }

    // constructors
    constexpr mdspan();
    constexpr mdspan(const mdspan& rhs) = default;
    constexpr mdspan(mdspan&& rhs) = default;

    template <class... OtherIndexTypes>
    constexpr explicit mdspan(data_handle_type ptr, OtherIndexTypes... exts);
    template <class OtherIndexType, size_t N>
    constexpr explicit(N != rank_dynamic())
        mdspan(data_handle_type p, span<OtherIndexType, N> exts);
    template <class OtherIndexType, size_t N>
    constexpr explicit(N != rank_dynamic())
        mdspan(data_handle_type p, const array<OtherIndexType, N>& exts);
    constexpr mdspan(data_handle_type p, const extents_type& ext);
    constexpr mdspan(data_handle_type p, const mapping_type& m);
    constexpr mdspan(data_handle_type p, const mapping_type& m, const accessor_type& a);

    template <class OtherElementType, class OtherExtents,
        class OtherLayoutPolicy, class OtherAccessorPolicy>
    constexpr explicit(/* see description */)
        mdspan(const mdspan<OtherElementType, OtherExtents,
            OtherLayoutPolicy, OtherAccessorPolicy>& other);

    constexpr mdspan& operator=(const mdspan& rhs) = default;
    constexpr mdspan& operator=(mdspan&& rhs) = default;

    // members
    template <class... OtherIndexTypes>
    constexpr reference operator[](OtherIndexTypes... indices) const;
    template <class OtherIndexType>
    constexpr reference operator[](span<OtherIndexType, rank()> indices) const;
    template <class OtherIndexType>
    constexpr reference operator[](const array<OtherIndexType, rank()>& indices) const;

    constexpr size_type size() const noexcept;
    [[nodiscard]] constexpr bool empty() const noexcept;

    friend constexpr void swap(mdspan& x, mdspan& y) noexcept;

    constexpr const extents_type& extents() const noexcept { return map_.extents(); }
    constexpr const data_handle_type& data_handle() const noexcept { return ptr_; }
    constexpr const mapping_type& mapping() const noexcept { return map_; }
    constexpr const accessor_type& accessor() const noexcept { return acc_; }

    static constexpr bool is_always_unique() { return mapping_type::is_always_unique(); }
    static constexpr bool is_always_exhaustive() { return mapping_type::is_always_exhaustive(); }
    static constexpr bool is_always_strided() { return mapping_type::is_always_strided(); }

    constexpr bool is_unique() const { return map_.is_unique(); }
    constexpr bool is_exhaustive() const { return map_.is_exhaustive(); }
    constexpr bool is_strided() const { return map_.is_strided(); }
    constexpr index_type stride(rank_type r) const { return map_.stride(r); }

private:
    accessor_type acc_;    // exposition only
    mapping_type map_;     // exposition only
    data_handle_type ptr_; // exposition only
};

template <class CArray>
    requires(is_array_v<CArray> && rank_v<CArray> == 1)
mdspan(CArray&)
    -> mdspan<remove_all_extents_t<CArray>, extents<size_t, extent_v<CArray, 0>>>;

template <class Pointer>
    requires(is_pointer_v<remove_reference_t<Pointer>>)
mdspan(Pointer&&)
    -> mdspan<remove_pointer_t<remove_reference_t<Pointer>>, extents<size_t>>;

template <class ElementType, class... Integrals>
    requires((is_convertible_v<Integrals, size_t> && ...) && sizeof...(Integrals) > 0)
explicit mdspan(ElementType*, Integrals...)
    -> mdspan<ElementType, dextents<size_t, sizeof...(Integrals)>>;

template <class ElementType, class OtherIndexType, size_t N>
mdspan(ElementType*, span<OtherIndexType, N>)
    -> mdspan<ElementType, dextents<size_t, N>>;

template <class ElementType, class OtherIndexType, size_t N>
mdspan(ElementType*, const array<OtherIndexType, N>&)
    -> mdspan<ElementType, dextents<size_t, N>>;

template <class ElementType, class IndexType, size_t... ExtentsPack>
mdspan(ElementType*, const extents<IndexType, ExtentsPack...>&)
    -> mdspan<ElementType, extents<IndexType, ExtentsPack...>>;

template <class ElementType, class MappingType>
mdspan(ElementType*, const MappingType&)
    -> mdspan<ElementType, typename MappingType::extents_type,
        typename MappingType::layout_type>;

template <class MappingType, class AccessorType>
mdspan(const typename AccessorType::data_handle_type&, const MappingType&,
    const AccessorType&)
    -> mdspan<typename AccessorType::element_type, typename MappingType::extents_type,
        typename MappingType::layout_type, AccessorType>;
} // namespace std
  // Class template std::extents
namespace std {
template <class IndexType, size_t... Extents>
class extents {
public:
    using index_type = IndexType;
    using size_type = make_unsigned_t<index_type>;
    using rank_type = size_t;

    // observers of the multidimensional index space
    static constexpr rank_type rank() noexcept { return sizeof...(Extents); }
    static constexpr rank_type rank_dynamic() noexcept {
        return /*dynamic-index*/ (rank());
    }
    static constexpr size_t static_extent(rank_type) noexcept;
    constexpr index_type extent(rank_type) const noexcept;

    // constructors
    constexpr extents() noexcept = default;

    template <class OtherIndexType, size_t... OtherExtents>
    constexpr explicit(/* see description */)
        extents(const extents<OtherIndexType, OtherExtents...>&) noexcept;
    template <class... OtherIndexTypes>
    constexpr explicit extents(OtherIndexTypes...) noexcept;
    template <class OtherIndexType, size_t N>
    constexpr explicit(N != rank_dynamic())
        extents(span<OtherIndexType, N>) noexcept;
    template <class OtherIndexType, size_t N>
    constexpr explicit(N != rank_dynamic())
        extents(const array<OtherIndexType, N>&) noexcept;

    // comparison operators
    template <class OtherIndexType, size_t... OtherExtents>
    friend constexpr bool operator==(const extents&,
        const extents<OtherIndexType,
            OtherExtents...>&) noexcept;

    // exposition-only helpers
    constexpr size_t /*fwd-prod-of-extents*/ (rank_type) const noexcept; // exposition only
    constexpr size_t /*rev-prod-of-extents*/ (rank_type) const noexcept; // exposition only
    template <class OtherIndexType>
    static constexpr auto /*index-cast*/ (OtherIndexType&&) noexcept; // exposition only

private:
    static constexpr rank_type /*dynamic-index*/ (rank_type) noexcept; // exposition only
    static constexpr rank_type
        /*dynamic-index-inv*/ (rank_type) noexcept;           // exposition only
    array<index_type, rank_dynamic()> /*dynamic-extents*/ {}; // exposition only
};

template <class... Integrals>
explicit extents(Integrals...)
    ->/* see description */;
} // namespace std
  // Layout mapping policies
namespace std {
struct layout_left {
    template <class Extents>
    class mapping;
};
struct layout_right {
    template <class Extents>
    class mapping;
};
struct layout_stride {
    template <class Extents>
    class mapping;
};
} // namespace std
  // Class template std::layout_left::mapping
namespace std {
template <class Extents>
class layout_left::mapping {
public:
    using extents_type = Extents;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using layout_type = layout_left;

    // constructors
    constexpr mapping() noexcept = default;
    constexpr mapping(const mapping&) noexcept = default;
    constexpr mapping(const extents_type&) noexcept;
    template <class OtherExtents>
    constexpr explicit(!is_convertible_v<OtherExtents, extents_type>)
        mapping(const mapping<OtherExtents>&) noexcept;
    template <class OtherExtents>
    constexpr explicit(/* see description */)
        mapping(const layout_right::mapping<OtherExtents>&) noexcept;
    template <class OtherExtents>
    explicit(extents_type::rank() > 0) constexpr mapping(const layout_stride::mapping<OtherExtents>&);

    constexpr mapping& operator=(const mapping&) noexcept = default;

    // observers
    constexpr const extents_type& extents() const noexcept { return extents_; }

    constexpr index_type required_span_size() const noexcept;

    template <class... Indices>
    constexpr index_type operator()(Indices...) const noexcept;

    static constexpr bool is_always_unique() noexcept { return true; }
    static constexpr bool is_always_exhaustive() noexcept { return true; }
    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }
    static constexpr bool is_exhaustive() noexcept { return true; }
    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type) const noexcept;

    template <class OtherExtents>
    friend constexpr bool operator==(const mapping&,
        const mapping<OtherExtents>&) noexcept;

private:
    extents_type extents_{}; // exposition only

    // submdspan mapping specialization
    template <class... SliceSpecifiers>
    constexpr auto /*submdspan-mapping-impl*/ ( // exposition only
        SliceSpecifiers... slices) const -> /* see description */;

    template <class... SliceSpecifiers>
    friend constexpr auto submdspan_mapping(
        const mapping& src, SliceSpecifiers... slices) {
        return src./*submdspan-mapping-impl*/ (slices...);
    }
};
} // namespace std
  // Class template std::layout_right::mapping
namespace std {
template <class Extents>
class layout_right::mapping {
public:
    using extents_type = Extents;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using layout_type = layout_right;

    // constructors
    constexpr mapping() noexcept = default;
    constexpr mapping(const mapping&) noexcept = default;
    constexpr mapping(const extents_type&) noexcept;
    template <class OtherExtents>
    constexpr explicit(!is_convertible_v<OtherExtents, extents_type>)
        mapping(const mapping<OtherExtents>&) noexcept;
    template <class OtherExtents>
    constexpr explicit(/* see description */)
        mapping(const layout_left::mapping<OtherExtents>&) noexcept;
    template <class OtherExtents>
    constexpr explicit(extents_type::rank() > 0)
        mapping(const layout_stride::mapping<OtherExtents>&) noexcept;

    constexpr mapping& operator=(const mapping&) noexcept = default;

    // observers
    constexpr const extents_type& extents() const noexcept { return extents_; }

    constexpr index_type required_span_size() const noexcept;

    template <class... Indices>
    constexpr index_type operator()(Indices...) const noexcept;

    static constexpr bool is_always_unique() noexcept { return true; }
    static constexpr bool is_always_exhaustive() noexcept { return true; }
    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }
    static constexpr bool is_exhaustive() noexcept { return true; }
    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type) const noexcept;

    template <class OtherExtents>
    friend constexpr bool operator==(const mapping&,
        const mapping<OtherExtents>&) noexcept;

private:
    extents_type extents_{}; // exposition only

    // submdspan mapping specialization
    template <class... SliceSpecifiers>
    constexpr auto /*submdspan-mapping-impl*/ ( // exposition only
        SliceSpecifiers... slices) const -> /* see description */;

    template <class... SliceSpecifiers>
    friend constexpr auto submdspan_mapping(
        const mapping& src, SliceSpecifiers... slices) {
        return src./*submdspan-mapping-impl*/ (slices...);
    }
};
} // namespace std
  // Class template std::layout_stride::mapping
namespace std {
template <class Extents>
class layout_stride::mapping {
public:
    using extents_type = Extents;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using layout_type = layout_stride;

private:
    static constexpr rank_type rank_ = extents_type::rank(); // exposition only

public:
    // constructors
    constexpr mapping() noexcept = default;
    constexpr mapping(const mapping&) noexcept = default;
    template <class OtherIndexType>
    constexpr mapping(const extents_type&, span<OtherIndexType, rank_>) noexcept;
    template <class OtherIndexType>
    constexpr mapping(const extents_type&, const array<OtherIndexType, rank_>&) noexcept;

    template <class StridedLayoutMapping>
    constexpr explicit(/* see description */) mapping(
        const StridedLayoutMapping&) noexcept;

    constexpr mapping& operator=(const mapping&) noexcept = default;

    // observers
    constexpr const extents_type& extents() const noexcept { return extents_; }
    constexpr array<index_type, rank_> strides() const noexcept { return strides_; }

    constexpr index_type required_span_size() const noexcept;

    template <class... Indices>
    constexpr index_type operator()(Indices...) const noexcept;

    static constexpr bool is_always_unique() noexcept { return true; }
    static constexpr bool is_always_exhaustive() noexcept { return false; }
    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }
    constexpr bool is_exhaustive() const noexcept;
    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type i) const noexcept { return strides_[i]; }

    template <class OtherMapping>
    friend constexpr bool operator==(const mapping&, const OtherMapping&) noexcept;

private:
    extents_type extents_{};             // exposition only
    array<index_type, rank_> strides_{}; // exposition only

    // submdspan mapping specialization
    template <class... SliceSpecifiers>
    constexpr auto /*submdspan-mapping-impl*/ ( // exposition only
        SliceSpecifiers... slices) const -> /* see description */;

    template <class... SliceSpecifiers>
    friend constexpr auto submdspan_mapping(
        const mapping& src, SliceSpecifiers... slices) {
        return src./*submdspan-mapping-impl*/ (slices...);
    }
};
} // namespace std

// Exposition - only helpers
template <class T> constexpr bool /*is-extents*/ = false; // exposition only
template <class SizeType, size_t... Args>
constexpr bool /*is-extents*/<extents<IndexType, Args...>> = true; // exposition only

template <class M> concept /*layout-mapping-alike*/ = requires { // exposition only
    requires /*is-extents*/<typename M::extents_type>;
    { M::is_always_strided() } -> same_as<bool>;
    { M::is_always_exhaustive() } -> same_as<bool>;
    { M::is_always_unique() } -> same_as<bool>;
    bool_constant<M::is_always_strided()>::value;
    bool_constant<M::is_always_exhaustive()>::value;
    bool_constant<M::is_always_unique()>::value;
};

template <class T>
constexpr T /*de-ice*/ (T val) { return val; } // exposition only
template </*integral-constant-like*/ T>
constexpr auto /*de-ice*/ (T) { return T::value; } // exposition only

template <class IndexType, size_t k, class... SliceSpecifiers>
constexpr IndexType /*first_*/ (SliceSpecifiers... slices); // exposition only

template <size_t k, class Extents, class... SliceSpecifiers> // exposition only
constexpr auto /*last_*/ (const Extents& src, SliceSpecifiers... slices);

template <class IndexType, size_t N, class... SliceSpecifiers> // exposition only
constexpr array<IndexType, sizeof...(SliceSpecifiers)>
    /*src-indices*/ (const array<IndexType, N>& indices, SliceSpecifiers... slices);
// Class template std::default_accessor
namespace std {
template <class ElementType>
struct default_accessor {
    using offset_policy = default_accessor;
    using element_type = ElementType;
    using reference = ElementType&;
    using data_handle_type = ElementType*;

    constexpr default_accessor() noexcept = default;
    template <class OtherElementType>
    constexpr default_accessor(default_accessor<OtherElementType>) noexcept;
    constexpr reference access(data_handle_type p, size_t i) const noexcept;
    constexpr data_handle_type offset(data_handle_type p, size_t i) const noexcept;
};
} // namespace std
  // Class template std::strided_slice
namespace std {
template <class OffsetType, class ExtentType, class StrideType>
struct strided_slice {
    using offset_type = OffsetType;
    using extent_type = ExtentType;
    using stride_type = StrideType;

    [[no_unique_address]] offset_type offset{};
    [[no_unique_address]] extent_type extent{};
    [[no_unique_address]] stride_type stride{};
};
} // namespace std
  // Class template std::submdspan_mapping_result
namespace std {
template <class LayoutMapping>
struct submdspan_mapping_result {
    [[no_unique_address]] LayoutMapping mapping = LayoutMapping();
    size_t offset{};
};
} // namespace std
#endif
