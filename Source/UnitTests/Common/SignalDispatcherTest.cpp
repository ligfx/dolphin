// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <array>
#include <gtest/gtest.h>
#include <thread>

#include "Common/MemoryUtil.h"
#include "Common/Subscribable.h"
// #include "Common/SignalDispatcher.h"

#include <algorithm>
#include <functional>
#include <map>
#include <utility>
#include <vector>

// SignalDispatcher:
// We want to associate callbacks, of any type signature, with key values, and then later invoke
// those callbacks. We also want a singular place to store everything. The key values may be of
// different types, though in most usages will be part of some common family: think Setting<int>,
// Setting<float>, Setting<std::string>, etc.

// First, since callbacks may have any type signature, they must be type-erased to be stored
// together. (Unfortunately, this means we'll have to manually cast them back to the correct type
// later.)

struct TypeErasedSubscribable
{
  virtual ~TypeErasedSubscribable() = default;
};

template <typename... Args>
struct SubscribableWrapper : public TypeErasedSubscribable
{
  Subscribable<Args...> wrapped;
};

// Then we define the actual dispatching table:

class Dispatcher
{
public:
  template <typename... Args>
  auto Subscribe(size_t key, std::function<void(Args...)> functor)
  {
    auto& subscribable = m_subscriptions[key];
    if (subscribable == nullptr)
      subscribable = new SubscribableWrapper<Args...>;

    return static_cast<SubscribableWrapper<Args...>*>(subscribable)->wrapped.Subscribe(functor);
  }

  template <typename... Args>
  void Activate(size_t key, const Args&... args)
  {
    auto subscribable = m_subscriptions.find(key);
    if (subscribable == m_subscriptions.end())
      return;

    static_cast<SubscribableWrapper<Args...>*>(subscribable->second)->wrapped.Send(args...);
  }

private:
  std::map<std::size_t, TypeErasedSubscribable*> m_subscriptions;
};

static Dispatcher g_dispatcher;

template <typename T>
struct Setting
{
  Setting(std::string name_, T value_ = {}) : name(name_), value(value_) {}
  template <typename F>
  auto Subscribe(F&& functor)
  {
    return g_dispatcher.Subscribe(std::hash<std::string>()(name), std::function<void(T)>(functor));
  }

  void Set(const T& new_value)
  {
    value = new_value;
    g_dispatcher.Activate(std::hash<std::string>()(name), new_value);
  }

  std::string name;
  T value;
};

TEST(SignalDispatcher, BasicUsage)
{
  // Dispatcher dispatcher;

  Setting<int> simple("simple");
  simple.Set(5);
  auto subscription = simple.Subscribe([](int value) { printf("new value: %i\n", value); });
  simple.Set(47);
  simple.Set(48);

  Setting<int> simple2("simple2");
  simple2.Set(49);
  auto subscription2 =
      simple2.Subscribe([](int value) { printf("new value for simple2: %i\n", value); });
  simple2.Set(50);
  //
  // dispatcher.Activate(5);
  // dispatcher.Subscribe(5, [] { printf("signal1\n"); });
  // dispatcher.Activate(5);
  // auto print_new_value = [](int new_value) { printf("new_value: %i\n", new_value); };
  // dispatcher.Subscribe<int, decltype(print_new_value), int>(6, print_new_value);
  // dispatcher.Activate(6, 47);

  // dispatcher.Dispatch(
  // // Create the Subscribable in a special memory page so later we can test that it doesn't get
  // // accessed.
  // size_t mem_alignment = std::alignment_of<Subscribable<int>>::value;
  // size_t mem_size = sizeof(Subscribable<int>) + mem_alignment;
  // void* memory = Common::AllocateMemoryPages(mem_size);
  // void* aligned_address = reinterpret_cast<void*>(
  //     reinterpret_cast<uintptr_t>(memory) + reinterpret_cast<uintptr_t>(memory) %
  //     mem_alignment);
  // auto* s = new (aligned_address) Subscribable<int>();
  //
  // // Send to zero subscribers
  // s->Send(0);
  //
  // // Send to many subscribers
  // std::array<Subscribable<int>::Subscription, 10> subscriptions;
  // std::array<int, 10> values{};
  // for (size_t i = 0; i < values.size(); ++i)
  // {
  //   subscriptions[i] = s->Subscribe([i, &values](int new_value) { values[i] = new_value; });
  // }
  //
  // s->Send(47);
  // for (const auto& value : values)
  //   EXPECT_EQ(value, 47);
  //
  // // Send again
  // s->Send(94);
  // for (const auto& value : values)
  //   EXPECT_EQ(value, 94);
  //
  // // Unsubscribe some
  // for (size_t i = 0; i < subscriptions.size(); ++i)
  // {
  //   if (i % 2)
  //     subscriptions[i].Unsubscribe();
  // }
  //
  // s->Send(188);
  // for (size_t i = 0; i < values.size(); ++i)
  //   EXPECT_EQ(values[i], i % 2 ? 94 : 188);
  //
  // // Doesn't try to access Subscribable after destruction
  // // Destroy the Subscribable and half of the Subscriptions in concurrent threads to try to
  // expose
  // // race conditions (especially useful when running with ThreadSanitizer).
  // std::thread destroy_subscriptions([&] {
  //   for (size_t i = 0; i < subscriptions.size() / 2; ++i)
  //     subscriptions[i].Unsubscribe();
  // });
  // std::thread destroy_subscribable([&] {
  //   s->~Subscribable<int>();
  //   Common::ReadProtectMemory(memory, mem_size);
  // });
  // destroy_subscriptions.join();
  // destroy_subscribable.join();
  //
  // // Destroy the rest of the Subscriptions after we're sure the Subscribable is destroyed
  // for (auto& subscription : subscriptions)
  //   subscription.Unsubscribe();
}
