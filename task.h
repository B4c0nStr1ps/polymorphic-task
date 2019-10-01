//
// Created by b4c0nstr1ps on 10/1/19.
//

#ifndef POLYMORPHIC_TASK_TASK_H
#define POLYMORPHIC_TASK_TASK_H

#include <type_traits>
#include <utility>
#include <memory>
#include <functional>

template <class>
class task;

//#define SMALL_OBJECT_OPT

#ifndef SMALL_OBJECT_OPT

template <class R, class... Args>
class task<R(Args...)> {

    struct concept;

    template <class F>
    struct model;

    std::unique_ptr<concept> _p;

public:

    template <class F>
    task(F&& f) : _p(std::make_unique<model<std::decay_t<F>>>(std::forward<F>(f))) {}

    R operator()(Args... args) {
        return _p->_invoke(std::forward<Args>(args)...);
    }
};

template <class R, class... Args>
struct task<R(Args...)>::concept {

    virtual ~concept() = default;

    virtual R _invoke(Args&&...) = 0;
};

template <class R, class... Args>
template <class F>
struct task<R(Args...)>::model final : concept {

    template <class G>
    model(G&& f) : _f(std::forward<G>(f)) {}

    R _invoke(Args&&... args) override {
        return  std::invoke(_f, std::forward<Args>(args)...);
    }

    F _f;
};

#else

template <class R, class... Args>
class task<R(Args...)> {

    struct concept;

    template<class F, bool Small>
    struct model;

    static constexpr std::size_t small_size = sizeof(void*) * 4;

    std::aligned_storage_t<small_size> _data;

    concept& self() { return  *static_cast<concept*>(static_cast<void*>(&_data)); }

public:

    template <class F>
    task(F&& f) {
        constexpr bool is_small = sizeof(model<std::decay_t<F>, true>) <= small_size;
        new (&_data) model<std::decay_t<F>, is_small>(std::forward<F>(f));
    }

    ~task() { self().~concept(); }

    task(task&& x) noexcept { x.self()._move(&_data); }
    task& operator=(task&& x) noexcept {
        self().~concept();
        x.self()._move(&_data);
        return *this;
    }

    R operator() (Args... args) {
        return self()._invoke(std::forward<Args>(args)...);
    }
};

template <class R, class... Args>
struct task<R(Args...)>::concept {

    virtual ~concept() = default;

    virtual R _invoke(Args&&...) = 0;

    virtual void _move(void*) = 0;
};

template <class R, class... Args>
template <class F>
struct task<R(Args...)>::model<F, true> final : concept {

    template <class G>
    model(G&& f) : _f(std::forward<G>(f)) {}

    R _invoke(Args&&... args) override {
        return std::invoke(_f, std::forward<Args>(args)...);
    }

    void _move(void* p) override { new (p) model(std::move(*this)); }

    F _f;
};

template <class R, class... Args>
template <class F>
struct task<R(Args...)>::model<F, false> final : concept {

    template <class G>
    model(G&& f) : _p(std::make_unique<F>(std::forward<F>(f))) {}

    R _invoke(Args&&... args) override {
        return std::invoke(*_p, std::forward<Args>(args)...);
    }

    void _move(void* p) override { new (p) model(std::move(*this)); }

    std::unique_ptr<F> _p;
};
#endif

#endif //POLYMORPHIC_TASK_TASK_H
