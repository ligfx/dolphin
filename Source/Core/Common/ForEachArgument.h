// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

// Given a function and some number of arguments, applies the function to each of the arguments in
// turn.

template <typename F, typename... Args>
void ForEachArgument(F f, Args&&... args)
{
  [](...) {}((f(std::forward<Args>(args)), 0)...);
}
