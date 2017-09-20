// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <utility>
#include <vector>

using SubscriptionID = uint32_t;

struct SubscriptionHandle
{
  const void* signal;
  SubscriptionID subscription_id;
};

class SignalDispatcher
{
private:
  struct DestructibleSubscription
  {
    DestructibleSubscription(SubscriptionID subscription_id) : m_subscription_id{subscription_id} {}
    virtual ~DestructibleSubscription() = default;

    SubscriptionID m_subscription_id;
  };

  template <typename... Args>
  struct CallableSubscription : public DestructibleSubscription
  {
    using DestructibleSubscription::DestructibleSubscription;

    CallableSubscription(SubscriptionID subscription_id, std::function<void(Args...)> function)
        : DestructibleSubscription(subscription_id), m_function{std::move(function)}
    {
    }

    void operator()(Args... args) { m_function(args...); }
    std::function<void(Args...)> m_function;
  };

public:
  template <typename F, typename... Args>
  SubscriptionHandle Subscribe(const void* signal, F&& functor)
  {
    SubscriptionID id = m_next_subscription_id++;
    m_subscriptions[signal].emplace_back(
        new CallableSubscription<Args...>(id, std::forward<F>(functor)));
    return {signal, id};
  }

  void Unsubscribe(const SubscriptionHandle& handle)
  {
    auto it = m_subscriptions.find(handle.signal);
    if (it == m_subscriptions.end())
      return;

    auto& subscriptions_for_signal = it->second;

    subscriptions_for_signal.erase(
        std::remove_if(subscriptions_for_signal.begin(), subscriptions_for_signal.end(),
                       [&](DestructibleSubscription* subscription) {
                         return subscription->m_subscription_id == handle.subscription_id;
                       }),
        subscriptions_for_signal.end());
  }

  template <typename... Args>
  void Activate(const void* signal, const Args&... args)
  {
    auto subscriptions_for_signal = m_subscriptions.find(signal);
    if (subscriptions_for_signal == m_subscriptions.end())
      return;

    for (const auto& subscription : subscriptions_for_signal->second)
    {
      (*static_cast<CallableSubscription<Args...>*>(subscription))(args...);
    }
  }

  static SignalDispatcher& Instance()
  {
    static SignalDispatcher s_dispatcher;
    return s_dispatcher;
  }

private:
  SignalDispatcher() = default;
  // XXX: will overflow after UINT32_MAX subscriptions
  SubscriptionID m_next_subscription_id = 0;
  std::map<const void*, std::vector<DestructibleSubscription*>> m_subscriptions;
};
