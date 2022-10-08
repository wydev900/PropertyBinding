#pragma once
#include <concepts>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <iostream>

template <typename T>
class PropertyData;

template <class T>
using SharedDataType = std::shared_ptr<PropertyData<T>>;

template <class T>
class BasicValue {
public:
    virtual ~BasicValue() { }
    virtual T value() const = 0;
};

template <class T, class D = T, class Operator = Noop>
class OneValue : public BasicValue<T> {
public:
    OneValue(const D& v) :
        data(v) { }

    OneValue(D&& v) :
        data(std::move(v)) { }

    T value() const override { return Operator::calc(data); }

private:
    D data;
};

template <class T, class Operator>
class OneValue<T, SharedDataType<T>, Operator> : public BasicValue<T> {
public:
    OneValue(const SharedDataType<T>& v) :
        data(v) { }

    OneValue(SharedDataType<T>&& v) :
        data(std::move(v)) { }

    T value() const override { return Operator::calc(data->value()); }

private:
    SharedDataType<T> data;
};

// ========================================================

template <class T, class Left, class Right, class Operator>
class TwoValue : public BasicValue<T> {
public:
    TwoValue(const Left& l, const Right& r) :
        lv(l), rv(r) { }

    TwoValue(Left&& l, Right&& r) :
        lv(std::move(l)), rv(std::move(r)) { }

    T value() const override { return Operator::calc(lv, rv); }

private:
    Left lv;
    Right rv;
};

template <class T, class Left, class Right, class Operator>
class TwoValue<T, SharedDataType<Left>, SharedDataType<Right>, Operator> : public BasicValue<T> {
public:
    TwoValue(const SharedDataType<Left>& l, const SharedDataType<Right>& r) :
        lv(l), rv(r) { }

    TwoValue(SharedDataType<Left>&& l, SharedDataType<Right>&& r) :
        lv(std::move(l)), rv(std::move(r)) { }

    T value() const override { return Operator::calc(lv->value(), rv->value()); }

private:
    SharedDataType<Left> lv;
    SharedDataType<Right> rv;
};

template <class T, class Left, class Right, class Operator>
class TwoValue<T, SharedDataType<Left>, Right, Operator> : public BasicValue<T> {
public:
    TwoValue(const SharedDataType<Left>& l, const Right& r) :
        lv(l), rv(r) { }

    TwoValue(SharedDataType<Left>&& l, Right&& r) :
        lv(std::move(l)), rv(std::move(r)) { }

    T value() const override { return Operator::calc(lv->value(), rv); }

private:
    SharedDataType<Left> lv;
    Right rv;
};

template <class T, class Left, class Right, class Operator>
class TwoValue<T, Left, SharedDataType<Right>, Operator> : public BasicValue<T> {
public:
    TwoValue(const Left& l, const SharedDataType<Right>& r) :
        lv(l), rv(r) { }

    TwoValue(Left&& l, SharedDataType<Right>&& r) :
        lv(std::move(l)), rv(std::move(r)) { }

    T value() const override { return Operator::calc(lv, rv->value()); }

private:
    Left lv;
    SharedDataType<Right> rv;
};

template <typename R, typename F>
class FunctorValue : public BasicValue<R> {
public:
    FunctorValue(F&& f) :
        functor(std::forward<F>(f)) { }

    R value() const override { return functor(); }

private:
    F functor;
};

template <typename F>
FunctorValue(F) -> FunctorValue<decltype(std::declval<F>()()), F>;

template <typename T>
class PropertyData {
    template <typename U, bool>
    friend class BasicProperty;

    template <typename C, std::invocable U>
    friend class PropertyBinding;

    using ValueType = T;

public:
    PropertyData(const T& value) :
        m_data(new OneValue<T>{value}) {
        std::cout << " cc-------> " << this << " " << value << std::endl;
    }

    PropertyData(T&& value) :
        m_data(new OneValue<T>{std::move(value)}) {
        std::cout << " mm-------> " << this << " " << value << std::endl;
    }

    PropertyData(const PropertyData<T>& o) = delete;

    PropertyData(PropertyData<T>&& o) = delete;

    template <std::invocable F>
    PropertyData(F&& func) :
        m_data(new FunctorValue<T, F>{std::forward<F>(func)}) {
        std::cout << " func-------> " << this << " " << m_data->value() << std::endl;
    }

    PropertyData(BasicValue<T>* value) :
        m_data(value) {
        std::cout << " BasicValue<T>* -------> " << this << " " << value << std::endl;
    }

    ~PropertyData() {
        if(m_data)
            delete m_data;
        std::cout << " delete-------> " << this << std::endl;
    }

    T value() const { return m_data->value(); }

    template <std::convertible_to<T> U>
    void setValue(U&& value) {
        if(m_data)
            delete m_data;
        m_data = new OneValue<T>{std::forward<U>(value)};
    }

    template <std::invocable F>
    void setValue(F&& func) {
        if(m_data)
            delete m_data;
        m_data = new FunctorValue<T, F>{std::forward<F>(func)};
    }

private:
    BasicValue<T>* m_data;
    void* m_owner = nullptr;
};
