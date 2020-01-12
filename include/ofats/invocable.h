#ifndef _ANY_INVOKABLE_H_
#define _ANY_INVOKABLE_H_

#include <functional>
#include <memory>
#include <type_traits>

// clang-format off
/*
namespace std {
  template<class Sig> class any_invocable; // never defined

  template<class R, class... ArgTypes>
  class any_invocable<R(ArgTypes...) cv ref noexcept(noex)> {
  public:
    using result_type = R;

    // SECTION.3, construct/copy/destroy
    any_invocable() noexcept;
    any_invocable(nullptr_t) noexcept;
    any_invocable(any_invocable&&) noexcept;
    template<class F> any_invocable(F&&);

    template<class T, class... Args>
      explicit any_invocable(in_place_type_t<T>, Args&&...);
    template<class T, class U, class... Args>
      explicit any_invocable(in_place_type_t<T>, initializer_list<U>, Args&&...);

    any_invocable& operator=(any_invocable&&) noexcept;
    any_invocable& operator=(nullptr_t) noexcept;
    template<class F> any_invocable& operator=(F&&);
    template<class F> any_invocable& operator=(reference_wrapper<F>) noexcept;

    ~any_invocable();

    // SECTION.4, any_invocable modifiers
    void swap(any_invocable&) noexcept;

    // SECTION.5, any_invocable capacity
    explicit operator bool() const noexcept;

    // SECTION.6, any_invocable invocation
    R operator()(ArgTypes...) cv ref noexcept(noex);

    // SECTION.7, null pointer comparisons
    friend bool operator==(const any_invocable&, nullptr_t) noexcept;

    // SECTION.8, specialized algorithms
    friend void swap(any_invocable&, any_invocable&) noexcept;
  };
}
*/
// clang-format on

namespace ofats {

namespace any_detail {

using buffer = std::aligned_storage_t<sizeof(void*) * 2, alignof(void*)>;

template <class T>
inline constexpr bool is_small_object_v =
    sizeof(T) <= sizeof(buffer) && alignof(buffer) % alignof(T) == 0 &&
    std::is_nothrow_move_constructible_v<T>;

union storage {
  void* ptr_ = nullptr;
  buffer buf_;
};

enum class action { destroy, move };

template <class R, class... ArgTypes>
struct handler_traits {
  template <class Derived>
  struct handler_base {
    static void handle(action act, storage* current, storage* other = nullptr) {
      switch (act) {
        case (action::destroy):
          Derived::destroy(*current);
          break;
        case (action::move):
          Derived::move(*current, *other);
          break;
      }
    }
  };

  template <class T>
  struct small_handler : handler_base<small_handler<T>> {
    template <class... Args>
    static void create(storage& s, Args&&... args) {
      new (static_cast<void*>(&s.buf_)) T(std::forward<Args>(args)...);
    }

    static void destroy(storage& s) noexcept {
      T& value = *static_cast<T*>(static_cast<void*>(&s.buf_));
      value.~T();
    }

    static void move(storage& dst, storage& src) noexcept {
      create(dst, std::move(*static_cast<T*>(static_cast<void*>(&src.buf_))));
      destroy(src);
    }

    static R call(storage& s, ArgTypes... args) {
      return std::invoke(*static_cast<T*>(static_cast<void*>(&s.buf_)),
                         std::forward<ArgTypes>(args)...);
    }
  };

  template <class T>
  struct large_handler : handler_base<large_handler<T>> {
    template <class... Args>
    static void create(storage& s, Args&&... args) {
      s.ptr_ = new T(std::forward<Args>(args)...);
    }

    static void destroy(storage& s) noexcept { delete static_cast<T*>(s.ptr_); }

    static void move(storage& dst, storage& src) noexcept {
      dst.ptr_ = src.ptr_;
    }

    static R call(storage& s, ArgTypes... args) {
      return std::invoke(*static_cast<T*>(s.ptr_),
                         std::forward<ArgTypes>(args)...);
    }
  };

  template <class T>
  using handler = std::conditional_t<is_small_object_v<T>, small_handler<T>,
                                     large_handler<T>>;
};

template <class T>
struct is_in_place_type : std::false_type {};

template <class T>
struct is_in_place_type<std::in_place_type_t<T>> : std::true_type {};

template <class T>
inline constexpr auto is_in_place_type_v = is_in_place_type<T>::value;

}  // namespace any_detail

template <class Signature>
class any_invocable;

template <class R, class... ArgTypes>
class any_invocable<R(ArgTypes...)> {
  template <class T>
  using handler =
      typename any_detail::handler_traits<R, ArgTypes...>::template handler<T>;

  using storage = any_detail::storage;
  using action = any_detail::action;
  using handle_func = void (*)(any_detail::action, any_detail::storage*,
                               any_detail::storage*);
  using call_func = R (*)(any_detail::storage&, ArgTypes...);

 public:
  using result_type = R;

 public:
  any_invocable() noexcept = default;
  any_invocable(std::nullptr_t) noexcept {}
  any_invocable(any_invocable&& rhs) noexcept {
    if (rhs.handle_) {
      handle_ = rhs.handle_;
      handle_(action::move, &storage_, &rhs.storage_);
      call_ = rhs.call_;
      rhs.handle_ = nullptr;
    }
  }
  template <
      class F, class FDec = std::decay_t<F>,
      class = std::enable_if_t<!std::is_same_v<FDec, any_invocable> &&
                               !any_detail::is_in_place_type_v<FDec> &&
                               std::is_constructible_v<FDec, F> &&
                               std::is_move_constructible_v<FDec> &&
                               std::is_invocable_r_v<R, FDec, ArgTypes...>>>
  any_invocable(F&& f) {
    create<FDec>(std::forward<F>(f));
  }

  template <class T, class... Args, class VT = std::decay_t<T>,
            class = std::enable_if_t<std::is_move_constructible_v<VT> &&
                                     std::is_constructible_v<VT, Args...> &&
                                     std::is_invocable_r_v<R, VT, ArgTypes...>>>
  explicit any_invocable(std::in_place_type_t<T>, Args&&... args) {
    create<VT>(std::forward<Args>(args)...);
  }

  template <
      class T, class U, class... Args, class VT = std::decay_t<T>,
      class = std::enable_if_t<
          std::is_move_constructible_v<VT> &&
          std::is_constructible_v<VT, std::initializer_list<U>&, Args...> &&
          std::is_invocable_r_v<R, VT, ArgTypes...>>>
  explicit any_invocable(std::in_place_type_t<T>, std::initializer_list<U> il,
                         Args&&... args) {
    create<VT>(il, std::forward<Args>(args)...);
  }

  any_invocable& operator=(any_invocable&& rhs) noexcept {
    any_invocable{std::move(rhs)}.swap(*this);
    return *this;
  }
  any_invocable& operator=(std::nullptr_t) noexcept {
    destroy();
    return *this;
  }
  template <class F, class FDec = std::decay_t<F>>
  std::enable_if_t<!std::is_same_v<FDec, any_invocable> &&
                       std::is_move_constructible_v<FDec>,
                   any_invocable&>
  operator=(F&& f) {
    any_invocable{std::forward<F>(f)}.swap(*this);
    return *this;
  }
  template <class F>
  any_invocable& operator=(std::reference_wrapper<F> f) {
    any_invocable{f}.swap(*this);
    return *this;
  }

  ~any_invocable() { destroy(); }

  void swap(any_invocable& rhs) noexcept {
    if (handle_) {
      if (rhs.handle_) {
        storage tmp;
        handle_(action::move, &tmp, &storage_);
        rhs.handle_(action::move, &storage_, &rhs.storage_);
        handle_(action::move, &rhs.storage_, &tmp);
        std::swap(handle_, rhs.handle_);
        std::swap(call_, rhs.call_);
      } else {
        rhs.swap(*this);
      }
    } else if (rhs.handle_) {
      rhs.handle_(action::move, &storage_, &rhs.storage_);
      handle_ = rhs.handle_;
      call_ = rhs.call_;
      rhs.handle_ = nullptr;
    }
  }

  explicit operator bool() const noexcept { return handle_ != nullptr; }

  R operator()(ArgTypes... args) {
    return call_(storage_, std::forward<ArgTypes>(args)...);
  }

  friend bool operator==(const any_invocable& f, std::nullptr_t) noexcept {
    return !f;
  }
  friend bool operator==(std::nullptr_t, const any_invocable& f) noexcept {
    return !f;
  }
  friend bool operator!=(const any_invocable& f, std::nullptr_t) noexcept {
    return static_cast<bool>(f);
  }
  friend bool operator!=(std::nullptr_t, const any_invocable& f) noexcept {
    return static_cast<bool>(f);
  }

  friend void swap(any_invocable& lhs, any_invocable& rhs) noexcept {
    lhs.swap(rhs);
  }

 private:
  template <class F, class... Args>
  void create(Args&&... args) {
    using hdl = handler<F>;
    hdl::create(storage_, std::forward<Args>(args)...);
    handle_ = &hdl::handle;
    call_ = &hdl::call;
  }

  void destroy() noexcept {
    if (handle_) {
      handle_(action::destroy, &storage_, nullptr);
      handle_ = nullptr;
    }
  }

 private:
  storage storage_;
  handle_func handle_ = nullptr;
  call_func call_;
};

}  // namespace ofats

#endif  // _ANY_INVOKABLE_H_
