#pragma once

#include <type_traits>
#include <list>
#include <iterator>

#include "Calc.hpp"
#include "PropertyData.hpp"

template <typename T, bool Writable>
class BasicProperty;

template <typename T, std::invocable F>
class PropertyBinding;

template <typename T>
class PropertyData;

template <class T>
struct type_traits_impl {
    using value_type = T;
};

template <class T, bool W>
struct type_traits_impl<BasicProperty<T, W>> {
    using value_type = T;
    constexpr static bool writable = W;
};

template <class T, class F>
struct type_traits_impl<PropertyBinding<T, F>> {
    using value_type = T;
    using func_type = F;
};

template <class T>
using value_t = typename type_traits_impl<std::decay_t<T>>::value_type;

template <class T>
using func_t = typename type_traits_impl<std::decay_t<T>>::func_type;

template <class T>
constexpr bool is_writable = type_traits_impl<std::decay_t<T>>::writable;

template <typename T, typename... Args>
concept SameAs = (... || std::same_as<T, std::decay_t<Args>>);

template <class T, class C = std::decay_t<T>>
concept IsProperty = std::same_as<C, BasicProperty<typename C::ValueType, C::IsWritable>>;

template <class T, class C = std::decay_t<T>>
concept IsPropertyBinding = std::same_as<C, PropertyBinding<typename C::ValueType, typename C::FuncType>>;

template <class V>
concept IsNotPB = requires(V&& v) {
    requires !(IsPropertyBinding<V> || IsProperty<V>);
};

template <typename O, typename U, typename V>
concept CanBeCalc = requires(U&& u, V&& v) {
    {O::calc(u, v)};
};

template <typename O, typename T>
concept CanBeCalcOne = requires(T&& v) {
    {O::calc(v)};
};

class BindingContext {
    friend class BindingNotifier;
    std::shared_ptr<bool> ptr = std::make_shared<bool>(true);

public:
    BindingContext(const BindingContext&) = delete;
    BindingContext(BindingContext&&) = default;
    void reset() {
        ptr = std::make_shared<bool>(true);
    }
};

class BindingNotifier {
    struct Data {
        BindingNotifier* obs = nullptr;
    };
    std::shared_ptr<Data> ptr{new Data{this}};

public:
    BindingNotifier() = default;
    BindingNotifier(const BindingNotifier&) = delete;
    BindingNotifier(BindingNotifier&& obs) :
        ptr(std::move(obs.ptr)) { ptr->obs = this; }

    BindingNotifier& operator=(const BindingNotifier& obs) = delete;

    BindingNotifier& operator=(BindingNotifier&& obs) {
        if(&obs == this)
            return *this;
        ptr = std::move(obs.ptr);
        ptr->obs = this;
        return *this;
    }

    // virtual ~BindingNotifier() { }
    // virtual void notify() = 0;

    void notify() {
        std::erase_if(m_observers, [](std::function<bool()>& func) -> bool {
            return func();
        });
        std::erase_if(m_bindings, [](std::weak_ptr<Data>& ptr) -> bool {
            auto data = ptr.lock();
            if(!data)
                return true;
            data->obs->notify();
            return false;
        });
    }

    void resetNotifier() { ptr.reset(); }

    void binding(BindingNotifier* notifier) {
        notifier->addObserver(this);
    }

    void binding(const std::vector<BindingNotifier*>& notifiers) {
        for(auto ntf : notifiers) {
            ntf->addObserver(this);
        }
    }

    void addObserver(BindingNotifier* obs) { m_bindings.push_back(obs->ptr); }

    template <std::invocable F>
    void addObserver(F&& f) {
        m_observers.push_back([func = std::forward<F>(f)] {
            func();
            return false;
        });
    }

    template <std::invocable F>
    void addObserver(const std::weak_ptr<void>& context, F&& f) {
        m_observers.push_back([&context, func = std::forward<F>(f)] {
            if(context.expired())
                return true;
            func();
            return false;
        });
    }

    template <std::invocable F>
    void addObserver(const BindingContext& context, F&& f) {
        addObserver(context.ptr, std::forward<F>(f));
    }

private:
    std::list<std::function<bool()>> m_observers;
    std::list<std::weak_ptr<Data>> m_bindings;
};

// PropertyBinding is mainly for handling ownership. If you assign PropertyBinding to Property,
// it will take its ownership of data, and assigning Property is only to binds it, data is shraed.
template <typename T, std::invocable F>
class PropertyBinding {
    template <typename C, std::invocable U>
    friend class PropertyBinding;

    template <typename U, bool>
    friend class BasicProperty;

    friend class _Binding_Impl;

public:
    using ValueType = T;
    using FuncType = F;

public:
    PropertyBinding(const PropertyBinding<T, F>& o) :
        func(o.func), notifiers(o.notifiers) { }

    PropertyBinding(PropertyBinding<T, F>&& o) :
        func(std::move(o.func)), notifiers(std::move(o.notifiers)) { }

    PropertyBinding(F&& func) :
        func(std::move(func)) { }

    inline T value() const { return func(); }

    /* If there is an implicit transformation,
     * you will lead to the expression of 'property==3' such an expression to create a new type of "PropertyBinding" instance
     * But sometimes you just want to check whether their values are equal
     */
    // operator T() const { return value(); }

private:
    void addNotifier(BindingNotifier* o) { notifiers.push_back(o); }
    void mergeNotifiers(const std::vector<BindingNotifier*>& o) {
        if(notifiers.empty()) {
            notifiers = o;
        } else {
            for(auto ntf : o)
                notifiers.push_back(ntf);
        }
    }

private:
    F func;
    std::vector<BindingNotifier*> notifiers;
};

template <class F>
PropertyBinding(F) -> PropertyBinding<decltype(std::declval<F>()()), F>;

template <typename T, bool Writable>
class BasicProperty {
    friend class _Binding_Impl;
    
    template <typename U, bool>
    friend class BasicProperty;

public:
    using ValueType = T;
    using DataType = PropertyData<T>;
    static constexpr bool IsWritable = Writable;

private:
    std::shared_ptr<DataType> data;
    BindingNotifier binder;

public:
    BasicProperty() :
        data(std::make_shared<DataType>(T{})) { data->m_owner = this; }

    BasicProperty(const T& value) :
        data(std::make_shared<DataType>(value)) { data->m_owner = this; }

    BasicProperty(T&& value) :
        data(std::make_shared<DataType>(std::move(value))) { data->m_owner = this; }

    template <size_t N>
    requires std::same_as<std::string, T>
    BasicProperty(char const (&value)[N]) :
        data(std::make_shared<DataType>(value)) { data->m_owner = this; }

    template<IsProperty P>
    requires std::convertible_to<value_t<P>, T>
    BasicProperty(P&& prop) {
        if constexpr(std::is_rvalue_reference_v<decltype(prop)>)
            _Init_Move(std::move(prop));
        else
            _Init_Copy(prop);
    }

    template <IsPropertyBinding B>
    requires std::convertible_to<value_t<B>, T>
    BasicProperty(B&& b) {
        _Init_From_Binding(std::forward<B>(b));
    }

    ~BasicProperty() {
        freeze();
        if(data->m_owner == this)
            data->m_owner = nullptr;
    }

    // operator T() const { return value(); }

    T value() const { return data->value(); }

    BasicProperty<T, Writable>& operator=(const T& value) requires Writable {
        setValue(value);
        return *this;
    }

    BasicProperty<T, Writable>& operator=(T&& value) requires Writable {
        setValue(std::move(value));
        return *this;
    }

    template<IsProperty P>
    requires Writable && std::convertible_to<value_t<P>, T>
    BasicProperty<T, Writable>& operator=(P&& prop) {
        if constexpr(std::same_as<value_t<P>, T>) {
            if(&prop == this)
                return *this;
        }
        if constexpr(std::is_rvalue_reference_v<decltype(prop)>) {
            _Init_Move(std::move(prop));
        } else {
            binder.resetNotifier();
            _Init_Copy(prop);
        }
        binder.notify();
        return *this;
    }

    template <IsPropertyBinding B>
    requires Writable && std::convertible_to<value_t<B>, T>
    BasicProperty<T, Writable>& operator=(B&& b) {
        binder.resetNotifier();
        _Init_From_Binding(std::forward<B>(b));
        binder.notify();
        return *this;
    }

    template <std::convertible_to<T> VT>
    void setValue(VT&& value) requires Writable {
        if(data->m_owner == this)
            data->setValue(std::forward<VT>(value));
        else {
            binder.resetNotifier();
            data = std::make_shared<DataType>(std::forward<VT>(value));
            data->m_owner = this;
        }
        binder.notify();
    }

    template <std::invocable F>
    void onValueChanged(F&& f) {
        binder.addObserver(std::forward<F>(f));
    }

    template <std::invocable<T> F>
    void onValueChanged(F&& f) {
        binder.addObserver([func = std::forward<F>(f), this] { func(value()); });
    }

    template <class C, std::invocable F>
    requires SameAs<C, BindingContext, std::weak_ptr<void>>
    void onValueChanged(C&& context, F&& f) {
        binder.addObserver(context, std::forward<F>(f));
    }

    template <class C, std::invocable<T> F>
    requires SameAs<C, BindingContext, std::weak_ptr<void>>
    void onValueChanged(C&& context, F&& f) {
        binder.addObserver(context, [func = std::forward<F>(f), this] { func(value()); });
    }

private:
    BindingNotifier* getBinder() const { return const_cast<BindingNotifier*>(&binder); }

    template<typename P>
    inline void _Init_Copy(const P& prop) {
        const_cast<P*>(&prop)->unshare_data();
        if constexpr(std::is_same_v<std::decay_t<T>, value_t<P>>) {
            data = prop.data;
        } else {
            data = std::make_shared<DataType>([data = prop.data] { return data->value(); });
            data->m_owner = this;
        }
        binder.binding(const_cast<BindingNotifier*>(&prop.binder));
    }

    template<typename P>
    inline void _Init_Move(P&& prop) {
        if constexpr(std::is_same_v<std::decay_t<T>, value_t<P>>)
            data = std::move(prop.data);
        else
            data = std::make_shared<DataType>([data = std::move(prop.data)] { return data->value(); });
        data->m_owner = this;
    }

    template <typename B>
    inline void _Init_From_Binding(B&& b) {
        if constexpr(std::is_rvalue_reference_v<decltype(b)>)
            data = std::make_shared<DataType>(std::move(b.func));
        else
            data = std::make_shared<DataType>(b.func);
        data->m_owner = this;
        binder.binding(b.notifiers);
    }

    inline void unshare_data() {
        if(data->m_owner != this) {
            data = std::make_shared<DataType>([v = data] { return v->value(); });
            data->m_owner = this;
        }
    }

    inline void freeze() {
        if(data->m_owner == this)
            data->setValue(data->value());
    }
};

template <typename T>
using property = BasicProperty<T, true>;

template <typename T>
using readonly = BasicProperty<T, false>;


struct _Binding_Impl {
    template <class T, typename F>
    inline static auto getData(const PropertyBinding<T, F>& binding) {
        return binding.func;
    }
    template <class T, typename F>
    inline static auto getData(PropertyBinding<T, F>&& binding) {
        return std::move(binding.func);
    }

    /* ----------------------------------------- */

    template <typename O, IsProperty P, IsNotPB V>
    static inline auto createBinaery(const P& prop, V&& value) {
        PropertyBinding binding{[data = prop.data, v = std::forward<V>(value)] { return O::calc(data->value(), v); }};
        binding.addNotifier(prop.getBinder());
        return binding;
    }

    template <typename O, IsPropertyBinding U, typename V>
    static inline auto createBinaery(U&& binding, V&& value) {
        PropertyBinding b{[func = getData(std::forward<U>(binding)), rv = std::forward<V>(value)] { return O::calc(func(), rv); }};
        b.mergeNotifiers(binding.notifiers);
        return b;
    }

    template <typename O, IsPropertyBinding U, IsProperty P>
    static inline auto createBinaery(U&& binding, const P& prop) {
        PropertyBinding b{[func = getData(std::forward<U>(binding)), rv = prop.data] { return O::calc(func(), rv->value()); }};
        b.mergeNotifiers(binding.notifiers);
        b.addNotifier(prop.getBinder());
        return b;
    }

    /* ----------------------------------------- */

    // ---------- Property and PropertyBinding binds -------------

    template <typename O, IsProperty P1, IsProperty P2>
    requires CanBeCalc<O, value_t<P1>, value_t<P2>>
    static inline auto Operator(const P1& a, const P2& b) {
        PropertyBinding binding{[pa = a.data, pb = b.data] { return O::calc(pa->value(), pb->value()); }};
        binding.addNotifier(a.getBinder());
        binding.addNotifier(b.getBinder());
        return binding;
    }

    template <typename O, IsPropertyBinding U1, IsProperty P>
    requires CanBeCalc<O, value_t<U1>, value_t<P>>
    static inline auto Operator(U1&& binding, const P& prop) {
        return createBinaery<O>(std::forward<U1>(binding), prop);
    }

    template <typename O, IsProperty P, IsPropertyBinding U2>
    requires CanBeCalc<O, value_t<P>, value_t<U2>>
    static inline auto Operator(const P& prop, U2&& binding) {
        return createBinaery<Revers<O>>(std::forward<U2>(binding), prop);
    }

    template <typename O, IsPropertyBinding U1, IsPropertyBinding U2>
    requires CanBeCalc<O, value_t<U1>, value_t<U2>>
    static inline auto Operator(U1&& a, U2&& b) {
        PropertyBinding binding{[pa = getData(std::forward<U1>(a)), pb = getData(std::forward<U2>(b))] { return O::calc(pa(), pb()); }};
        binding.mergeNotifiers(a.notifiers);
        binding.mergeNotifiers(b.notifiers);
        return binding;
    }

    // ---------- ordinary type binds -------------

    template <typename O, IsProperty P, IsNotPB V>
    requires CanBeCalc<O, value_t<P>, V>
    static inline auto Operator(const P& prop, V&& value) {
        return createBinaery<O>(prop, std::forward<V>(value));
    }

    template <typename O, IsNotPB V, IsProperty P>
    requires CanBeCalc<O, value_t<P>, V>
    static inline auto Operator(V&& value, const P& prop) {
        return createBinaery<Revers<O>>(prop, std::forward<V>(value));
    }

    template <typename O, IsPropertyBinding U, IsNotPB V>
    requires CanBeCalc<O, value_t<U>, V>
    static inline auto Operator(U&& binding, V&& value) {
        return createBinaery<O>(std::forward<U>(binding), std::forward<V>(value));
    }

    template <typename O, IsNotPB V, IsPropertyBinding U>
    requires CanBeCalc<O, value_t<U>, V>
    static inline auto Operator(V&& value, U&& binding) {
        return createBinaery<Revers<O>>(std::forward<U>(binding), std::forward<V>(value));
    }

    // ---------- single arg -------------

    template <typename O, IsProperty P>
    requires CanBeCalcOne<O, value_t<P>>
    static inline auto Operator(const P& prop) {
        PropertyBinding b{[v = prop.data]() { return O::calc(v->value()); }};
        b.addNotifier(prop.getBinder());
        return b;
    }

    template <typename O, IsPropertyBinding U>
    requires CanBeCalcOne<O, value_t<U>>
    static inline auto Operator(U&& binding) {
        PropertyBinding b{[func = getData(std::forward<U>(binding))] { return O::calc(func()); }};
        b.mergeNotifiers(binding.notifiers);
        return b;
    }
};

// ---------- concept of operators -------------

template <class U, class V, class O>
concept AccptOperator = requires(U&& u, V&& v) {
    {_Binding_Impl::Operator<O>(std::forward<U>(u), std::forward<V>(v))};
};

template <class U, class O>
concept AccptOperatorS = requires(U&& u) {
    {_Binding_Impl::Operator<O>(std::forward<U>(u))};
};

// ---------- operators -------------

template <typename T1, typename T2, typename O = Plus>
requires AccptOperator<T1, T2, O>
inline auto operator+(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Minus>
requires AccptOperator<T1, T2, O>
inline auto operator-(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Multiplies>
requires AccptOperator<T1, T2, O>
inline auto operator*(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Divides>
requires AccptOperator<T1, T2, O>
inline auto operator/(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Modulus>
requires AccptOperator<T1, T2, O>
inline auto operator%(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T, typename O = Negate>
requires AccptOperatorS<T, O>
inline auto operator-(T&& a) {
    return _Binding_Impl::Operator<O>(std::forward<T>(a));
}

template <typename T1, typename T2, typename O = Equal>
requires AccptOperator<T1, T2, O>
inline auto operator==(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = NotEqual>
requires AccptOperator<T1, T2, O>
inline auto operator!=(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Greater>
requires AccptOperator<T1, T2, O>
inline auto operator>(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = Less>
requires AccptOperator<T1, T2, O>
inline auto operator<(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = GreaterEqual>
requires AccptOperator<T1, T2, O>
inline auto operator>=(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = LessEqual>
requires AccptOperator<T1, T2, O>
inline auto operator<=(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = LogicalAnd>
requires AccptOperator<T1, T2, O>
inline auto operator&&(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = LogicalOr>
requires AccptOperator<T1, T2, O>
inline auto operator||(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T, typename O = LogicalNot>
requires AccptOperatorS<T, O>
inline auto operator!(T&& a) {
    return _Binding_Impl::Operator<O>(std::forward<T>(a));
}

template <typename T1, typename T2, typename O = BitAnd>
requires AccptOperator<T1, T2, O>
inline auto operator&(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = BitOr>
requires AccptOperator<T1, T2, O>
inline auto operator|(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T1, typename T2, typename O = BitXor>
requires AccptOperator<T1, T2, O>
inline auto operator^(T1&& a, T2&& b) {
    return _Binding_Impl::Operator<O>(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename T, typename O = BitNot>
requires AccptOperatorS<T, O>
inline auto operator~(T&& a) {
    return _Binding_Impl::Operator<O>(std::forward<T>(a));
}
