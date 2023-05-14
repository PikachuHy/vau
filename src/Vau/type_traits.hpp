#pragma once
#include <functional>
#include <memory>

template <typename Function>
struct function_traits;

template <typename Return, typename... Arguments>
struct function_traits<Return (*)(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct function_traits<Return (*)(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct function_traits<Return(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename Return, typename... Arguments>
struct function_traits<Return(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
};

template <typename This, typename Return, typename... Arguments>
struct function_traits<Return (This::*)(Arguments...)> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct function_traits<Return (This::*)(Arguments...) noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct function_traits<Return (This::*)(Arguments...) const> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return, typename... Arguments>
struct function_traits<Return (This::*)(Arguments...) const noexcept> {
  using parameters_type = std::tuple<std::remove_cvref_t<Arguments>...>;
  using return_type = Return;
  using class_type = This;
};

template <typename Return>
struct function_traits<Return (*)()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct function_traits<Return (*)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct function_traits<Return (&)()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct function_traits<Return (&)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct function_traits<Return()> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename Return>
struct function_traits<Return() noexcept> {
  using parameters_type = void;
  using return_type = Return;
};

template <typename This, typename Return>
struct function_traits<Return (This::*)()> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct function_traits<Return (This::*)() noexcept> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct function_traits<Return (This::*)() const> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

template <typename This, typename Return>
struct function_traits<Return (This::*)() const noexcept> {
  using parameters_type = void;
  using return_type = Return;
  using class_type = This;
};

// Support function object and lambda expression
template <class Function>
struct function_traits : function_traits<decltype(&Function::operator())> {};

template <typename Function>
using function_parameters_t =
    typename function_traits<std::remove_cvref_t<Function>>::parameters_type;

template <typename Function>
using last_parameters_type_t =
    std::tuple_element_t<std::tuple_size_v<function_parameters_t<Function>> - 1,
                         function_parameters_t<Function>>;

template <typename Function>
using function_return_type_t =
    typename function_traits<std::remove_cvref_t<Function>>::return_type;

template <typename Function>
using class_type_t =
    typename function_traits<std::remove_cvref_t<Function>>::class_type;

template <typename F, typename... Args>
struct is_invocable
    : std::is_constructible<
          std::function<void(std::remove_reference_t<Args>...)>,
          std::reference_wrapper<typename std::remove_reference<F>::type>> {};

template <typename F, typename... Args>
inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;
