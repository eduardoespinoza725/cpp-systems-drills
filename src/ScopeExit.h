#include <type_traits>
#include <utility>

/*
 * @brief RAII guard that invokes callable on scope exit.
 * 
 * @tparam F callable type that can be moced without throwing (noexcept()), preserving
 *    strong exception safe guarantees of RAII
 * 
 * @note Is movable, non-copyable.
 */
template <class F> class ScopeExit {
public:
  /*
   * @brief Constructs a new ScopeExit obj
   * 
   * @tparam F callable type that can be moced without throwing (noexcept()), preserving
   *    strong exception safe guarantees of RAII
   * @param f callable to be invoked on scope exit
   * @param active_ Flag that indicates guard is active or not.
   */
  explicit ScopeExit(F &&f) noexcept(std::is_nothrow_move_constructible_v<F>)
      : f_(std::forward<F>(f)), active_(true) {}

  ScopeExit(const ScopeExit &) = delete;
  ScopeExit &operator=(const ScopeExit &) = delete;

  ScopeExit(ScopeExit &&other) noexcept(std::is_nothrow_move_constructible_v<F>)
      : f_(std::move(other.f_)), active_(other.active_) {
    other.release(); // becomes inactive
  }
  
  // noexcept = must not throw
  ~ScopeExit() noexcept {
    // true = will invoke callable exactly once.
    // false = destructor do nothing.
    if (active_) {
      f_();
    }
  }

  // Disables execution
  void release() noexcept { active_ = false; }

private:
  F f_;
  bool active_;
};

/*
 * Class-Template Argument Deduction (CTAD) guide for ScopeExit.
 * 
 * - Declares template param 'F' that represents type of callable passed to ScopeExit.
 * - Tells compiler "when you see a ctor of 'ScopeExit' that takes single arg of type 'F', deduce
 *      template arg 'F' and instantiate class as 'ScopeExit<F>'."
 */
template <class F>
ScopeExit(F) ->ScopeExit<F>;
