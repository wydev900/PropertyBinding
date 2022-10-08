#pragma once

#include <utility>


template <typename R, typename... Args>
class FuncHandler {
public:
    virtual R invoke(Args... args) const { return {}; }
};

template <typename F, typename R, typename... Args>
class FuncHandlerC : public FuncHandler<R, Args...> {
public:
    FuncHandlerC(F&& f) :
        functor(std::forward<F>(f)) { }

    R invoke(Args... args) const override { return functor(std::forward<Args>(args)...); }

private:
    F functor;
};

template <typename R, typename... Args>
class Functor;

template <typename R, typename... Args>
class Functor<R(Args...)> {
public:
    template <typename F>
    Functor(F&& f) { handler = new FuncHandlerC<F, R, Args...>(std::forward<F>(f)); }

    R operator()(Args... args) const { return handler->invoke(std::forward<Args>(args)...); }

private:
    FuncHandler<R, Args...>* handler;
};

