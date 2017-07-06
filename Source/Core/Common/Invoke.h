// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <utility>

// Like C++17's std::invoke, provides a way to treat functors and member function pointers
// identically.

template <class Base, class Type, class Object, class... Args>
auto Invoke(Type Base::*f, Object* obj, Args&&... args)
{
  return (obj->*f)(std::forward<Args>(args)...);
}

template <class Functor, class... Args>
auto Invoke(Functor&& f, Args&&... args)
{
  return f(std::forward<Args>(args)...);
}
