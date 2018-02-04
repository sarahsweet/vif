#ifndef PHYPP_INCLUDING_CORE_VEC_BITS
#error this file is not meant to be included separately, include "phypp/core/vec.hpp" instead
#endif

namespace phypp {
    // Generic vector type
    template<std::size_t Dim, typename Type>
    struct vec;

namespace meta {

    // Helper to get the vector internal storage type.
    // This is used to avoid std::vector<bool>.
    template<typename T>
    struct dtype {
        using type = T;
    };
    template<>
    struct dtype<bool> {
        using type = char;
    };

    template<typename T>
    using dtype_t = typename dtype<T>::type;

    // Helper to get the decayed data type of a vector
    // vec<D,T>       = T
    // vec<D,T*>      = T
    // vec<D,const T> = T
    template<typename T>
    using rtype_t = typename std::remove_cv<
        typename std::remove_pointer<
            typename std::remove_cv<T>::type
        >::type
    >::type;
}

namespace impl {
    namespace meta_impl {
        template<typename T>
        struct is_vec_ : public std::false_type {};

        template<std::size_t Dim, typename Type>
        struct is_vec_<vec<Dim,Type>> : public std::true_type {};

        template<typename T>
        struct vec_dim_ : std::integral_constant<std::size_t,0> {};

        template<std::size_t Dim, typename T>
        struct vec_dim_<vec<Dim,T>> : std::integral_constant<std::size_t,Dim> {};
    }
}

namespace meta {
    // Helper to check if a given type is a generic vector.
    template<typename T>
    using is_vec = impl::meta_impl::is_vec_<typename std::decay<T>::type>;

    // Return the data type of the provided type
    template<typename T>
    struct data_type {
        using type = T;
    };

    template<std::size_t Dim, typename T>
    struct data_type<vec<Dim,T>> {
        using type = T;
    };

    template<typename T>
    using data_type_t = typename data_type<T>::type;

    // Return the dimension of a vector
    template<typename T>
    struct vec_dim : impl::meta_impl::vec_dim_<typename std::decay<T>::type> {};

    // Helper to match a type to the corresponding vector internal type.
    template<typename T>
    struct vtype {
        using type = T;
    };

    template<>
    struct vtype<char*> {
        using type = std::string;
    };

    template<>
    struct vtype<const char*> {
        using type = std::string;
    };

    template<std::size_t N>
    struct vtype<char[N]> {
        using type = std::string;
    };

    template<typename T>
    using vtype_t = typename vtype<typename std::decay<T>::type>::type;

    // Compute the total number of dimensions generated by a set of arguments (for indexing)
    template<typename T>
    struct dim_index : std::integral_constant<std::size_t, 1> {};
    template<std::size_t N, typename T>
    struct dim_index<std::array<T,N>> : std::integral_constant<std::size_t, N> {};

    template<typename T, typename ... Args>
    struct dim_total : std::integral_constant<std::size_t,
        dim_index<typename std::decay<T>::type>::value + dim_total<Args...>::value> {};

    template<typename T>
    struct dim_total<T> : std::integral_constant<std::size_t,
        dim_index<typename std::decay<T>::type>::value> {};
    }

    // Trait to figure out if type list matches an dimension list
namespace impl {
    namespace meta_impl {
        template<typename T>
        struct is_dim_elem_ : std::is_integral<T> {};

        template<typename T, std::size_t N>
        struct is_dim_elem_<std::array<T,N>> : std::true_type {};
    }
}

namespace meta {
    template<typename T>
    using is_dim_elem = impl::meta_impl::is_dim_elem_<typename std::decay<T>::type>;

    template<typename ... Args>
    struct is_dim_list : std::integral_constant<bool,
        are_all_true<bool_list<is_dim_elem<Args>::value...>>::value> {};

    template<>
    struct is_dim_list<> : std::true_type {};
}

namespace impl {
    namespace meta_impl {
        template<typename TFrom, typename TTo>
        struct vec_implicit_convertible_ : std::is_convertible<TFrom,TTo> {};

        // Implicit conversion to/from bool is disabled
        template<typename TFrom>
        struct vec_implicit_convertible_<TFrom,bool> : std::false_type {};
        template<typename TTo>
        struct vec_implicit_convertible_<bool,TTo> : std::false_type {};
        template<>
        struct vec_implicit_convertible_<bool,bool> : std::true_type{};

        template<typename TFrom, typename TTo>
        struct vec_explicit_convertible_ : std::is_convertible<TFrom,TTo> {};
    }
}

namespace meta {
    // Trait to define if a vector type can be implicitly converted into another
    template<typename TFrom, typename TTo>
    using vec_implicit_convertible = impl::meta_impl::vec_implicit_convertible_<
        typename std::decay<typename std::remove_pointer<TFrom>::type>::type,
        typename std::decay<typename std::remove_pointer<TTo>::type>::type
    >;

    // Trait to define if a vector type can be explicitly converted into another
    template<typename TFrom, typename TTo>
    using vec_explicit_convertible = impl::meta_impl::vec_explicit_convertible_<
        typename std::decay<typename std::remove_pointer<TFrom>::type>::type,
        typename std::decay<typename std::remove_pointer<TTo>::type>::type
    >;

    // Trait to define if a vector type can be converted *only* explicitly into another
    template<typename TFrom, typename TTo>
    using vec_only_explicit_convertible = std::integral_constant<bool,
            vec_explicit_convertible<TFrom,TTo>::value &&
            !vec_implicit_convertible<TFrom, TTo>::value>;
} // end namespace meta

namespace impl {
    // Helpers to reference/dereference variables only when necessary.
    // 'Type' must be the second template argument of the vector from which this
    // value comes, i.e. vec<Dim,Type>.
    template<typename Type, typename T>
    meta::rtype_t<Type>& dref(T& t) {
        return reinterpret_cast<meta::rtype_t<Type>&>(t);
    }
    template<typename Type, typename T>
    const meta::rtype_t<Type>& dref(const T& t) {
        return reinterpret_cast<const meta::rtype_t<Type>&>(t);
    }

    template<typename Type, typename T>
    meta::rtype_t<Type>& dref(T* t) {
        return reinterpret_cast<meta::rtype_t<Type>&>(*t);
    }
    template<typename Type, typename T>
    const meta::rtype_t<Type>& dref(const T* t) {
        return reinterpret_cast<const meta::rtype_t<Type>&>(*t);
    }

    template<typename Type, typename T>
    meta::rtype_t<Type>* ptr(T* t) {
        return reinterpret_cast<meta::rtype_t<Type>*>(t);
    }
    template<typename Type, typename T>
    const meta::rtype_t<Type>* ptr(const T* t) {
        return reinterpret_cast<const meta::rtype_t<Type>*>(t);
    }

    template<typename Type, typename T>
    meta::rtype_t<Type>* ptr(T& t) {
        return reinterpret_cast<meta::rtype_t<Type>*>(&t);
    }
    template<typename Type, typename T>
    const meta::rtype_t<Type>* ptr(const T& t) {
        return reinterpret_cast<const meta::rtype_t<Type>*>(&t);
    }

    // Assign an arbitrary list of values to an array.
    // Supports scalars and other arrays.
    template<std::size_t N, typename T, std::size_t I>
    void set_array_(std::array<T,N>& v, meta::cte_t<I>) {}

    template<std::size_t N, typename T, std::size_t I, typename U, std::size_t M, typename ... Args>
    void set_array_(std::array<T,N>& v, meta::cte_t<I>, const std::array<U,M>& t, Args&& ... args);

    template<std::size_t N, typename T, std::size_t I, typename U, typename ... Args>
    void set_array_(std::array<T,N>& v, meta::cte_t<I>, const U& t, Args&& ... args) {
        v[I] = static_cast<T>(t);
        set_array_(v, meta::cte_t<I+1>{}, std::forward<Args>(args)...);
    }

    template<std::size_t N, typename T, std::size_t I, typename U, std::size_t M, typename ... Args>
    void set_array_(std::array<T,N>& v, meta::cte_t<I>, const std::array<U,M>& t, Args&& ... args) {
        for (uint_t i : range(M)) {
            v[I+i] = static_cast<T>(t[i]);
        }

        set_array_(v, meta::cte_t<I+M>{}, std::forward<Args>(args)...);
    }

    template<std::size_t N, typename T, typename ... Args>
    void set_array(std::array<T,N>& v, Args&& ... args) {
        static_assert(N == meta::dim_total<Args...>::value, "wrong number of elements for this array");
        set_array_(v, meta::cte_t<0>{}, std::forward<Args>(args)...);
    }
} // end namespace impl

namespace meta {
    // Get the dimensions of a vector or scalar
    template<std::size_t Dim, typename T>
    vec1u dims(const vec<Dim,T>& v) {
        vec1u d(Dim);
        for (uint_t i : range(Dim)) {
            d.safe[i] = v.dims[i];
        }
        return d;
    }

    template<typename T, typename enable = typename std::enable_if<!meta::is_vec<T>::value>::type>
    vec1u dims(const T& t) {
        return {1u};
    }

    // Trait to identify output types: vec<D,T>& or vec<D,T*>
    template<typename T>
    struct is_output_type : std::integral_constant<bool,
        !std::is_const<typename std::remove_reference<T>::type>::value &&
        std::is_reference<T>::value> {};

    template<std::size_t D, typename T>
    struct is_output_type<vec<D,T>> : std::integral_constant<bool,
        !std::is_const<typename std::remove_pointer<T>::type>::value &&
        std::is_pointer<T>::value> {};

    // Trait to identify output types compatible with input type
    template<typename I, typename O>
    struct is_compatible_output_type : std::integral_constant<bool,
        is_output_type<O>::value && is_vec<I>::value == is_vec<O>::value &&
        vec_dim<I>::value == vec_dim<O>::value> {};

    // Resize vectors to provided dimensions, or check views have the right dimensions
    template<std::size_t Dim1, typename T, std::size_t Dim2>
    void resize_or_check(const vec<Dim1,T*>& s, const std::array<uint_t,Dim2>& dims) {
        phypp_check(s.dims == dims, "incompatible dimensions for view (expected ", dims, ", "
            "got ", s.dims);
    }

    template<std::size_t Dim, typename T,
        typename enable = typename std::enable_if<!std::is_pointer<T>::value>::type>
    void resize_or_check(vec<Dim,T>& s, const std::array<uint_t,Dim>& dims) {
        s.resize(dims);
    }
} // end namespace meta

namespace impl {
namespace meta_impl {
    template<typename T, bool hasNaN>
    struct strict_comparator_less_;

    template<typename T>
    struct strict_comparator_less_<T, false> {
        bool operator() (const T& t1, const T& t2) {
            return t1 < t2;
        }
    };

    template<typename T>
    struct strict_comparator_less_<T, true> {
        bool operator() (const T& t1, const T& t2) {
            if (std::isnan(t1)) return false;
            if (std::isnan(t2)) return true;
            return t1 < t2;
        }
    };

    template<typename T, bool hasNaN>
    struct strict_comparator_greater_;

    template<typename T>
    struct strict_comparator_greater_<T, false> {
        bool operator() (const T& t1, const T& t2) {
            return t1 > t2;
        }
    };

    template<typename T>
    struct strict_comparator_greater_<T, true> {
        bool operator() (const T& t1, const T& t2) {
            if (std::isnan(t1)) return false;
            if (std::isnan(t2)) return true;
            return t1 > t2;
        }
    };
} // end namespace meta_impl
} // end namespace impl

namespace meta {
    template<typename T>
    using strict_comparator_less = impl::meta_impl::strict_comparator_less_<T,
        std::numeric_limits<T>::has_quiet_NaN || std::numeric_limits<T>::has_signaling_NaN
    >;
    template<typename T>
    using strict_comparator_greater = impl::meta_impl::strict_comparator_greater_<T,
        std::numeric_limits<T>::has_quiet_NaN || std::numeric_limits<T>::has_signaling_NaN
    >;
} // end namespace meta
} // end namespace phypp
