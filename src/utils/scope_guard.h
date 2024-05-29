#ifndef IMPLAYER_SCOPE_GUARD_H
#define IMPLAYER_SCOPE_GUARD_H

#include <functional>

namespace implayer
{
  class ScopeGuard
  {
  public:
    explicit ScopeGuard(std::function<void()> onExitScope)
        : onExitScope_(onExitScope) {}

    ~ScopeGuard() { onExitScope_(); }

  private:
    std::function<void()> onExitScope_;

  private: // noncopyable
    ScopeGuard(ScopeGuard const &) = delete;
    ScopeGuard &operator=(ScopeGuard const &) = delete;
  };
}

#define SCOPEGUARD_LINENAME_CAT(name, line) name##line
#define SCOPEGUARD_LINENAME(name, line) SCOPEGUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPEGUARD_LINENAME(EXIT, __LINE__)(callback)

#endif // IMPLAYER_SCOPE_GUARD_H
